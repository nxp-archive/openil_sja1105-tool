/*
 * SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
 *
 * Copyright (c) 2018, NXP Semiconductors
 */

#include <linux/version.h>
#include <linux/time64.h>
#include "sja1105.h"

#include <lib/include/static-config.h>
#include <lib/include/gtable.h>

#define SJA1105PQRS_PTPSYNCTS_ADDR  0x1F
#define SJA1105QS_PTPCLKCORP_ADDR   0x1E
#define SJA1105PQRS_PTPTSCLK_ADDR   0x1C
#define SJA1105PQRS_PTPCLKRATE_ADDR 0x1B
#define SJA1105PQRS_PTPCLKVAL_ADDR  0x19
#define SJA1105PQRS_PTPPINDUR_ADDR  0x17
#define SJA1105PQRS_PTPPINST_ADDR   0x15
#define SJA1105QS_PTPSCHTM_ADDR     0x13

#define SJA1105T_PTPCLKCORP_ADDR    0x1D
#define SJA1105ET_PTPTSCLK_ADDR     0x1B
#define SJA1105ET_PTPCLKRATE_ADDR   0x1A
#define SJA1105ET_PTPCLKVAL_ADDR    0x18
#define SJA1105ET_PTPPINDUR_ADDR    0x16
#define SJA1105ET_PTPPINST_ADDR     0x14
#define SJA1105T_PTPSCHTM_ADDR      0x12

/* Offset into CORE_ADDR */
#define PTP_ADDR                    0x0

/* At full swing, the PTPCLKVAL can either speed up to 2x PTPTSCLK (when
 * PTPCLKRATE = 0xffffffff), or slow down to 1/2x PTPTSCLK (when PTPCLKRATE =
 * 0x0). PTPCLKRATE is centered around 0x80000000.  This means that the
 * hardware supports one full billion parts per billion frequency adjustments,
 * i.e. recover 1 whole second of offset (or NSEC_PER_SEC as the ppb unit
 * expresses it) during 1 second.
 */
#define SJA1105_MAX_ADJ_PPB        NSEC_PER_SEC

enum sja1105_ptpegr_ts_source {
	TS_PTPTSCLK = 0,
	TS_PTPCLK = 1,
};

struct sja1105_ptp_cmd {
	u64 ptpstrtsch;   /* start schedule */
	u64 ptpstopsch;   /* stop schedule */
	u64 startptpcp;   /* start pin toggle  */
	u64 stopptpcp;    /* stop pin toggle */
	u64 cassync;      /* if cascaded master, trigger a toggle of the
	                     PTP_CLK pin, and store the timestamp of its
	                     1588 clock (ptpclk or ptptsclk, depending on
	                     corrclk4ts), in ptpsyncts.
	                     only for P/Q/R/S series */
	u64 resptp;       /* reset */
	u64 corrclk4ts;   /* if (1) timestamps are based on ptpclk,
	                     if (0) timestamps are based on ptptsclk */
	u64 ptpclksub;    /* only for P/Q/R/S series */
	u64 ptpclkadd;    /* enum sja1105_ptp_clk_add_mode */
};

enum ptp_op {
	PTP_NONE,
	PTP_CLOCKSTEP,
	PTP_ADJUSTFREQ,
};

/* Global variable */
static enum ptp_op sja1105_last_ptp_op;

static int
timespec_lower(const struct timespec64 *lhs, const struct timespec64 *rhs)
{
	if (lhs->tv_sec == rhs->tv_sec)
		return lhs->tv_nsec < rhs->tv_nsec;
	else
		return lhs->tv_sec < rhs->tv_sec;
}

static void
timespec_diff(const struct timespec64 *start, const struct timespec64 *stop,
              struct timespec64 *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + NSEC_PER_SEC;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}
}

static void
sja1105_ptp_cmd_access(void *buf, struct sja1105_ptp_cmd *cmd,
                       int write, u64 device_id)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;
	/* No need to keep this as part of the structure */
	u64 valid = 1;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(cmd, 0, sizeof(*cmd));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &valid,           31, 31, 4);
	pack_or_unpack(buf, &cmd->ptpstrtsch, 30, 30, 4);
	pack_or_unpack(buf, &cmd->ptpstopsch, 29, 29, 4);
	pack_or_unpack(buf, &cmd->startptpcp, 28, 28, 4);
	pack_or_unpack(buf, &cmd->stopptpcp,  27, 27, 4);
	if (IS_ET(device_id)) {
		pack_or_unpack(buf, &cmd->resptp,      3,  3, 4);
		pack_or_unpack(buf, &cmd->corrclk4ts,  2,  2, 4);
		pack_or_unpack(buf, &cmd->ptpclksub,   1,  1, 4);
		pack_or_unpack(buf, &cmd->ptpclkadd,   0,  0, 4);
	} else {
		pack_or_unpack(buf, &cmd->cassync,    25, 25, 4);
		pack_or_unpack(buf, &cmd->resptp,      2,  2, 4);
		pack_or_unpack(buf, &cmd->corrclk4ts,  1,  1, 4);
		pack_or_unpack(buf, &cmd->ptpclkadd,   0,  0, 4);
	}
}
#define sja1105_ptp_cmd_pack(buf, cmd, device_id) \
	sja1105_ptp_cmd_access(buf, cmd, 1, device_id)
#define sja1105_ptp_cmd_unpack(buf, cmd, device_id) \
	sja1105_ptp_cmd_access(buf, cmd, 0, device_id)

/* Wrapper around sja1105_spi_send_packed_buf() */
static int sja1105_ptp_cmd_commit(struct sja1105_spi_private *priv,
                                  struct sja1105_ptp_cmd *cmd)
{
	u64 device_id = priv->device_id;
	const int BUF_LEN = 4;
	u8 packed_buf[BUF_LEN];
	int ptp_control_addr;

	if (IS_ET(device_id))
		ptp_control_addr = 0x17;
	else
		ptp_control_addr = 0x18;

	sja1105_ptp_cmd_pack(packed_buf, cmd, device_id);
	return sja1105_spi_send_packed_buf(priv, SPI_WRITE,
	                                   CORE_ADDR + ptp_control_addr,
	                                   packed_buf, BUF_LEN);
}

/* Wrapper around sja1105_spi_send_int() using SPI_READ
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_read_reg(struct sja1105_spi_private *priv,
                     u64 reg_offset, u64 *value, u64 size_bytes)
{
	return sja1105_spi_send_int(priv, SPI_READ, CORE_ADDR + PTP_ADDR +
	                            reg_offset, value, size_bytes);
}

/* Wrapper around sja1105_spi_send_int() using SPI_WRITE
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_write_reg(struct sja1105_spi_private *priv, u64 reg_offset,
                      u64 *value, u64 size_bytes)
{
	return sja1105_spi_send_int(priv, SPI_WRITE, CORE_ADDR + PTP_ADDR +
	                            reg_offset, value, size_bytes);
}

static void
sja1105_timespec_to_ptp_time(const struct timespec64 *ts, u64 *ptp_time)
{
	*ptp_time = (ts->tv_sec * NSEC_PER_SEC + ts->tv_nsec) / 8;
}

static void
sja1105_ptp_time_to_timespec(struct timespec64 *ts, u64 ptp_time)
{
	/* Check whether we can actually multiply by 8ns
	 * (the hw resolution) without overflow */
	if (ptp_time >= 0x1FFFFFFFFFFFFFFFull)
		pr_err("Integer overflow during timespec conversion!\n");
	u64_to_timespec64(ts, ptp_time * 8);
}

int sja1105_ptp_pin_toggle_start(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.startptpcp = 1;
	return sja1105_ptp_cmd_commit(priv, &cmd);
}

int sja1105_ptp_pin_toggle_stop(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.stopptpcp = 1;
	return sja1105_ptp_cmd_commit(priv, &cmd);
}

/* Write to PTPPINST */
int sja1105_ptp_pin_start_time_set(struct sja1105_spi_private *priv,
                                   const struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptppinst_addr;
	u64 pinst;

	if (IS_ET(device_id))
		ptppinst_addr = SJA1105ET_PTPPINST_ADDR;
	else
		ptppinst_addr = SJA1105PQRS_PTPPINST_ADDR;

	sja1105_timespec_to_ptp_time(ts, &pinst);
	return sja1105_ptp_write_reg(priv, ptppinst_addr, &pinst, 8);
}

#define UINT32_MAX  0xFFFFFFFF

/* Write to PTPPINDUR */
int sja1105_ptp_pin_duration_set(struct sja1105_spi_private *priv,
                                 const struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptppindur_addr;
	u64 pindur;
	int rc;

	if (IS_ET(device_id))
		ptppindur_addr = SJA1105ET_PTPPINDUR_ADDR;
	else
		ptppindur_addr = SJA1105PQRS_PTPPINDUR_ADDR;

	sja1105_timespec_to_ptp_time(ts, &pindur);
	if (pindur >= UINT32_MAX) {
		dev_err(&priv->spi_dev->dev,
		        "pindur: provided ts is too large\n");
		rc = -ERANGE;
		goto out;
	}
	rc = sja1105_ptp_write_reg(priv, ptppindur_addr, &pindur, 4);
out:
	return rc;
}

static int sja1105_tas_check_running(struct sja1105_spi_private *priv)
{
	u64 device_id = priv->device_id;
	struct  sja1105_ptp_cmd cmd;
	const int BUF_LEN = 4;
	u8 packed_buf[BUF_LEN];
	int ptp_control_addr;
	int rc = 0;

	if (IS_PQRS(device_id))
		/* Only Q/S will enter here. */
		ptp_control_addr = 0x18;
	else
		/* Only T will enter here */
		ptp_control_addr = 0x17;

	rc = sja1105_spi_send_packed_buf(priv, SPI_READ, CORE_ADDR +
	                                 ptp_control_addr, packed_buf, BUF_LEN);
	if (rc < 0)
		goto out;

	sja1105_ptp_cmd_unpack(packed_buf, &cmd, device_id);

	if (cmd.ptpstrtsch == 1)
		/* Schedule successfully started */
		priv->tas_state = TAS_STATE_RUNNING;
	else if (cmd.ptpstopsch == 1)
		/* Schedule is stopped */
		priv->tas_state = TAS_STATE_DISABLED;
	else
		/* Schedule is probably not configured with PTP clock source */
		rc = -EINVAL;
out:
	return rc;
}

/* Write to PTPCLKCORP */
static int
sja1105_tas_apply_correction_period(struct sja1105_spi_private *priv,
                                    const struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptpclkcorp_addr;
	u64 ptpclkcorp;
	int rc;

	if (IS_ET(device_id))
		/* Only T will enter here */
		ptpclkcorp_addr = SJA1105T_PTPCLKCORP_ADDR;
	else
		/* Only Q/S will enter here */
		ptpclkcorp_addr = SJA1105QS_PTPCLKCORP_ADDR;

	sja1105_timespec_to_ptp_time(ts, &ptpclkcorp);
	if (ptpclkcorp >= UINT32_MAX) {
		dev_err(&priv->spi_dev->dev,
		        "ptpclkcorp: provided ts is too large");
		rc = -ERANGE;
		goto out;
	}
	rc = sja1105_ptp_write_reg(priv, ptpclkcorp_addr, &ptpclkcorp, 4);
out:
	return rc;
}

/* Write to PTPSCHTM */
static int
sja1105_tas_set_start_time(struct sja1105_spi_private *priv,
                           const struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptpschtm_addr;
	u64 ptpschtm;
	int rc;

	if (IS_ET(device_id))
		/* Only T enters here */
		ptpschtm_addr = SJA1105T_PTPSCHTM_ADDR;
	else
		/* Only Q/S enter here */
		ptpschtm_addr = SJA1105QS_PTPSCHTM_ADDR;

	sja1105_timespec_to_ptp_time(ts, &ptpschtm);
	rc = sja1105_ptp_write_reg(priv, ptpschtm_addr, &ptpschtm, 8);
	if (rc < 0)
		return rc;

	priv->tas_start_time = *ts;
	return 0;
}

static int sja1105_tas_start(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;
	int rc;

	if (priv->tas_state == TAS_STATE_ENABLED_NOT_RUNNING ||
	    priv->tas_state == TAS_STATE_RUNNING) {
		dev_err(&priv->spi_dev->dev, "TAS already started!\n");
		return -EINVAL;
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.ptpstrtsch = 1;
	rc = sja1105_ptp_cmd_commit(priv, &cmd);
	if (rc < 0)
		return rc;

	priv->tas_state = TAS_STATE_ENABLED_NOT_RUNNING;
	return 0;
}

static int sja1105_tas_stop(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;
	int rc;

	if (priv->tas_state == TAS_STATE_DISABLED) {
		dev_err(&priv->spi_dev->dev, "TAS already disabled!\n");
		return -EINVAL;
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.ptpstopsch = 1;
	rc = sja1105_ptp_cmd_commit(priv, &cmd);
	if (rc < 0)
		return rc;

	priv->tas_state = TAS_STATE_DISABLED;
	return 0;
}

static int sja1105_ptp_gettime(struct ptp_clock_info *ptp,
                               struct timespec64 *ts);

void sja1105_tas_state_machine(struct work_struct *work)
{
	struct sja1105_spi_private *priv = container_of(work, struct
	                                   sja1105_spi_private, tas_work);
	struct timespec64 tas_start_time;
	struct timespec64 ptpclk_now;
	struct timespec64 diff;
	u64 tas_start_time_ns;
	u64 tas_cycle_len_ns;
	u64 ptpclk_now_ns;
	int rc;

	switch (priv->tas_state) {
	case TAS_STATE_DISABLED:

		dev_dbg(&priv->spi_dev->dev, "TAS state: disabled\n");
		/* Can't do anything at all if clock is still being stepped */
		if (sja1105_last_ptp_op != PTP_ADJUSTFREQ)
			break;

		rc = sja1105_ptp_gettime(&priv->ptp_caps, &ptpclk_now);
		if (rc < 0)
			return;

		/* Delay start time to the beginning of the first TAS cycle
		 * that starts at least 3 seconds from now.
		 * This should buy us some time.
		 */
		ptpclk_now.tv_sec += 3;
		sja1105_timespec_to_ptp_time(&ptpclk_now, &ptpclk_now_ns);
		sja1105_timespec_to_ptp_time(&priv->tas_cycle_len, &tas_cycle_len_ns);
		/* Round start_time_ns to the closest cycle_len_ns multiple */
		tas_start_time_ns = 1 + div_u64(ptpclk_now_ns, tas_cycle_len_ns);
		tas_start_time_ns *= tas_cycle_len_ns;
		sja1105_ptp_time_to_timespec(&tas_start_time, tas_start_time_ns);

		rc = sja1105_tas_set_start_time(priv, &tas_start_time);
		if (rc < 0)
			return;

		rc = sja1105_tas_apply_correction_period(priv, &priv->tas_cycle_len);
		if (rc < 0)
			return;

		rc = sja1105_tas_start(priv);
		if (rc < 0)
			return;
		break;

	case TAS_STATE_ENABLED_NOT_RUNNING:
		/* Check if TAS has actually started, by comparing the
		 * scheduled start time with the SJA1105 PTP clock
		 */
		dev_dbg(&priv->spi_dev->dev,
		        "TAS state: enabled but not running\n");

		/* Clock was stepped.. bad news for TAS */
		if (sja1105_last_ptp_op != PTP_ADJUSTFREQ) {
			sja1105_tas_stop(priv);
			break;
		}

		rc = sja1105_ptp_gettime(&priv->ptp_caps, &ptpclk_now);
		if (rc < 0)
			return;

		if (timespec_lower(&ptpclk_now, &priv->tas_start_time)) {
			/* TAS has not started yet */
			timespec_diff(&ptpclk_now, &priv->tas_start_time, &diff);
			dev_dbg(&priv->spi_dev->dev,
			        "time to start: [%lld.%09ld]",
			         diff.tv_sec, diff.tv_nsec);
			break;
		}

		/* Time elapsed, what happened? */
		rc = sja1105_tas_check_running(priv);
		if (rc < 0)
			return;

		if (priv->tas_state == TAS_STATE_RUNNING)
			/* TAS has started */
			dev_dbg(&priv->spi_dev->dev,
			        "TAS state: transitioned to running\n");
		else
			dev_err(&priv->spi_dev->dev,
			        "TAS state: not started despite time elapsed\n");

		break;

	case TAS_STATE_RUNNING:
		dev_dbg(&priv->spi_dev->dev, "TAS state: running\n");

		/* Clock was stepped.. bad news for TAS */
		if (sja1105_last_ptp_op != PTP_ADJUSTFREQ) {
			sja1105_tas_stop(priv);
			break;
		}

		rc = sja1105_tas_check_running(priv);
		if (rc < 0)
			return;

		if (priv->tas_state != TAS_STATE_RUNNING) {
			dev_err(&priv->spi_dev->dev, "TAS surprisingly stopped\n");
			break;
		}
		rc = sja1105_ptp_gettime(&priv->ptp_caps, &ptpclk_now);
		if (rc < 0)
			return;

		timespec_diff(&priv->tas_start_time, &ptpclk_now, &diff);
		dev_dbg(&priv->spi_dev->dev,
		        "Time since TAS started: [%lld.%09ld]",
		         diff.tv_sec, diff.tv_nsec);
		break;

	default:
		if (net_ratelimit())
			dev_err(&priv->spi_dev->dev,
			        "TAS in an invalid state (incorrect use of API)!\n");
	}
}

int sja1105_ptpegr_ts_poll(struct sja1105_spi_private *priv,
                           enum sja1105_ptpegr_ts_source source,
                           int port, int ts_regid, struct timespec64 *ts)
{
	const int ts_reg_index = 2 * port + ts_regid;
	const int SIZE_PTPEGR_TS = 4;
	u8  packed_buf[SIZE_PTPEGR_TS];
	u64 device_id = priv->device_id;
	u64 ptpegr_ts_reconstructed;
	u64 ptpegr_ts_partial;
	u64 ptpegr_ts_mask;
	u64 ptp_full_current_ts;
	u64 ptpclk_addr;
	u64 update;
	int rc;

	if (source == TS_PTPCLK)
		/* Use PTPCLK */
		ptpclk_addr = IS_ET(device_id) ?
		              SJA1105ET_PTPCLKVAL_ADDR :
		              SJA1105PQRS_PTPCLKVAL_ADDR;
	else if (source == TS_PTPTSCLK)
		/* Use PTPTSCLK */
		ptpclk_addr = IS_ET(device_id) ?
		              SJA1105ET_PTPTSCLK_ADDR :
		              SJA1105PQRS_PTPTSCLK_ADDR;
	else {
		loge("%s: invalid source selection: %d", __func__, source);
		rc = -EINVAL;
		goto out;
	}
	rc = sja1105_spi_send_packed_buf(priv, SPI_READ,
	                                 CORE_ADDR + 0xC0 + ts_reg_index,
	                                 packed_buf, SIZE_PTPEGR_TS);
	if (rc < 0) {
		loge("failed to read ptp egress timestamp register %d",
		     ts_reg_index);
		goto out;
	}
	gtable_unpack(packed_buf, &ptpegr_ts_partial, 31, 8, SIZE_PTPEGR_TS);
	gtable_unpack(packed_buf, &update,             0, 0, SIZE_PTPEGR_TS);

	if (!update) {
		/* No update. Keep trying, you'll make it someday. */
		rc = -EAGAIN;
		goto out;
	}
	rc = sja1105_ptp_read_reg(priv, ptpclk_addr, &ptp_full_current_ts, 8);
	if (rc < 0) {
		loge("failed to read ptpclkval/ptptsclk");
		goto out;
	}
	/* E/T and P/Q/R/S have different sized egress timestamps */
	if (IS_ET(device_id))
		ptpegr_ts_mask = (1ull << 24ull) - 1;
	else
		ptpegr_ts_mask = (1ull << 32ull) - 1;

	ptpegr_ts_reconstructed = (ptp_full_current_ts & ~ptpegr_ts_mask) |
	                           ptpegr_ts_partial;
	/* Check if wraparound occurred between moment when the partial
	 * ptpegr timestamp was generated, and the moment when that
	 * timestamp is being read out (now, ptpclkval/ptptsclk).
	 * If last 24 bits (32 for P/Q/R/S) of current ptpclkval/ptptsclk
	 * time are lower than the partial timestamp, then wraparound surely
	 * occurred, as ptpclkval is 64-bit.
	 * What is up to anyone's guess is how many times has the wraparound
	 * occurred. The code assumes (perhaps foolishly?) that if wraparound
	 * is present, it has only occurred once, and thus corrects for it.
	 */
	if ((ptp_full_current_ts & ptpegr_ts_mask) <= ptpegr_ts_partial)
		ptpegr_ts_reconstructed -= (ptpegr_ts_mask + 1ull);

	sja1105_ptp_time_to_timespec(ts, ptpegr_ts_reconstructed);
out:
	return rc;
}

/* Read PTPTSCLK */
int sja1105_ptp_ts_clk_get(struct sja1105_spi_private *priv,
                           struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptptsclk_addr;
	u64 ptptsclk;
	int rc;

	if (IS_ET(device_id))
		ptptsclk_addr = SJA1105ET_PTPTSCLK_ADDR;
	else
		ptptsclk_addr = SJA1105PQRS_PTPTSCLK_ADDR;

	rc = sja1105_ptp_read_reg(priv, ptptsclk_addr, &ptptsclk, 8);
	if (rc < 0) {
		dev_err(&priv->spi_dev->dev, "Failed to read ptptsclk\n");
		goto out;
	}
	sja1105_ptp_time_to_timespec(ts, ptptsclk);
out:
	return rc;
}

static int sja1105_ptp_reset(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.resptp = 1;
	dev_dbg(&priv->spi_dev->dev, "Resetting PTP clock\n");
	return sja1105_ptp_cmd_commit(priv, &cmd);
}

static inline int
sja1105_ptp_add_mode_set(struct sja1105_spi_private *priv,
                         enum sja1105_ptp_clk_add_mode mode)
{
	struct sja1105_ptp_cmd cmd;
	int rc;

	if (priv->ptp_add_mode == mode)
		return 0;

	memset(&cmd, 0, sizeof(cmd));
	cmd.ptpclkadd = mode;
	rc = sja1105_ptp_cmd_commit(priv, &cmd);
	if (rc < 0)
		return rc;

	priv->ptp_add_mode = mode;
	return 0;
}

static inline int
sja1105_ptp_clk_write(struct sja1105_spi_private *priv,
                      const struct timespec64 *ts)
{
	u64 device_id = priv->device_id;
	u64 ptpclkval_addr;
	u64 ptpclkval;

	if (IS_ET(device_id))
		ptpclkval_addr = SJA1105ET_PTPCLKVAL_ADDR;
	else
		ptpclkval_addr = SJA1105PQRS_PTPCLKVAL_ADDR;

	sja1105_timespec_to_ptp_time(ts, &ptpclkval);
	return sja1105_ptp_write_reg(priv, ptpclkval_addr,
	                             &ptpclkval, 8);
}

static int sja1105_ptp_gettime(struct ptp_clock_info *ptp,
                               struct timespec64 *ts)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	u64 device_id = priv->device_id;
	u64 ptpclkval_addr;
	u64 ptpclkval;
	int rc;

	if (IS_ET(device_id))
		ptpclkval_addr = SJA1105ET_PTPCLKVAL_ADDR;
	else
		ptpclkval_addr = SJA1105PQRS_PTPCLKVAL_ADDR;

	rc = sja1105_ptp_read_reg(priv, ptpclkval_addr, &ptpclkval, 8);
	if (rc < 0) {
		dev_err(&priv->spi_dev->dev, "failed to read ptpclkval\n");
		goto out;
	}
	sja1105_ptp_time_to_timespec(ts, ptpclkval);
out:
	return rc;
}

/* Write to PTPCLKVAL while PTPCLKADD is 0 */
static int sja1105_ptp_settime(struct ptp_clock_info *ptp,
                               const struct timespec64 *ts)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	int rc;

	rc = sja1105_ptp_add_mode_set(priv, PTP_SET_MODE);
	if (rc < 0) {
		dev_err(&priv->spi_dev->dev, "Failed to put PTPCLK in set mode\n");
		goto out;
	}
	rc = sja1105_ptp_clk_write(priv, ts);

	if (priv->configured_for_scheduling) {
		sja1105_last_ptp_op = PTP_CLOCKSTEP;
		schedule_work(&priv->tas_work);
	}
out:
	return rc;
}

/* Write to PTPCLKRATE */
static int sja1105_ptp_adjfine(struct ptp_clock_info *ptp, long scaled_ppm)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	u64 device_id = priv->device_id;
	u64 ptpclkrate_addr;
	u64 ptpclkrate;
	int rc;

	if (IS_ET(device_id))
		ptpclkrate_addr = SJA1105ET_PTPCLKRATE_ADDR;
	else
		ptpclkrate_addr = SJA1105PQRS_PTPCLKRATE_ADDR;

	/*            This range is actually +/- SJA1105_MAX_ADJ_PPB
	 *            divided by 1000 (ppb -> ppm) and with a 16-bit
	 *            "fractional" part (actually fixed point).
	 *                                    |
	 *                                    v
	 * Convert scaled_ppm from the +/- ((10^6) << 16) range
	 * into the +/- (1 << 31) range (which the hw supports).
	 *
	 *   ptpclkrate = scaled_ppm * 2^31 / (10^6 * 2^16)
	 *   simplifies to
	 *   ptpclkrate = scaled_ppm * 2^9 / 5^6
	 */
	ptpclkrate = (u64) scaled_ppm << 9;
	ptpclkrate = div_s64(ptpclkrate, 15625);
	/* Take a +/- value and re-center it around 2^31. */
	ptpclkrate += 0x80000000ull;

	rc = sja1105_ptp_write_reg(priv, ptpclkrate_addr, &ptpclkrate, 4);

	if (priv->configured_for_scheduling) {
		sja1105_last_ptp_op = PTP_ADJUSTFREQ;
		schedule_work(&priv->tas_work);
	}

	return rc;
}

/* The physical significance of delta (expressed in ppb) is "how many unit
 * parts of clock offset need to be recovered during the next full unit".
 * In English, that means "how many nanoseconds of offset need to be recovered
 * during the next second".
 */
#if KERNEL_VERSION(4, 9, 0) >= LINUX_VERSION_CODE
static int sja1105_ptp_adjfreq(struct ptp_clock_info *ptp, s32 delta)
{
	s64 scaled_ppm;

	/*
	 * Convert the ppb value back into ppm + 16 bit fractional field
	 * (derived from scaled_ppm_to_ppb()).
	 *
	 * scaled_ppm = ppb * 2^16 / 1000
	 * simplifies to
	 * scaled_ppm = ppb * 2^13 / 125
	 */
	scaled_ppm = delta << 13;
	scaled_ppm = div_s64(scaled_ppm, 125);

	return sja1105_ptp_adjfine(ptp, (long)scaled_ppm);
}
#endif

/* Write to PTPCLKVAL while PTPCLKADD is 1 */
static int sja1105_ptp_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	struct timespec64 ts = ns_to_timespec64(delta);
	int rc;

	rc = sja1105_ptp_add_mode_set(priv, PTP_ADD_MODE);
	if (rc < 0) {
		dev_err(&priv->spi_dev->dev, "Failed to put PTPCLK in add mode\n");
		goto out;
	}
	rc = sja1105_ptp_clk_write(priv, &ts);

	if (priv->configured_for_scheduling) {
		sja1105_last_ptp_op = PTP_CLOCKSTEP;
		schedule_work(&priv->tas_work);
	}

out:
	return rc;
}

static const struct ptp_clock_info sja1105_ptp_caps = {
	.owner     = THIS_MODULE,
	.name      = "SJA1105 PHC",
	.max_adj   = SJA1105_MAX_ADJ_PPB, /* has real physical significance */
#if KERNEL_VERSION(4, 9, 0) >= LINUX_VERSION_CODE
	.adjfreq   = sja1105_ptp_adjfreq,
#else
	.adjfine   = sja1105_ptp_adjfine,
#endif
	.adjtime   = sja1105_ptp_adjtime,
	.gettime64 = sja1105_ptp_gettime,
	.settime64 = sja1105_ptp_settime,
};

int sja1105_ptp_clock_register(struct sja1105_spi_private *priv)
{
	struct timespec64 now;
	int rc;

	INIT_WORK(&priv->tas_work, sja1105_tas_state_machine);

	priv->ptp_caps = sja1105_ptp_caps;

	priv->clock = ptp_clock_register(&priv->ptp_caps, &priv->spi_dev->dev);
	if (IS_ERR_OR_NULL(priv->clock))
		return PTR_ERR(priv->clock);

	rc = sja1105_ptp_reset(priv);
	if (rc < 0)
		return -EIO;
	getnstimeofday64(&now);
	sja1105_ptp_settime(&priv->ptp_caps, &now);
	priv->tas_state = TAS_STATE_DISABLED;

	return 0;
}

void sja1105_ptp_clock_unregister(struct sja1105_spi_private *priv)
{
	if (IS_ERR_OR_NULL(priv->clock))
		return;

	cancel_work_sync(&priv->tas_work);

	ptp_clock_unregister(priv->clock);
	priv->clock = NULL;
}

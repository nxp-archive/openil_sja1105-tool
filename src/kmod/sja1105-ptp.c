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
#include <lib/include/spi.h>

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

/* From UM10944.pdf:
 * PTPCLKRATE: This field determines the speed of PTPCLKVAL. It implements a
 * fixed-point clock rate value with a single-bit integer part and a 31-bit
 * fractional part allowing for sub-ppb rate corrections. PTPCLKVAL ticks at
 * the rate of PTPTSCLK multiplied by this field. So any value having the
 * integer part set to 0 (i.e. bit 31 set to 0) will cause PTPCLKVAL to be
 * slower than PTPTSCLK. Any value having the integer part set to one will
 * cause PTPCLKVAL to be at least as fast as PTPTSCLK. E.g. a value of
 * h’90000000 will cause PTPCLKVAL to tick 1.125 = (2 0 + 2 3 ) faster than
 * PTPTSCLK. This field returns all 0s on read.
 */
#define SJA1105_MAX_ADJ             ((1ull << 31) - 1)

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
	u64 device_id = priv->spi_setup.device_id;
	const int BUF_LEN = 4;
	u8 packed_buf[BUF_LEN];
	int ptp_control_addr;

	/* Cannot perform all the compatibility matrix checks
	 * in this relatively time-critical code portion.
	 * Relying on the caller to not access an unsupported scheduling
	 * register on non-TTEthernet-capable devices.
	 */
	if (IS_ET(device_id))
		ptp_control_addr = 0x17;
	else
		ptp_control_addr = 0x18;

	sja1105_ptp_cmd_pack(packed_buf, cmd, device_id);
	return sja1105_spi_send_packed_buf(&priv->spi_setup, SPI_WRITE,
	                                   CORE_ADDR + ptp_control_addr,
	                                   packed_buf, BUF_LEN);
}

/* Wrapper around sja1105_spi_send_int() using SPI_READ
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_read_reg(struct sja1105_spi_private *priv,
                     u64 reg_offset, u64 *value,
                     u64 size_bytes)
{
	return sja1105_spi_send_int(&priv->spi_setup, SPI_READ,
	                            CORE_ADDR + PTP_ADDR + reg_offset,
	                            value, size_bytes);
}

/* Wrapper around sja1105_spi_send_int() using SPI_WRITE
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_write_reg(struct sja1105_spi_private *priv, u64 reg_offset,
                      u64 *value, u64 size_bytes)
{
	return sja1105_spi_send_int(&priv->spi_setup, SPI_WRITE,
	                            CORE_ADDR + PTP_ADDR + reg_offset,
	                            value, size_bytes);
}

static void
sja1105_timespec_to_ptp_time(const struct timespec64 *ts, u64 *ptp_time)
{
	*ptp_time = (ts->tv_sec * NSEC_PER_SEC + ts->tv_nsec) / 8;
}

static void
sja1105_ptp_time_to_timespec(struct timespec64 *ts, u64 ptp_time)
{
	ptp_time *= 8;
	ts->tv_sec  = div_u64(ptp_time, NSEC_PER_SEC);
	ts->tv_nsec = ptp_time - NSEC_PER_SEC * ts->tv_sec;
}

#define UINT32_MAX  0xFFFFFFFF

int sja1105_ptp_is_schedule_running(struct sja1105_spi_private *priv)
{
	u64 device_id = priv->spi_setup.device_id;
	struct  sja1105_ptp_cmd cmd;
	const int BUF_LEN = 4;
	u8 packed_buf[BUF_LEN];
	int ptp_control_addr;
	int rc;

	if (!SUPPORTS_TSN(device_id)) {
		loge("1588 + TTEthernet is only supported on T and Q/S!");
		rc = -EINVAL;
		goto out;
	}
	if (IS_PQRS(device_id))
		/* Only Q/S will enter here. */
		ptp_control_addr = 0x18;
	else
		/* Only T will enter here */
		ptp_control_addr = 0x17;

	rc = sja1105_spi_send_packed_buf(&priv->spi_setup, SPI_READ,
	                                 CORE_ADDR + ptp_control_addr,
	                                 packed_buf, BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}
	sja1105_ptp_cmd_unpack(packed_buf, &cmd, device_id);

	if (cmd.ptpstrtsch == 1)
		/* Schedule successfully started */
		rc = 0;
	else if (cmd.ptpstopsch == 1)
		/* Schedule is stopped */
		rc = 1;
	else
		/* Schedule is probably not configured with PTP clock source */
		rc = -EINVAL;

out:
	return rc;
}

int sja1105_ptp_schedule_start(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.ptpstrtsch = 1;
	return sja1105_ptp_cmd_commit(priv, &cmd);
}

int sja1105_ptp_schedule_stop(struct sja1105_spi_private *priv)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.ptpstopsch = 1;
	return sja1105_ptp_cmd_commit(priv, &cmd);
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
	u64 device_id = priv->spi_setup.device_id;
	u64 ptppinst_addr;
	u64 pinst;

	if (IS_ET(device_id))
		ptppinst_addr = SJA1105ET_PTPPINST_ADDR;
	else
		ptppinst_addr = SJA1105PQRS_PTPPINST_ADDR;

	sja1105_timespec_to_ptp_time(ts, &pinst);
	return sja1105_ptp_write_reg(priv, ptppinst_addr, &pinst, 8);
}

/* Write to PTPPINDUR */
int sja1105_ptp_pin_duration_set(struct sja1105_spi_private *priv,
                                 const struct timespec64 *ts)
{
	u64 device_id = priv->spi_setup.device_id;
	u64 ptppindur_addr;
	u64 pindur;
	int rc;

	if (IS_ET(device_id))
		ptppindur_addr = SJA1105ET_PTPPINDUR_ADDR;
	else
		ptppindur_addr = SJA1105PQRS_PTPPINDUR_ADDR;

	sja1105_timespec_to_ptp_time(ts, &pindur);
	if (pindur >= UINT32_MAX) {
		loge("%s: provided ts is too large", __func__);
		rc = -ERANGE;
		goto out;
	}
	rc = sja1105_ptp_write_reg(priv, ptppindur_addr, &pindur, 4);
out:
	return rc;
}

/* Write to PTPCLKCORP */
int
sja1105_ptp_schedule_correction_period_set(struct sja1105_spi_private *priv,
                                           const struct timespec64 *ts)
{
	u64 device_id = priv->spi_setup.device_id;
	u64 ptpclkcorp_addr;
	u64 ptpclkcorp;
	int rc;

	if (!SUPPORTS_TSN(device_id)) {
		loge("1588 + TTEthernet is only supported on T and Q/S!");
		rc = -EINVAL;
		goto out;
	}
	if (IS_ET(device_id))
		/* Only T will enter here */
		ptpclkcorp_addr = SJA1105T_PTPCLKCORP_ADDR;
	else
		/* Only Q/S will enter here */
		ptpclkcorp_addr = SJA1105QS_PTPCLKCORP_ADDR;

	sja1105_timespec_to_ptp_time(ts, &ptpclkcorp);
	if (ptpclkcorp >= UINT32_MAX) {
		loge("%s: provided ts is too large", __func__);
		rc = -ERANGE;
		goto out;
	}
	rc = sja1105_ptp_write_reg(priv, ptpclkcorp_addr, &ptpclkcorp, 4);
out:
	return rc;
}

/* Write to PTPSCHTM */
int sja1105_ptp_schedule_start_time_set(struct sja1105_spi_private *priv,
                                        const struct timespec64 *ts)
{
	u64 device_id = priv->spi_setup.device_id;
	u64 ptpschtm_addr;
	u64 ptpschtm;

	if (!SUPPORTS_TSN(device_id)) {
		loge("1588 + TTEthernet is only supported on T and Q/S!");
		return -EINVAL;
	}
	if (IS_ET(device_id))
		/* Only T enters here */
		ptpschtm_addr = SJA1105T_PTPSCHTM_ADDR;
	else
		/* Only Q/S enter here */
		ptpschtm_addr = SJA1105QS_PTPSCHTM_ADDR;

	sja1105_timespec_to_ptp_time(ts, &ptpschtm);
	return sja1105_ptp_write_reg(priv, ptpschtm_addr, &ptpschtm, 8);
}

int sja1105_ptp_corrclk4ts_set(struct sja1105_spi_private *priv,
                               enum sja1105_ptpegr_ts_source source)
{
	struct sja1105_ptp_cmd cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.corrclk4ts = source;
	return sja1105_ptp_cmd_commit(priv, &cmd);
}

int sja1105_ptpegr_ts_poll(struct sja1105_spi_private *priv,
                           enum sja1105_ptpegr_ts_source source,
                           int port, int ts_regid, struct timespec64 *ts)
{
	const int ts_reg_index = 2 * port + ts_regid;
	const int SIZE_PTPEGR_TS = 4;
	u8  packed_buf[SIZE_PTPEGR_TS];
	u64 device_id = priv->spi_setup.device_id;
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
	rc = sja1105_spi_send_packed_buf(&priv->spi_setup, SPI_READ,
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
	u64 device_id = priv->spi_setup.device_id;
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
	u64 device_id = priv->spi_setup.device_id;
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
	u64 device_id = priv->spi_setup.device_id;
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
	if (ptpclkval == 0) {
		/* XXX: Find out why this is happening */
		dev_err(&priv->spi_dev->dev, "ptp_gettime returned zero!\n");
		rc = -EAGAIN;
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
		dev_err(&priv->spi_dev->dev, "Failed configuring set mode for ptp clk\n");
		goto out;
	}
	rc = sja1105_ptp_clk_write(priv, ts);
out:
	return rc;
}

/* Write to PTPCLKRATE */
static int sja1105_ptp_adjfine(struct ptp_clock_info *ptp, long scaled_ppm)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	u64 device_id = priv->spi_setup.device_id;
	u64 ptpclkrate_addr;
	u64 ptpclkrate;

	if (IS_ET(device_id))
		ptpclkrate_addr = SJA1105ET_PTPCLKRATE_ADDR;
	else
		ptpclkrate_addr = SJA1105PQRS_PTPCLKRATE_ADDR;

	if (scaled_ppm < 0)
		ptpclkrate = (1ull << 31) & (-scaled_ppm);
	else
		ptpclkrate = (1ull << 31) | scaled_ppm;

	return sja1105_ptp_write_reg(priv, ptpclkrate_addr, &ptpclkrate, 4);
}

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
		dev_err(&priv->spi_dev->dev, "failed configuring add mode for ptp clk\n");
		goto out;
	}
	rc = sja1105_ptp_clk_write(priv, &ts);
out:
	return rc;
}

static int sja1105_ptp_enable(struct ptp_clock_info *ptp,
                              struct ptp_clock_request *rq, int on)
{
	struct sja1105_spi_private *priv = container_of(ptp, struct
	                                   sja1105_spi_private, ptp_caps);
	struct timespec64 now;

	if (sja1105_ptp_reset(priv) < 0)
		return -EIO;
	getnstimeofday64(&now);
	return sja1105_ptp_settime(&priv->ptp_caps, &now);
}

static const struct ptp_clock_info sja1105_ptp_caps = {
	.owner     = THIS_MODULE,
	.name      = "SJA1105 PHC",
	.max_adj   = SJA1105_MAX_ADJ,
#if KERNEL_VERSION(4, 9, 0) >= LINUX_VERSION_CODE
	.adjfreq   = sja1105_ptp_adjfreq,
#else
	.adjfine   = sja1105_ptp_adjfine,
#endif
	.adjtime   = sja1105_ptp_adjtime,
	.gettime64 = sja1105_ptp_gettime,
	.settime64 = sja1105_ptp_settime,
	.enable    = sja1105_ptp_enable,
};

int sja1105_ptp_clock_register(struct sja1105_spi_private *priv)
{
	priv->ptp_caps = sja1105_ptp_caps;

	priv->clock = ptp_clock_register(&priv->ptp_caps, &priv->spi_dev->dev);
	if (IS_ERR_OR_NULL(priv->clock))
		return PTR_ERR(priv->clock);

	return 0;
}

void sja1105_ptp_clock_unregister(struct sja1105_spi_private *priv)
{
	if (IS_ERR_OR_NULL(priv->clock))
		return;

	ptp_clock_unregister(priv->clock);
}

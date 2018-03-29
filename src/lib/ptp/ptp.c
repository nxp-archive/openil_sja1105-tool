/******************************************************************************
 * Copyright (c) 2017, NXP Semiconductors
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#include <string.h>
#include <inttypes.h>
#include <errno.h>
/* These are our own include files */
#include <lib/include/ptp.h>
#include <lib/include/gtable.h>
#include <lib/include/status.h>
#include <lib/include/spi.h>
#include <common.h>

#define NSEC_PER_SEC 1000000000LL

static void sja1105_ptp_cmd_access(
		void *buf,
		struct sja1105_ptp_cmd *ptp_cmd,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;
	/* No need to keep this as part of the structure */
	uint64_t valid = 1;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(ptp_cmd, 0, sizeof(*ptp_cmd));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &valid,               31, 31, 4);
	pack_or_unpack(buf, &ptp_cmd->ptpstrtsch, 30, 30, 4);
	pack_or_unpack(buf, &ptp_cmd->ptpstopsch, 29, 29, 4);
	pack_or_unpack(buf, &ptp_cmd->startptpcp, 28, 28, 4);
	pack_or_unpack(buf, &ptp_cmd->stopptpcp,  27, 27, 4);
	pack_or_unpack(buf, &ptp_cmd->resptp,      2,  2, 4);
	pack_or_unpack(buf, &ptp_cmd->corrclk4ts,  1,  1, 4);
	pack_or_unpack(buf, &ptp_cmd->ptpclkadd,   0,  0, 4);
}

void sja1105_ptp_cmd_pack(void *buf,
                          struct sja1105_ptp_cmd *ptp_cmd)
{
	sja1105_ptp_cmd_access(buf, ptp_cmd, 1);
}

void sja1105_ptp_cmd_unpack(void *buf,
                            struct sja1105_ptp_cmd *ptp_cmd)
{
	sja1105_ptp_cmd_access(buf, ptp_cmd, 0);
}

void sja1105_ptp_cmd_show(struct sja1105_ptp_cmd *ptp_cmd)
{
	printf("PTPSTRTSCH %" PRIX64 "\n", ptp_cmd->ptpstrtsch);
	printf("PTPSTOPSCH %" PRIX64 "\n", ptp_cmd->ptpstopsch);
	printf("STARTPTPCP %" PRIX64 "\n", ptp_cmd->startptpcp);
	printf("STOPPTPCP  %" PRIX64 "\n", ptp_cmd->stopptpcp);
	printf("RESPTP     %" PRIX64 "\n", ptp_cmd->resptp);
	printf("CORRCLK4TS %" PRIX64 "\n", ptp_cmd->corrclk4ts);
	printf("PTPCLKADD  %" PRIX64 "\n", ptp_cmd->ptpclkadd);
}

/* Wrapper around sja1105_spi_send_packed_buf() */
int sja1105_ptp_cmd_commit(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_ptp_cmd *ptp_cmd)
{
	const int PTP_CONTROL_ADDR = 0x17;
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];

	sja1105_ptp_cmd_pack(packed_buf, ptp_cmd);
	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CORE_ADDR + PTP_CONTROL_ADDR,
	                                   packed_buf,
	                                   BUF_LEN);
};

int sja1105_ptp_qbv_running(struct sja1105_spi_setup *spi_setup)
{
	const int PTP_CONTROL_ADDR = 0x17;
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	struct  sja1105_ptp_cmd ptp_cmd;
	int rc;

	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + PTP_CONTROL_ADDR,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}
	sja1105_ptp_cmd_unpack(packed_buf, &ptp_cmd);

	if (ptp_cmd.ptpstrtsch == 1) {
		/* Qbv successfully started */
		rc = 0;
	} else if (ptp_cmd.ptpstopsch == 1) {
		/* Qbv is stopped */
		rc = 1;
	} else {
		/* Qbv is probably not configured with PTP clock source */
		rc = -1;
	}
out:
	return rc;
}

int sja1105_ptp_qbv_start(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.ptpstrtsch = 1;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

int sja1105_ptp_qbv_stop(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.ptpstopsch = 1;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

int sja1105_ptp_pin_toggle_start(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.startptpcp = 1;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

int sja1105_ptp_pin_toggle_stop(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.stopptpcp = 1;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

int sja1105_ptp_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.resptp = 1;
	logv("Resetting PTP clock");
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

static inline int
sja1105_ptp_add_mode_set(struct sja1105_spi_setup *spi_setup,
                         enum sja1105_ptp_clk_add_mode mode)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.ptpclkadd = mode;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

/* Wrapper around sja1105_spi_send_int() using SPI_READ
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_read_reg(struct sja1105_spi_setup *spi_setup,
                     uint64_t reg_offset,
                     uint64_t *value,
                     uint64_t size_bytes)
{
	return sja1105_spi_send_int(spi_setup,
	                            SPI_READ,
	                            CORE_ADDR + PTP_ADDR + reg_offset,
	                            value,
	                            size_bytes);
}

/* Wrapper around sja1105_spi_send_int() using SPI_WRITE
 * at CORE_ADDR + reg_offset */
static inline int
sja1105_ptp_write_reg(struct sja1105_spi_setup *spi_setup,
                      uint64_t reg_offset,
                      uint64_t *value,
                      uint64_t size_bytes)
{
	return sja1105_spi_send_int(spi_setup,
	                            SPI_WRITE,
	                            CORE_ADDR + PTP_ADDR + reg_offset,
	                            value,
	                            size_bytes);
}

/* Read PTPTSCLK */
int sja1105_ptp_ts_clk_get(struct sja1105_spi_setup *spi_setup,
                           struct timespec *ts)
{
	uint64_t ptptsclk;
	int rc;

	rc = sja1105_ptp_read_reg(spi_setup,
	                          SJA1105_PTPTSCLK_ADDR,
	                          &ptptsclk, 8);
	if (rc < 0) {
		loge("%s: failed to read ptptsclk", __func__);
		goto out;
	}
	sja1105_ptp_time_to_timespec(ts, ptptsclk);
out:
	return rc;
}

/* Read PTPCLKVAL */
int sja1105_ptp_clk_get(struct sja1105_spi_setup *spi_setup,
                        struct timespec *ts)
{
	uint64_t ptpclkval;
	int rc;

	rc = sja1105_ptp_read_reg(spi_setup,
	                          SJA1105_PTPCLKVAL_ADDR,
	                          &ptpclkval, 8);
	if (rc < 0) {
		loge("%s: failed to read ptpclkval", __func__);
		goto out;
	}
	if (ptpclkval == 0) {
		/* XXX: Find out why this is happening */
		loge("%s: returned zero", __func__);
		rc = -EAGAIN;
		goto out;
	}
	sja1105_ptp_time_to_timespec(ts, ptpclkval);
out:
	return rc;
}

void sja1105_timespec_to_ptp_time(const struct timespec *ts, uint64_t *ptp_time)
{
	*ptp_time = (ts->tv_sec * NSEC_PER_SEC + ts->tv_nsec) / 8;
}

void sja1105_ptp_time_to_timespec(struct timespec *ts, uint64_t ptp_time)
{
	ptp_time *= 8;
	ts->tv_sec  = ptp_time / NSEC_PER_SEC;
	ts->tv_nsec = ptp_time % NSEC_PER_SEC;
}

static inline int
sja1105_ptp_clk_write(struct sja1105_spi_setup *spi_setup,
                      const struct timespec *ts)
{
	uint64_t ptpclkval;
	sja1105_timespec_to_ptp_time(ts, &ptpclkval);
	return sja1105_ptp_write_reg(spi_setup,
	                             SJA1105_PTPCLKVAL_ADDR,
	                             &ptpclkval, 8);
}

/* Write to PTPCLKVAL while PTPCLKADD is 0 */
int sja1105_ptp_clk_set(struct sja1105_spi_setup *spi_setup,
                        const struct timespec *ts)
{
	int rc;

	rc = sja1105_ptp_add_mode_set(spi_setup, PTP_SET_MODE);
	if (rc < 0) {
		loge("failed configuring set mode for ptp clk");
		goto out;
	}
	rc = sja1105_ptp_clk_write(spi_setup, ts);
out:
	return rc;
}

/* Write to PTPCLKVAL while PTPCLKADD is 1 */
int sja1105_ptp_clk_add(struct sja1105_spi_setup *spi_setup,
                        const struct timespec *ts)
{
	int rc;

	rc = sja1105_ptp_add_mode_set(spi_setup, PTP_ADD_MODE);
	if (rc < 0) {
		loge("failed configuring add mode for ptp clk");
		goto out;
	}
	rc = sja1105_ptp_clk_write(spi_setup, ts);
out:
	return rc;
}

/* IEEE 754 (double precision): fractional part is 52 bits.
 * Take most significant 31 bits of that.
 */
#define DOUBLE_FRACTION_AS_UINT32(double_ptr) \
	(uint32_t)(((uint64_t) *double_ptr >> (52 - 31)) & 0x7fffffff)

static inline int
sja1105_ptpclkrate_from_ratio(double ratio, uint32_t *ptpclkrate)
{
	uint64_t *ratio_ptr;
	int rc = 0;

	if (ratio <= 0 || ratio >= 2) {
		loge("ratio %lf outside of range", ratio);
		rc = -ERANGE;
		goto out;
	}
	ratio_ptr = (uint64_t *)(&ratio);
	if (ratio < 1) {
		/* Put fractional part of proper ratio (larger than 1)
		 * into result. Integer part (msb) is zero */
		ratio += 1;
		*ptpclkrate = DOUBLE_FRACTION_AS_UINT32(ratio_ptr);
	} else {
		/* Put fractional part of ratio into result.
		 * Make integer part (msb) 1 */
		*ptpclkrate = DOUBLE_FRACTION_AS_UINT32(ratio_ptr);
		*ptpclkrate |= 0x80000000;
	}
out:
	return rc;
}

/* Write to PTPCLKRATE */
int sja1105_ptp_clk_rate_set(struct sja1105_spi_setup *spi_setup,
                             double ratio)
{
	uint32_t ptpclkrate;
	uint64_t ptpclkrate_ext;
	int rc;

	rc = sja1105_ptpclkrate_from_ratio(ratio, &ptpclkrate);
	if (rc < 0) {
		loge("%s: failed to convert ratio %lf", __func__, ratio);
		return rc;
	}
	ptpclkrate_ext = ptpclkrate;
	return sja1105_ptp_write_reg(spi_setup,
	                             SJA1105_PTPCLKRATE_ADDR,
	                             &ptpclkrate_ext, 4);
}

/* Write to PTPPINST */
int sja1105_ptp_pin_start_time_set(struct sja1105_spi_setup *spi_setup,
                                   const struct timespec *ts)
{
	uint64_t pinst;
	sja1105_timespec_to_ptp_time(ts, &pinst);
	return sja1105_ptp_write_reg(spi_setup,
	                             SJA1105_PTPPINST_ADDR,
	                             &pinst, 8);
}

/* Write to PTPPINDUR */
int sja1105_ptp_pin_duration_set(struct sja1105_spi_setup *spi_setup,
                                 const struct timespec *ts)
{
	uint64_t pindur;
	int rc;

	sja1105_timespec_to_ptp_time(ts, &pindur);
	if (pindur >= UINT32_MAX) {
		loge("%s: provided ts is too large", __func__);
		rc = -1;
		goto out;
	}
	rc = sja1105_ptp_write_reg(spi_setup,
	                           SJA1105_PTPPINDUR_ADDR,
	                           &pindur, 4);
out:
	return rc;
}

/* Write to PTPCLKCORP */
int sja1105_ptp_qbv_correction_period_set(struct sja1105_spi_setup *spi_setup,
                                          const struct timespec *ts)
{
	uint64_t ptpclkcorp;
	int rc;

	sja1105_timespec_to_ptp_time(ts, &ptpclkcorp);
	if (ptpclkcorp >= UINT32_MAX) {
		loge("%s: provided ts is too large", __func__);
		rc = -1;
		goto out;
	}
	rc = sja1105_ptp_write_reg(spi_setup,
	                           SJA1105_PTPCLKCORP_ADDR,
	                           &ptpclkcorp, 4);
out:
	return rc;
}

/* Write to PTPSCHTM */
int sja1105_ptp_qbv_start_time_set(struct sja1105_spi_setup *spi_setup,
                                   const struct timespec *ts)
{
	uint64_t ptpschtm;
	sja1105_timespec_to_ptp_time(ts, &ptpschtm);
	return sja1105_ptp_write_reg(spi_setup,
	                             SJA1105_PTPSCHTM_ADDR,
	                             &ptpschtm, 8);
}

int sja1105_ptp_corrclk4ts_set(struct sja1105_spi_setup *spi_setup,
                               enum sja1105_ptpegr_ts_source source)
{
	struct sja1105_ptp_cmd ptp_cmd;

	memset(&ptp_cmd, 0, sizeof(ptp_cmd));
	ptp_cmd.corrclk4ts = source;
	return sja1105_ptp_cmd_commit(spi_setup, &ptp_cmd);
}

int sja1105_ptpegr_ts_poll(struct sja1105_spi_setup *spi_setup,
                           enum sja1105_ptpegr_ts_source source,
                           int port, int ts_regid,
                           struct timespec *ts)
{
	const int ts_reg_index = 2 * port + ts_regid;
	const int SIZE_PTPEGR_TS = 4;
	uint8_t   packed_buf[SIZE_PTPEGR_TS];
	uint64_t  ptpegr_ts_reconstructed;
	uint64_t  ptpegr_ts_partial;
	uint64_t  ptpegr_ts_mask;
	uint64_t  ptp_full_current_ts;
	uint64_t  ptpclk_addr;
	uint64_t  update;
	int       rc;

	if (source == TS_PTPCLK) {
		ptpclk_addr = SJA1105_PTPCLKVAL_ADDR;
	} else if (source == TS_PTPTSCLK) {
		ptpclk_addr = SJA1105_PTPTSCLK_ADDR;
	} else {
		loge("%s: invalid source selection: %d", __func__, source);
		rc = -1;
		goto out;
	}
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + 0xC0 + ts_reg_index,
	                                 packed_buf,
	                                 SIZE_PTPEGR_TS);
	if (rc < 0) {
		loge("failed to read ptp egress timestamp register %d",
		     ts_reg_index);
		goto out;
	}
	gtable_unpack(packed_buf, &ptpegr_ts_partial, 31, 8, SIZE_PTPEGR_TS);
	gtable_unpack(packed_buf, &update,             0, 0, SIZE_PTPEGR_TS);

	if (!update) {
		/* No update. Keep trying, you'll make it someday. */
		rc = -1;
		goto out;
	}
	rc = sja1105_ptp_read_reg(spi_setup, ptpclk_addr,
	                          &ptp_full_current_ts, 8);
	if (rc < 0) {
		loge("failed to read ptpclkval/ptptsclk");
		goto out;
	}
	ptpegr_ts_mask = (1ull << 24ull) - 1;
	ptpegr_ts_reconstructed = (ptp_full_current_ts & ~ptpegr_ts_mask) |
	                           ptpegr_ts_partial;
	/* Check if wraparound occurred between moment when the partial
	 * ptpegr timestamp was generated, and the moment when that
	 * timestamp is being read out (now, ptpclkval/ptptsclk).
	 * If last 24 bits of current ptpclkval/ptptsclk time are lower
	 * than the partial timestamp, then wraparound surely occurred.
	 * Otherwise, we'll never know...
	 **/
	if ((ptp_full_current_ts & ptpegr_ts_mask) <= ptpegr_ts_partial) {
		ptpegr_ts_reconstructed -= (1ull << 24ull);
	}
	sja1105_ptp_time_to_timespec(ts, ptpegr_ts_reconstructed);
out:
	return rc;
}


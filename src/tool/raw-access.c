/*
 * raw-access.c
 *
 *  Created on: Mar 28, 2018
 *      Author: tescott
 */
#include "internal.h"
#include <string.h>
#include <lib/include/status.h>
#include <lib/include/spi.h>

static void print_usage()
{
	printf("Usage: sja1105-tool reg address [write_value] : Read or write register\n");
	printf("       sja1105-tool reg dump address count    : incrementally dump registers starting at the given address\n");
}

int sja1105_reg_get(struct sja1105_spi_setup *spi_setup, uint32_t address, uint32_t *pVal,uint32_t size)
{
	int rc;
	uint8_t buf[4];
	uint8_t *p = buf;

	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 address,
	                                 buf,
	                                 size);
	if (rc < 0) {
		loge("spi read failed");
		goto out;
	}

	for (*pVal = 0; size > 0; size--)
	{
		*pVal = (*pVal << 8) | *p;
		p++;
	}
out:
	return rc;
}

int sja1105_reg_set(struct sja1105_spi_setup *spi_setup, uint32_t address, uint8_t *buf, uint32_t size)
{
	int rc;

	rc = sja1105_spi_send_packed_buf(spi_setup,
									 SPI_WRITE,
	                                 address,
	                                 buf,
	                                 size);
	if (rc < 0) {
		loge("spi write failed");
		goto out;
	}

out:
	return rc;
}

int reg_parse_args(struct sja1105_spi_setup *spi_setup,
                      int argc, char **argv)
{
	uint32_t dwAddress;
	uint32_t dwData;
	uint32_t dwCount;
	uint32_t dwSize = 4;
	int rc = 0;
	uint32_t i;

	if (argc < 1) {
		rc = -1;
		goto parse_error;
	}

	if (strcmp(argv[0],"dump") == 0)
	{
		argc--;argv++; // consume the 'dump' parm
		if (argc == 2)
		{
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			if ((sscanf(argv[0],"%x",&dwAddress) == 1) && (sscanf(argv[1],"%d",&dwCount) == 1))
			{
				for (i = 0; i < dwCount; i++)
				{
					rc = sja1105_reg_get(spi_setup,dwAddress + i,&dwData,dwSize);
					if (rc < 0)
					{
						loge("register address not read");
						goto error;
					}
					printf("0x%08X: %08X\n",dwAddress + i,dwData);
				}
			}
			else
			{
				loge("invalid address / count");
				goto error;
			}
		}
		else
		{
			loge("invalid parms");
			goto error;
		}
	}
	else if (argc == 1)
	{
		// perform a read...
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}

		if (sscanf(argv[0],"%x",&dwAddress) == 1)
		{
			rc = sja1105_reg_get(spi_setup,dwAddress,&dwData,dwSize);
			if (rc < 0)
			{
				loge("register address not read");
				goto error;
			}
			printf("0x%08X: %08X\n",dwAddress,dwData);
		}
		else
		{
			loge("register address not read");
			goto error;
		}
	}
	else if (argc == 2)
	{
		// perform a write
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}

		if ((sscanf(argv[0],"%x",&dwAddress) == 1) && (sscanf(argv[1],"%x",&dwData) == 1))
		{
			uint8_t buf[4];

			if (dwSize == 1)
			{
				buf[0] = (uint8_t)dwData;
			}
			else if (dwSize == 2)
			{
				buf[0] = (uint8_t)(dwData >> 8);
				buf[1] = (uint8_t)(dwData);
			}
			else
			{
				buf[0] = (uint8_t)(dwData >> 24);
				buf[1] = (uint8_t)(dwData >> 16);
				buf[2] = (uint8_t)(dwData >> 8);
				buf[3] = (uint8_t)(dwData);
			}

			rc = sja1105_reg_set(spi_setup,dwAddress,buf,dwSize);
			if (rc < 0)
			{
				loge("register address not set");
				goto error;
			}
		}
		else
		{
			loge("register address not read");
			goto error;
		}
	}
	else
	{
		loge("command line not understood");
		goto error;
	}
	return 0;

parse_error:
	print_usage();
error:
	return rc;
}


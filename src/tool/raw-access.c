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
#include <inttypes.h>

static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool reg <address> [<write_value>] :"
	       "Read or write register\n");
	printf(" * sja1105-tool reg dump <address> <count>    :"
	       "Incrementally dump registers starting at the given address\n");
}

int reg_parse_args(struct sja1105_spi_setup *spi_setup,
                      int argc, char **argv)
{
	struct reg_cmd {
		uint64_t address;
		uint64_t data;
		uint64_t count;
		uint64_t size;
	} reg_cmd;
	int rc = 0;
	uint64_t i;

	if (argc < 1) {
		rc = -EINVAL;
		goto out_parse_error;
	}

	/* Make this assumption for now */
	reg_cmd.size = 4;
	if (matches(argv[0], "dump") == 0) {
		/* consume the 'dump' parameter */
		argc--; argv++;
		if (argc != 2) {
			loge("Please supply 2 parameters.");
			goto out_parse_error;
		}
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto out_spi_configure_failed;
		}
		rc = reliable_uint64_from_string(&reg_cmd.address,
		                                 argv[0], NULL);
		if (rc < 0) {
			loge("could not read address param %s", argv[0]);
			goto out_parse_error;
		}
		rc = reliable_uint64_from_string(&reg_cmd.count,
		                                 argv[1], NULL);
		if (rc < 0) {
			loge("could not read count param %s", argv[1]);
			goto out_parse_error;
		}
		for (i = 0; i < reg_cmd.count; i++) {
			rc = sja1105_spi_send_int(spi_setup,
			                          SPI_READ,
			                          reg_cmd.address + i,
			                          &reg_cmd.data,
			                          reg_cmd.size);
			if (rc < 0) {
				loge("register address not read");
				goto out_read_failed;
			}
			printf("0x%08" PRIx64 ": %08" PRIx64 "\n",
			       reg_cmd.address + i, reg_cmd.data);
		}
	} else if (argc == 1) {
		// perform a read...
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto out_spi_configure_failed;
		}

		rc = reliable_uint64_from_string(&reg_cmd.address,
		                                 argv[0], NULL);
		if (rc < 0) {
			loge("invalid register address at %s", argv[0]);
			goto out_parse_error;
		}
		rc = sja1105_spi_send_int(spi_setup,
		                          SPI_READ,
		                          reg_cmd.address,
		                          &reg_cmd.data,
		                          reg_cmd.size);
		if (rc < 0) {
			loge("Failed to read from address %" PRIx64,
			     reg_cmd.address);
			goto out_read_failed;
		}
		printf("0x%08" PRIx64 ": %08" PRIx64 "\n",
		       reg_cmd.address, reg_cmd.data);
	} else if (argc == 2) {
		// perform a write
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto out_spi_configure_failed;
		}

		rc = reliable_uint64_from_string(&reg_cmd.address,
		                                 argv[0], NULL);
		if (rc < 0) {
			loge("could not read address param %s", argv[0]);
			goto out_parse_error;
		}
		rc = reliable_uint64_from_string(&reg_cmd.data,
		                                 argv[1], NULL);
		if (rc < 0) {
			loge("could not read data param %s", argv[1]);
			goto out_parse_error;
		}
		rc = sja1105_spi_send_int(spi_setup,
		                          SPI_WRITE,
		                          reg_cmd.address,
		                          &reg_cmd.data,
		                          reg_cmd.size);
		if (rc < 0) {
			loge("Could not write data %" PRIx64
			     " at address %" PRIx64,
			     reg_cmd.data, reg_cmd.address);
			goto out_write_failed;
		}
	} else {
		loge("command line not understood");
		goto out_parse_error;
	}
	return 0;

out_parse_error:
	print_usage();
out_spi_configure_failed:
out_write_failed:
out_read_failed:
	return rc;
}


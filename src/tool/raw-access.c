/*
 * raw-access.c
 *
 *  Created on: Mar 28, 2018
 *      Author: tescott
 */
#include "internal.h"
#include <string.h>
#include <inttypes.h>

static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool reg <address> [<write_value>] :"
	       "Read or write register\n");
	printf(" * sja1105-tool reg dump <address> <count>    :"
	       "Incrementally dump registers starting at the given address\n");
}


static int write_register(struct sja1105_spi_setup *spi_setup,
                         uint64_t address, uint64_t value)
{
	int rc;
	int len;
	char buf[80];

	len = snprintf(buf, sizeof(buf), "0x%" PRIx64 " 0x%" PRIx64,
	               address, value) + 1;
	rc = sysfs_write(spi_setup, "reg_access", buf, len);

	return (rc == len) ? 0 : -1;
}

static int read_register(struct sja1105_spi_setup *spi_setup,
                         uint64_t address, uint64_t *value)
{
	int rc;
	int len;
	char buf[80];
	char *next_ptr;
	uint64_t read_address;

	len = snprintf(buf, sizeof(buf), "0x%" PRIx64, address) + 1;
	rc = sysfs_write(spi_setup, "reg_access", buf, len);
	if (rc != len) {
		rc = -1;
		goto out;
	}

	rc = sysfs_read(spi_setup, "reg_access", buf, sizeof(buf));
	if (rc <= 0) {
		rc = -1;
		goto out;
	}

	rc = reliable_uint64_from_string(&read_address, buf, &next_ptr);
	if ((rc < 0) || (read_address != address)) {
		logv("%s: could not read back address", __FUNCTION__);
		goto out;
	}

	rc = reliable_uint64_from_string(value, next_ptr, NULL);
	if (rc < 0) {
		logv("%s: could not read value", __FUNCTION__);
		goto out;
	}

out:
	return rc;
}


int reg_parse_args(struct sja1105_spi_setup *spi_setup,
                      int argc, char **argv)
{
	struct reg_cmd {
		uint64_t address;
		uint64_t data;
		uint64_t count;
	} reg_cmd;
	int rc = 0;
	uint64_t i;

	if (argc < 1) {
		rc = -EINVAL;
		goto out_parse_error;
	}

	if (matches(argv[0], "dump") == 0) {
		/* consume the 'dump' parameter */
		argc--; argv++;
		if (argc != 2) {
			loge("Please supply 2 parameters.");
			goto out_parse_error;
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
			rc = read_register(spi_setup, reg_cmd.address,
			                   &reg_cmd.data);
			if (rc < 0) {
				loge("register address not read");
				goto out_read_failed;
			}
			printf("0x%08" PRIx64 ": %08" PRIx64 "\n",
			       reg_cmd.address + i, reg_cmd.data);
		}
	} else if (argc == 1) {
		// perform a read...
		rc = reliable_uint64_from_string(&reg_cmd.address,
		                                 argv[0], NULL);
		if (rc < 0) {
			loge("invalid register address at %s", argv[0]);
			goto out_parse_error;
		}

		rc = read_register(spi_setup, reg_cmd.address, &reg_cmd.data);
		if (rc < 0) {
			loge("Failed to read from address %" PRIx64,
			     reg_cmd.address);
			goto out_read_failed;
		}
		printf("0x%08" PRIx64 ": %08" PRIx64 "\n",
		       reg_cmd.address, reg_cmd.data);
	} else if (argc == 2) {
		// perform a write
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
		rc = write_register(spi_setup, reg_cmd.address, reg_cmd.data);
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
out_write_failed:
out_read_failed:
	return rc;
}


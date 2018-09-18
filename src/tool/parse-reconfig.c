/*
 * parse-reconfig.c
 *
 *  Created on: Sep 18, 2018
 *      Author: Georg Waibel
 */
#include "internal.h"
#include <string.h>
#include <lib/include/status.h>
#include <lib/include/spi.h>
#include <lib/include/dynamic-config.h>
#include <inttypes.h>

static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool reconfig speed <port> <speed>: "
	       "Set a port's (0..4) link speed (10, 100, 1000)\n");
}

int reconfig_parse_args(struct sja1105_spi_setup *spi_setup,
                      int argc, char **argv)
{
	uint64_t port;
	uint64_t speed_mbps;
	uint64_t speed;
	struct sja1105_staging_area staging_area;
	struct sja1105_mac_config_entry mac_entry;
	int rc = 0;

	if (argc < 3) {
		rc = -EINVAL;
		goto out_parse_error;
	}

	if (matches(argv[0], "speed") == 0) {
		rc = reliable_uint64_from_string(&port, argv[1], NULL);
		if (rc < 0) {
			loge("could not read port param %s", argv[1]);
			goto out_parse_error;
		}
		if (port >= 5) {
			rc = -EINVAL;
			loge("invalid port %s", argv[1]);
			goto out_parse_error;
		}
		rc = reliable_uint64_from_string(&speed_mbps, argv[2], NULL);
		if (rc < 0) {
			loge("could not read speed param %s", argv[2]);
			goto out_parse_error;
		}
		switch (speed_mbps)
		{
		case 10:
			speed = 3;
			break;
		case 100:
			speed = 2;
			break;
		case 1000:
			speed = 1;
			break;
		default:
			rc = -EINVAL;
			loge("invalid speed %s", argv[2]);
			goto out_parse_error;
			break;
		}

		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto out_spi_configure_failed;
		}

		rc = staging_area_load(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			loge("loading staging area failed");
			goto out_read_config_failed;
		}

		/* Check if speed in staic config is 0 */
		if (staging_area.static_config.mac_config[port].speed != 0) {
			rc = -EINVAL;
			loge("error, static speed config shall be 0, but is %u",
			     (unsigned)staging_area.static_config.mac_config[port].speed);
			goto out_static_config_missmatch;
		}

		/* Read, modify and write mac config table */
		rc = sja1105_mac_config_get(spi_setup, &mac_entry, port);
		if (rc < 0) {
			goto out_read_failed;
		}
		logv("Setting MAC speed for port %u: Speed value before: %u, now: %u",
		     (unsigned)port, (unsigned)mac_entry.speed,
		     (unsigned)speed);
		mac_entry.speed = speed;
		rc = sja1105_mac_config_set(spi_setup, &mac_entry, port);
		if (rc < 0) {
			goto out_read_failed;
		}

		/* Configure the CGU (PHY link modes and speeds) */
		rc = sja1105_clocking_setup_port(spi_setup, port,
		                     &staging_area.static_config.xmii_params[0],
		                     &mac_entry);
		if (rc < 0) {
			loge("sja1105_clocking_setup failed");
			goto out_write_failed;
		}
	} else {
		rc = -EINVAL;
		loge("command line not understood");
		goto out_parse_error;
	}

	return 0;

out_parse_error:
	print_usage();
out_static_config_missmatch:
out_spi_configure_failed:
out_read_config_failed:
out_write_failed:
out_read_failed:
	return rc;
}

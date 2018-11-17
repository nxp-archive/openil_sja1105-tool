/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018, Sensor-Technik Wiedemann GmbH
 */
#include <linux/device.h>
#include "sja1105.h"
#include "common.h"

/*
 * Parse the string to find a specific port.
 * The port can be specified by ist port number (the chip's port number) or
 * by the name of the net device that is assigned to this port/PHY.
 * Return NULL if port is not found ind priv->port_list_head.
 */
static struct sja1105_port *sja1105_parse_port(struct sja1105_spi_private *priv,
                                               const char *buf)
{
	struct sja1105_port *port = NULL;
	struct list_head *pos, *q;
	int port_index;
	int found_port = 0;

	list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
		port = list_entry(pos, struct sja1105_port, list);
		if (sysfs_streq(buf, port->net_dev->name)) {
			/* net device name match */
			found_port = 1;
		}
		else if (!kstrtoint(buf, 0, &port_index) &&
			 (port_index == port->index)) {
			/* port number match */
			found_port = 1;
		}

		if (found_port)
			break;
	}

	return port;
}

/*
 * Read a "space" separated number of values from "buf" and copy it to the
 * "values" array. Maximum "num" values are parsed.
 * Return: >=0: Number of values read from buf
 *          -1: Format error
 */
static int sja1105_parse_string_values(const char* buf, u64 *values, int num)
{
	int rc = -1;
	int i, count = 0;
	char *str, *curr, *next, *end;

	str = kstrdup(buf, GFP_KERNEL);
	if (!str)
		return -1;

	curr = strim(str);
	end  = str + strlen(str);
	for (i = 0; i < num; i++) {
		next = strchr(curr, ' ');
		if (next)
			*next++ = '\0'; /* make sure curr is \0 terminated
			                 * else kstrtou64 fails */
		rc = kstrtou64(curr, 0, &values[i]);
		if (rc == 0)
			count++;
		if ((rc < 0) || (next == NULL))
			break;
		curr = strim(next);
	}

	kfree(str);
	return (rc) ? -1 : count;
}

/*
 * Parse the string written to sysfs attribute reg_access.
 * Format: "address" => read operation, value read on sysfs attribure read
 *         "address value" => write operation
 * Return: 1: got an address
 *         2: got address and value
 *        -1: error
 */
static int sja1105_reg_access_parse_input(const char* buf,
                                          u64 *addr, u64 *value)
{
	int rc;
	u64 params[2];

	rc = sja1105_parse_string_values(buf, params, 2);
	if (rc == 1) {
		*addr = params[0];
	}
	else if (rc == 2) {
		*addr = params[0];
		*value = params[1];
	}
	else {
		rc = -1;
	}

	return rc;
}

/*
 * Parse the string written to sysfs attribute vlan_lookup.
 * Format: "vlanid" => read operation, value read on sysfs attribure read
 *         "ving_mirr vegr_mirr vmemb_port vlan_bc tag_port vlanid valident"
 *                  => write operation
 * Return: 1: got a vlinaid -> read
 *         2: got a new entry -> write
 *        -1: error
 */
static int sja1105_vlan_lookup_parse_input(const char* buf,
                                        struct sja1105_vlan_lookup_entry *entry,
                                        int *valident)
{
	int rc;
	u64 values[7];

	rc = sja1105_parse_string_values(buf, values, 7);
	if (rc == 1) {
		entry->vlanid = values[0];
	}
	else if (rc == 7) {
		entry->ving_mirr  = values[0];
		entry->vegr_mirr  = values[1];
		entry->vmemb_port = values[2];
		entry->vlan_bc    = values[3];
		entry->tag_port   = values[4];
		entry->vlanid     = values[5];
		*valident         = values[6];
		rc = 2;
	}
	else {
		rc = -1;
	}

	return rc;
}

static ssize_t sja1105_sysfs_rd(struct device *dev,
                                struct device_attribute *attr,
                                char *buf);

static ssize_t sja1105_sysfs_wr(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count);

static DEVICE_ATTR(device_id,         S_IRUGO,
                   sja1105_sysfs_rd,  NULL);
static DEVICE_ATTR(general_status,    S_IRUGO,
                   sja1105_sysfs_rd,  NULL);
static DEVICE_ATTR(port_status_clear, S_IWUSR,
                   NULL,              sja1105_sysfs_wr);
static DEVICE_ATTR(port_mapping,      S_IRUGO,
                   sja1105_sysfs_rd,  NULL);
static DEVICE_ATTR(reg_access,        S_IRUGO | S_IWUSR,
                   sja1105_sysfs_rd,  sja1105_sysfs_wr);
static DEVICE_ATTR(config_upload,     S_IWUSR,
                   NULL,              sja1105_sysfs_wr);
static DEVICE_ATTR(vlan_lookup,       S_IRUGO | S_IWUSR,
                   sja1105_sysfs_rd,  sja1105_sysfs_wr);

static ssize_t sja1105_sysfs_wr(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count)
{
	int rc = -EOPNOTSUPP;
	struct spi_device *spi = to_spi_device(dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_port *port = NULL;
	u64 addr, value;
	int port_no;

	mutex_lock(&priv->lock);

	if (attr == &dev_attr_config_upload) {
		if (buf[0] != '1')
			goto out_error;

		/* Load firmware from user space (rootfs) and unpack it */
		rc = sja1105_load_firmware(priv);
		if (rc)
			goto out_error;

		/* Upload static configuration */
		rc = sja1105_static_config_flush(priv);
		if (rc < 0)
			goto out_error;
		dev_info(dev, "Uploaded static configuration to device\n");

		/* TODO: Resetting / Reinit of PHYs required ? */
		rc = count;
	} else if (attr == &dev_attr_port_status_clear) {
		if (sysfs_streq(buf, "all")) {
			port_no = -1;
			rc = 0;
		} else {
			rc = -ENOENT;
			port = sja1105_parse_port(priv, buf);
			if (port) {
				port_no = port->index;
				rc = 0;
			}
		}

		if (rc == 0) {
			rc = sja1105_port_status_clear(priv,
							port_no);
			rc = (rc) ? -EIO : count;
		}
	} else if (attr == &dev_attr_reg_access) {
		rc = sja1105_reg_access_parse_input(buf, &addr, &value);
		if (rc == 1) {
			/* read operation => remember address and read
			 * on sysfs read operation */
			dev_info(dev, "Prepare reading of register 0x%08X\n",
			         (u32)addr);
			priv->reg_addr = addr;
			rc = count;
		} else if (rc == 2) {
			/* write operation */
			rc = sja1105_spi_send_int(priv, SPI_WRITE,
			                           addr, &value, 4);
			dev_info(dev, "Writing value 0x%08X to register 0x%08X, rc=%i\n",
			         (u32)value, (u32)addr, rc);
			rc = (rc) ? rc : count;
		} else {
			dev_err(dev, "Invalid parameter written to \"reg_access\"\n");
			rc = -EINVAL;
		}
	}
	else if (attr == &dev_attr_vlan_lookup) {
		int valident;
		struct sja1105_vlan_lookup_entry entry;

		rc = sja1105_vlan_lookup_parse_input(buf, &entry,
		                                     &valident);
		if (rc == 1) {
			/* read operation => remember the entries vid and
			 * on sysfs read operation */
			dev_info(dev, "Prepare reading of vlan lookup entry with vid 0x%X\n",
			         (u32)entry.vlanid);
			priv->vlanid = entry.vlanid;
			rc = count;
		}
		else if (rc == 2) {
			/* write operation */
			rc = sja1105_vlan_lookup_set(priv,
			                             &entry,
			                             valident);
			dev_info(dev, "Writing vlan lookup entry with vid 0x%X, rc=%i\n",
			         (u32)entry.vlanid, rc);
			rc = (rc) ? rc : count;
		}
		else {
			dev_err(dev, "Invalid parameter written to \"lookup_entry\"\n");
			rc = -EINVAL;
		}
	}

out_error:
	mutex_unlock(&priv->lock);
	return rc;
}

static ssize_t sja1105_sysfs_rd(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
	int  rc = -EOPNOTSUPP;
	struct spi_device *spi = to_spi_device(dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_general_status gen_status;
	struct list_head *pos, *q;
	struct sja1105_port *port = NULL;
	u64 value;

	mutex_lock(&priv->lock);

	if (attr == &dev_attr_device_id) {
		const char *name;

		name = sja1105_device_id_string_get(priv->device_id,
		                                    priv->part_nr);
		rc = snprintf(buf, PAGE_SIZE,
		              "device id:   0x%08X\n"
		              "part number: 0x%08X\n"
		              "name:        %s\n",
		              (u32)priv->device_id,
		              (u32)priv->part_nr,
		              name);
	} else if (attr == &dev_attr_general_status) {
		rc = sja1105_general_status_get(priv, &gen_status);
		if (rc) {
			rc = -EIO;
			goto err_out;
		}
		sja1105_general_status_show(&gen_status, buf, PAGE_SIZE,
		                            priv->device_id);
		rc = strlen(buf);
	} else if (attr == &dev_attr_port_mapping) {
		list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
			port = list_entry(pos, struct sja1105_port, list);
			formatted_append(buf, PAGE_SIZE, "%s\n", "%s %i",
			                 port->net_dev->name, port->index);
		}
		rc = strlen(buf);
	} else if (attr == &dev_attr_reg_access) {
		/* read a register value and return in format "address value" */
		rc = sja1105_spi_send_int(priv, SPI_READ,
		                          priv->reg_addr, &value, 4);
		dev_info(dev, "Reading register 0x%08X 0x%08X, rc=%i\n",
			 (u32)priv->reg_addr, (u32)value, rc);
		if (rc)
			goto err_out;

		rc = snprintf(buf, PAGE_SIZE, "0x%08X 0x%08X\n",
		              (u32)priv->reg_addr, (u32)value);
	}
	else if (attr == &dev_attr_vlan_lookup) {
		struct sja1105_vlan_lookup_entry entry;

		/* read a vlan lookup table entry */
		entry.vlanid = priv->vlanid;
		rc = sja1105_vlan_lookup_get(priv,
		                             &entry);
		dev_info(dev, "Reading vlan lookup table entry 0x%X, rc=%i\n",
			 (u32)priv->vlanid, rc);
		if (rc)
			goto err_out;

		// Print members in same order as in vlan lookup table
		rc = snprintf(buf, PAGE_SIZE, "0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n",
		              (u32)entry.ving_mirr, (u32)entry.vegr_mirr,
		              (u32)entry.vmemb_port, (u32)entry.vlan_bc,
		              (u32)entry.tag_port, (u32)entry.vlanid);
	}

err_out:
	mutex_unlock(&priv->lock);
	return rc;
}

int sja1105_sysfs_init(struct sja1105_spi_private *priv)
{
	int rc;
	struct device *dev = &priv->spi_dev->dev;

	rc  = device_create_file(dev, &dev_attr_device_id);
	rc |= device_create_file(dev, &dev_attr_general_status);
	rc |= device_create_file(dev, &dev_attr_port_status_clear);
	rc |= device_create_file(dev, &dev_attr_port_mapping);
	rc |= device_create_file(dev, &dev_attr_reg_access);
	rc |= device_create_file(dev, &dev_attr_vlan_lookup);
	rc |= device_create_file(dev, &dev_attr_config_upload);

	return (rc) ? -1 : 0;
}

void sja1105_sysfs_remove(struct sja1105_spi_private *priv)
{
	struct device *dev = &priv->spi_dev->dev;

	device_remove_file(dev, &dev_attr_device_id);
	device_remove_file(dev, &dev_attr_general_status);
	device_remove_file(dev, &dev_attr_port_status_clear);
	device_remove_file(dev, &dev_attr_port_mapping);
	device_remove_file(dev, &dev_attr_reg_access);
	device_remove_file(dev, &dev_attr_vlan_lookup);
	device_remove_file(dev, &dev_attr_config_upload);
}

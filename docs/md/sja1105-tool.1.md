% sja1105-tool(1) | SJA1105-TOOL

NAME
====

sja1105-tool - NXP SJA1105 Configuration Tool

SYNOPSIS
========

**sja1105-tool** _VERB_ \[_OPTIONS_\]

_VERB_ := { config | status | reset }

DESCRIPTION
===========

The sja1105-tool is a Linux userspace application for configuring the NXP
SJA1105 Automotive Ethernet L2 switch.

The communication with the SJA1105 is done over SPI, through the character
device exported by the kernel at _/dev/spidevX.Y_.

All options are specified to sja1105-tool through _/etc/sja1105/sja1105.conf_,
which is read on each command invocation.

The tool supports:

  * Importing a configuration for the SJA1105 switch from an XML file
  * Exporting the current SJA1105 configuration as an XML file
  * Uploading the current SJA1105 configuration to the switch through its SPI
    interface
  * On-the-fly modification of the current SJA1105 configuration through command
    line or scripting interface
  * Inspecting the current SJA1105 configuration
  * Inspecting the current SJA1105 status
  * Resetting the SJA1105 switch

FILES
=====

_/etc/sja1105/sja1105.conf_ is the configuration file for sja1105-tool.

_/etc/sja1105/standard-config.xml_ is a sample XML description of a basic SJA1105 switch configuration.

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-conf(5),
sja1105-tool-config-format(5),
sja1105-tool-config(1),
sja1105-tool-status(1),
sja1105-tool-reset(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


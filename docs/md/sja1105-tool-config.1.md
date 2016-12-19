% sja1105-tool-config(1) | SJA1105-TOOL

NAME
====

sja1105-tool-config - Configuration command of NXP sja1105-tool

SYNOPSIS
========

**sja1105-tool** config _ACTION_ \[_OPTIONS_\]

**sja1105-tool** config show \[_`TABLE_NAME`_\]

**sja1105-tool** config default _`BUILTIN_CONFIG`_

**sja1105-tool** config upload

**sja1105-tool** config save _`XML_FILE`_

**sja1105-tool** config load _`XML_FILE`_

**sja1105-tool** config hexdump

**sja1105-tool** config new

**sja1105-tool** config modify _`TABLE_NAME`_\[_`ENTRY_INDEX`_\]
                 _`FIELD_NAME`_ _`FIELD_NEW_VALUE`_

_ACTION_ := { show | default | upload | save | load | hexdump | new | modify }

_`BUILTIN_CONFIG`_ := { ls1021atsn-standard | ls1021atsn-policing | ls1021atsn-scheduling }

_`TABLE-NAME`_ := { schedule-table | schedule-entry-points-table | vl-lookup-table |
                      vl-policing-table | vl-forwarding-table | l2-address-lookup-table |
                      l2-policing-table | vlan-lookup-table | l2-forwarding-table |
                      mac-configuration-table | schedule-parameters-table |
                      schedule-entry-points-parameters-table | vl-forwarding-parameters-table |
                      l2-address-lookup-parameters-table | l2-address-lookup-parameters-table |
                      l2-forwarding-parameters-table | clock-synchronization-parameters-table |
                      avb-parameters-table | general-parameters-table | retagging-table |
                      xmii-mode-parameters-table }

DESCRIPTION
===========

Since the actual configuration area of the SJA1105 switch is write-only,
the sja1105-tool does not actually retrieve it from the switch over SPI.
Instead, all config operations work with a local "staging area" (a file)
where a binary form of the switch configuration is held.

Modifying and inspecting the configuration operate on this staging area,
which is local to the host processor. The staging area is only uploaded
("committed") to the switch via the "**sja1105-tool config upload**" command.

The location of the staging area is specified in **/etc/sja1105/sja1105.conf**,
property "staging-area" under the \[spi-setup\] section.


ACTIONS
=======

show \[_`TABLE_NAME`_\]

:   - Read the configuration stored in the staging area, and display it to
      stdout in a human-readable form, compatible with the interpretation
      found in UM10944.pdf.

    - If no TABLE_NAME is specified then the whole switch configuration is
      displayed.

    - Each entry of the table is displayed on its own column to make better
      use of screen space (width). The "screen-width" and "entries-per-line"
      properties under the \[general\] section of **/etc/sja1105/sja1105.conf**
      are taken into account for this operation.

default _ls1021atsn-standard_ | _ls1021atsn-policing_ | _ls1021atsn-scheduling_

:   - These three configurations are builtin into the sja1105-tool. They are
      only guaranteed to provide a meaningful configuration for the LS1021ATSN
      board. The builtin configurations are saved to the staging area.

upload

:   - Read the configuration stored in the staging area, packetize it in 260-byte
      messages and "commit" (send) it over SPI to the SJA1105 switch.

    - The SJA1105 switch needs a warm reset prior to this operation. The reset
      command is issued automatically before sending the configuration.

    - After the configuration is sent to the switch, a proper Clock Generation
      Unit (CGU, see chapter 5.3 in UM10944.pdf) configuration is determined
      from the xMII Mode Parameters Table present in the staging area. The CGU
      configuration is programmed automatically at the end of this command.

save _`XML_FILE`_

:   - Read the configuration stored in the staging area and export it in a
      human-readable form to the _`XML_FILE`_ specified.

load _`XML_FILE`_

:   - Import the SJA1105 switch configuration stored in the _`XML_FILE`_ specified,
      and write it to the staging area.

hexdump

:   - Read the configuration stored in the staging area, and display a hexdump
      interpretation to stdout. Individual entries of each configuration tables
      are identified and separated according to their table headers.

new

:   - Write an empty SJA1105 switch configuration to the staging area.

modify _`TABLE_NAME`_\[_`ENTRY_INDEX`_\] _`FIELD_NAME`_ _`FIELD_NEW_VALUE`_

:   - Change the entry _`ENTRY_INDEX`_ of _`TABLE_NAME`_: set _`FIELD_NAME`_
      to _`FIELD_NEW_VALUE`_.

    - _`ENTRY_INDEX`_ must be specified in square brackets right next to
      _`TABLE_NAME`_  (no spaces in between). If `ENTRY_INDEX` is not specified
      it is assumed to be zero.

    - _`ENTRY_INDEX`_ must be larger or equal to zero, and strictly smaller than
      "entry-count" of _`TABLE_NAME`_.

    - The possibilities for _`FIELD_NAME`_ are unique to each _`TABLE_NAME`_
      and are listed in UM10944.pdf.

    - Additionally, if _`FIELD_NAME`_ is "entry-count", then _`TABLE_NAME`_ is
      resized to have _`FIELD_NEW_VALUE`_ entries. All the fields of a new entry
      are set to zero. Erasing a configuration table may be done by setting
      the "entry-count" to zero and then back to its original value.

    - If _`FIELD_NAME`_ points to an array, then _`FIELD_NEW_VALUE`_ should be
      specified to the sja1105-tool enclosed in quotes. This prevents the shell
      from interpreting array elements as separate parameters.

BUGS
====

Showing a single entry of a configuration table is currently not supported.

The entry count of configuration tables is not checked to be consistent with
the requirements specified in UM10944.pdf.

The following configuration tables are currently unsupported:

* VL Lookup Table (TABLE := vl-lookup-table)
* VL Policing Table (TABLE := vl-policing-table)
* VL Forwarding Table (TABLE := vl-forwarding-table)
* VL Forwarding Parameters Table (TABLE := vl-forwarding-parameters-table)
* Clock Synchronization Parameters Table (TABLE := clock-synchronization-parameters-table)
* AVB Parameters Table (TABLE := avb-parameters-table)
* Retagging Table (TABLE := retagging-table)

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-conf(5),
sja1105-tool-config-format(5),
sja1105-tool(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


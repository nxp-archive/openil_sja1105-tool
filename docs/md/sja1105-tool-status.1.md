% sja1105-tool-status(1) | SJA1105-TOOL

NAME
====

sja1105-tool-status - Status command for NXP sja1105-tool

SYNOPSIS
========

**sja1105-tool** status _AREA_ \[_OPTIONS_\]

_AREA_ := { general | ports }

**sja1105-tool** status general

**sja1105-tool** status ports \[_`PORT_NUMBER`_\]

DESCRIPTION
===========

This command sends SPI messages to the SJA1105 and reads registers from the
SJA1105 Status Area (see chapter 5.1 in UM10944.pdf). Currently the General
Status Information (chapter 5.1.1) and Ethernet Port Status (chapter 5.1.4)
are supported.

Each field name and value is printed on a separate line.
Field name and field value are space-separated.

AREAS
=====

general

:   The following registers are read from the General Status Information area:

    * General status register 1 (address 03h)
    * General status register 2 (address 04h)
    * General status register 3 (address 05h)
    * General status register 4 (address 06h)
    * General status register 5 (address 07h)
    * General status register 6 (address 08h)
    * General status register 7 (address 09h)
    * General status register 8 (address 0Ah)
    * General status register 9 (address 0Bh)
    * General status register 10 (address 0Ch)
    * General status registers 11 to 20 (addresses C0h to C9h)

ports \[_`PORT_NUMBER`_\]

:   The following registers are read from the Ethernet Port Status Information area:

    * Ethernet port status - MAC-level diagnostic counters (relative address 0h)
    * Ethernet port status - diagnostic flags (relative address 1h)
    * Ethernet high-level port status diagnostic counters part 1
    * Ethernet high-level port status diagnostic counters part 2

    Differences from UM10944.pdf:

    * N_ERG_DISABLED is called N_EGR_DISABLED - this is a typo in the user manual
    * POLIECERR is called POLICEERR - this is a typo in the user manual
    * N_RXFRMSH and N_RXFRM have been concatenated into a single 64-bit field named N_RXFRM
    * N_RXBYTESH and N_RXBYTE have been concatenated into a single 64-bit field named N_RXBYTE
    * N_TXFRMSH and N_TXFRM have been concatenated into a single 64-bit field named N_TXFRM
    * N_TXBYTESH and N_TXBYTE have been concatenated into a single 64-bit field named N_TXBYTE

    The readout is done for the port specified as _`PORT_NUMBER`_. If
    _`PORT_NUMBER`_ is not specified, the command prints the status of all
    5 ports, each port on its own vertical column.


EXAMPLES
========

The following is a shell script that shows counters for sent, received
and dropped packets for switch port 2:

```bash
#!/bin/sh

export status=`sja1105-tool status port 2`
port2_sent=`echo "$status" | grep "N_TXFRM"  | awk '{print $2}'`
port2_recv=`echo "$status" | grep "N_RXFRM"  | awk '{print $2}'`
port2_drop=`echo "$status" | grep "N_POLERR" | awk '{print $2}'`
echo "Port 2 status: $port2_sent packets sent, $port2_recv packets received, $port2_drop packets dropped"
```

The following script checks if the SJA1105 configuration is valid. Note
that "**sja1105-tool config upload**" does not perform this check.

```bash
#!/bin/sh

if [[ $(sja1105-tool status general | grep "CONFIGS" | awk '{print $2}') == "1" ]]; then
	echo "Device configuration is valid"
else
	echo "Device configuration is invalid"
fi
```

BUGS
====

**sja1105-tool status ports** does not take into account the screen width and
max entry count specified in **/etc/sja1105/sja1105.conf**.

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-conf(5),
sja1105-tool-status(1),
sja1105-tool(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


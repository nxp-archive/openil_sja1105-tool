% sja1105-tool-reset(1) | SJA1105-TOOL

NAME
====

sja1105-tool-reset - Reset command for NXP sja1105-tool

SYNOPSIS
========

**sja1105-tool** reset { warm | cold }

DESCRIPTION
===========

This command sends an SPI write message to the Reset Generation Unit (RGU)
(Chapter 5.4 of UM10944.pdf) of the SJA1105. This triggers a warm or cold
reset of the software on the switch, which returns to an unconfigured
state. All port counters are reset to zero.

Note that the "**sja1105-tool config upload**" command issues a warm reset 
to the switch automatically.

BUGS
====

The difference in expected behavior between warm and cold reset is unclear,
since it is not mentioned in UM10944.pdf.

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-conf(5),
sja1105-tool-config(1),
sja1105-tool(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.



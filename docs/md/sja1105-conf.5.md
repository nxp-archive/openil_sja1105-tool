% sja1105-conf(5) | SJA1105-TOOL

NAME
====

_/etc/sja1105/sja1105.conf_ - Configuration File for NXP sja1105-tool

DESCRIPTION
===========

_/etc/sja1105/sja1105.conf_ is the configuration file for _sja1105-tool_. It is
read on every invocation of the tool and used to describe part of its behavior.

Syntactically it follows the INI file format, with section names marked between
square brackets, and key-value pairs written one per line, separated by the "="
(equal) sign and an arbitrary amount of whitespace (tabs or spaces). Lines may
be ignored completely if the first non-whitespace character is "#" (hash).
Comments at the end of the line are not allowed and will lead to incorrect
parsing of the file.

If the file does not exist or syntax errors are present, the **sja1105-tool**
throws an error and loads default values.

THE SPI SETUP SECTION
---------------------

This section begins when a line contains the string "[spi-setup]" and ends
at the beginning of a different section or at the end of file. Values should
conform to UM10944.pdf "Chapter 3. SPI interface". The following keys are
allowed in this section:

staging-area

:   This is the filename used to store the staging, binary configuration
    employed by the command family **sja1105-tool config**. If not
    specified, it defaults to "_/etc/sja1105/.staging_". It should be
    specified without quotes even if spaces are present in the filename.

device

:   This is the filename used to communicate with the SJA1105. It must
    be a character device that supports the following ioctl calls:
    SPI_IOC_RD_MODE, SPI_IOC_WR_MODE, SPI_IOC_WR_BITS_PER_WORD,
    SPI_IOC_RD_BITS_PER_WORD, SPI_IOC_RD_MAX_SPEED_HZ,
    SPI_IOC_WR_MAX_SPEED_HZ, SPI_IOC_MESSAGE. If not specified, it
    defaults to "_/dev/spidev0.1_". The same restrictions apply as
    above.

bits

:   Number of bits per word for the SPI communication. Should be set to 8.

delay

:   Number of microseconds for how long to delay after sending the last bit
    of this transfer before optionally deselecting the chip until the next one.
    Defaults to zero.

cs_change

:   Set to 1 to deactivate Chip Select for the SPI device before starting
    the next transfer, or 0 to keep Chip Select always active. Defaults
    to zero.

mode

:   Used to describe the Clock Polarity and Clock Phase for the SPI
    transmission. May be set to "0" or "SPI_CPHA" or "SPI_CPOL" or "SPI_CPHA |
    SPI_CPOL". Should be set (and defaults) to SPI_CPHA.

speed

:   Maximum SPI clock speed, in Hz. Should be set higher than 700 Hz and
    lower than 17.8 MHz. Defaults to 1 MHz.

dry_run

:   If set to "true" then instead of sending a SPI_IOC_MESSAGE ioctl to
    the SPI character device, commands such as "**sja1105-tool config upload**"
    just print the SPI messages as a hexdump to stdout. No communication
    is performed over SPI.

THE GENERAL SECTION
-------------------

This section begins when a line contains the string "[general-setup]" and ends
at the beginning of a different section or at the end of file. The following
keys are allowed in this section:

debug

:   If set to "true", debugging output is produced for bit-level access to the
    switch configuration tables. Recommended "false".

verbose

:   If set to "false", nothing is printed to stdout, only error messages are
    printed to stderr.

screen-width

:   Used for the following commands:

* **sja1105-tool config show**
* **sja1105-tool status ports**

    The idea is to make better use of screen space by fitting more entries on
    separate vertical columns. The total width of the vertical columns does not
    exceed the "screen-width" value. Set "screen-width" to a value greater or
    equal to 80, and less than the output of ```tput cols```.

entries-per-line

:   See "screen-width" above. Additionally, no more than "entries-per-line"
    vertical columns are allowed to be concatenated. The result is that each
    line will contain the minimum of "entries-per-line" and how many columns
    physically fit in "screen-width" characters.

EXAMPLE
=======

```conf
[spi-setup]
	staging-area = FILE
	device       = FILE
	bits         = 8
	speed        = 1000000
	delay        = 0
	cs_change    = 0
	mode         = SPI_CPHA
	dry_run      = false

[general]
	screen-width     = 200
	entries-per-line = 10
	verbose          = false

```

BUGS
====

**sja1105-tool status ports** currently ignores the values "entries-per-line"
and "screen-width".

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-tool(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


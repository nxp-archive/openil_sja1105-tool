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
    specified, it defaults to "_/lib/firmware/sja1105.bin_". This is the
    loacation and filename from where the sja1105 kernel module tries to load
    the configuration. It should be specified without quotes even if spaces
    are present in the filename.

device

:   This is the filename used to communicate with the sja1105 kernel module.
    It shall point to the sja1105 device in the sysfs.
    defaults to "_/sys/bus/spi/drivers/sja1105/spi0.1_". The same restrictions
    apply as above.

auto_flush

: - Sets the flush condition to true for some of the sja1105-tool commands
    that perform modifications to the staging area, and after which the
    configuration is still likely to be valid. Commands in this category
    are: "**sja1105-tool config modify**", "**sja1105-tool config
    load**", "**sja1105-tool config default**", but not "**sja1105-tool
    config new**". Also see sja1105-tool-config(1).

  - Even if "auto_flush" is set to false here, the flush condition can
    be forced to true by invoking the commands mentioned above with the
    -f|--flush argument.

  - If the flush condition is set to true, care must be taken that all
    intermediate configurations are valid, or else the respective
    sja1105-tool command will fail. For a list of the checks performed
    on the configuration by the sja1105-tool see
    sja1105-tool-config-format(5).

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
	auto_flush   = false

[general]
	screen-width     = 120
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
sja1105-tool-config(1)
sja1105-tool-config-format(5)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


% sja1105-kmod(1) | SJA1105-TOOL

NAME
====

_sja1105.ko_ - Kernel driver module for NXP SJA1105

DESCRIPTION
===========

_sja1105.ko_ is the kernel driver module (kmod) for the SJA1105 switch chip.
It loads the static switch configuration from user space and uploads it to
the chip, monitors the link status of the port's PHYs to reconfigure the
port's MAC speed and provides a user space interface to control the operation
of the switch.


STATIC CONFIGURATION
--------------------

On driver probe, the kmod loads the static configuration from user
space (default location "_/lib/firmware/sja1105.bin_") and uploads
it to the chip.

RESET PIN HANDLING
------------------

If defined in device tree, the kmod controls the reset lines of the switch
and PHY chips. On driver probe, it releses the switch from reset, uploads
the static configuration, then releases the PHYs from reset.

NETWORK DEVICE INTERFACES
-------------------------

The kmod creates a network interface for each port of the switch to configure
and monitor the PHY connected to this port. On a link up event, the link speed
of the port's MAC is adjusted to match with the PHY's speed.

SYSFS USER SPACE INTERFACE
--------------------------

The kmod exports some sysfs attributes to control and monitor the switch.

config_upload

:   Write '1' to the file to reload the static configuration from user space
    and upload it to the switch chip.

device_id

:   Read the device id of the switch chip. Reading the file returns the device
    id, part number and the chip's name.

general_status

:   Read general status information from the switch. Reading the file returns
    the status information in text format.

port_mapping

:   Get the mapping between the network interface name and port number
    for all ports. Reading the file returns a "_netif\_name_ _port\_number_"
    pair for each port.

port_status

:   Read status information for a specific port from the switch. To select
    a port, write either the network interface name which is assigned to the
    port or the port number to the file prior to reading. Consecutive reads
    always return the status of the same port.

port_status_clear

:   Clear the status counters for a specific port. To clear the counters of
    a port, write either the network interface name which is assigned to the
    port or the port number to the file. Writing "_all_" clears the counters
    of all ports.

reg_access

:   Read or write a register of the switch chip.
    **Register write:** Write the register address and value, separated by a
    space character to the file.
    **Register read:** Write the register address to the file prior to
    reading. When reading the file, the register value is read from the
    switch and returned in the format "_reg\_address_ _value_". Consecutive
    reads always return the same registers value.

DEVICE-TREE
===========

BINDINGS
--------

reset-gpio

:   GPIO connected to the reset input of the switch chip.

phy-reset-gpio

:   GPIO connected to the reset input of a PHY chip. Can be specified for each
    port.

phy-reset-duration

:   Length of the reset pule in milliseconds.

phy-reset-delay

:   Time before the PHY is ready after reset in milliseconds.


DEVICE-TREE EXAMPLE
-------------------

```
&dspi0 {
	sja1105@1 {
		compatible = "nxp,sja1105t";
		reg = <0x1>;
		spi-max-frequency = <4000000>;
		#address-cells = <1>;
		#size-cells = <0>;
		fsl,spi-cs-sck-delay = <1000>;
		fsl,spi-sck-cs-delay = <1000>;
		#reset-gpio = <&spigpio1_1 0 1>;
		port@0 {
			sja1105,port-label = "seth5"; /* ETH5 written on chassis */
			phy-handle = <&rgmii_phy3>;
			phy-mode = "rgmii";
			#phy-reset-gpio = <&spigpio1_1 1 1>;
			#phy-reset-duration = <1>;
			#phy-reset-delay = <120>;
			reg = <0>;
		};
		port@1 {
			sja1105,port-label = "seth2"; /* ETH2 written on chassis */
			phy-handle = <&rgmii_phy4>;
			phy-mode = "rgmii";
			reg = <1>;
		};
		port@2 {
			sja1105,port-label = "seth3"; /* ETH3 written on chassis */
			phy-handle = <&rgmii_phy5>;
			phy-mode = "rgmii";
			reg = <2>;
		};
		port@3 {
			sja1105,port-label = "seth4"; /* ETH4 written on chassis */
			phy-handle = <&rgmii_phy6>;
			phy-mode = "rgmii";
			reg = <3>;
		};
		port@4 {
			sja1105,port-label = "sethi"; /* Internal port connected to eth2 */
			phy-mode = "rgmii";
			reg = <4>;
			fixed-link {
				speed = <1000>;
				full-duplex;
			};
		};
	};
};
```

BUGS
====

AUTHOR
======

sja1105-tool was written by Vladimir Oltean <vladimir.oltean@nxp.com>
Kernel module was written by Georg Waibel <georg.waibel@sensor-technik.de>

SEE ALSO
========

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same authors.


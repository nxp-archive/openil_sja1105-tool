TODO list
=========

Currently this driver is a prototype that only registers netdevices for
each SJA1105 port and links them with the phylib (which is the major
reason for its existence). The driver does not yet communicate with the
`sja1105-tool`, but is intended to do so.

The short-term set of goals would be:
* to be able to compile the `src/lib/*.c` code into `sja1105.ko` as well
  as `libsja1105.so`
* to move
  [spi-transfer.c](https://github.com/openil/sja1105-tool/blob/master/src/lib/spi/spi-transfer.c)
  into the kernel module, and rewrite it so as to make use of the
  in-kernel SPI API instead of spidev ioctl
* parts of the `src/tool/*.c` will probably have to be moved into the
  kernel as well, such as [static_config_upload and
  static_config_flush](https://github.com/openil/sja1105-tool/blob/master/src/tool/staging-area.c#L238-L366).
  Then their implementation needs to be replaced with a
  userspace-to-kernel communication method (such as ioctl or netlink)
  which signals `sja1105.ko` that it should trigger a `request_firmware`
  call for the updated staging area.
* The request for firmware needs to be written for the kernel module.
  The sanity checking can be reused from the lib code.

At this point, `sja1105-tool config upload` would be able to place a
reconfigured staging area at `/lib/firmware/sja1105.bin` and trigger
the kernel to reload it. The kernel module will also request the staging
area file during probe time.

Once the kernel module can be able to program the staging area, the MAC
reconfiguration tables code should be used in the `sja1105_adjust_link`
callback, to complete the integration with phylib.

The broader set of goals includes:
* Registering the switch's 1588 hardware clock with the kernel's PTP
  infrastructure and being able to control the clock with the `phc_ctl`
  program.
* Implementing the Spanning Tree Protocol specification to disable
  switch ports such that L2 loops are prevented.

Using the SJA1105 kernel module
===============================

In your board device tree (e.g. `arch/arm/boot/dts/ls1021a-tsn.dts`),
replace the previous bindings for the userspace-accessible spidev
driver:

```
&dspi0 {
	spidev@1 {
		compatible = "rohm,dh2228fv";
		reg = <0x1>;
		spi-max-frequency = <4000000>;
		#address-cells = <1>;
		#size-cells = <1>;
	};
};
```

With something like this:

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
		port@0 {
			sja1105,port-label = "seth5"; /* ETH5 written on chassis */
			phy-handle = <&rgmii_phy3>;
			phy-mode = "rgmii";
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


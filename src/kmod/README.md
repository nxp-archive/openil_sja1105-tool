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


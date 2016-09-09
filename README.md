# SJA1105 Tool

## Supported Platforms

* LS1021ATSN

## Features

The sja1105-tool is a Linux userspace application for configuring the NXP SJA1105 Automotive Ethernet L2 switch. The tool supports:
* Importing a configuration for the SJA1105 switch from an XML file
* Exporting the current SJA1105 configuration as an XML file
* Uploading the current SJA1105 configuration to the switch through its SPI interface
* Inspecting the current SJA1105 configuration
* On-the-fly modification of the current SJA1105 configuration through command line or scripting interface

### Modifications

Each switch configuration is described in `/etc/sja1105/`, under the names `standard-config.xml`, `policing-config.xml` and `scheduling-config.xml`.
Each XML file describes the configuration tables involved, and their corresponding entries. The configuration should be edited according to the UM10944.pdf User Guide.
The `sja1105-tool` can manage not having the XML configuration files in place. To re-generate these, one can run the following commands:

## Compilation

### Standalone

Prerequisites:

* A toolchain for the target platform. For the LS1021, you can find an ARM toolchain [here](http://sun.ap.freescale.net/images).

```bash
cd <base_dir>
mkdir toolchain
cd toolchain
# Keep a note of this folder, because we'll be referencing it later
export TOOLCHAIN_PATH=$PWD
wget http://sun.ap.freescale.net/images/fsl-sdk-v2.0/fsl-toolchain/fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
chmod +x fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
./fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
# When prompted to "Enter target directory for SDK (default: /opt/fsl-qoriq/2.0):", please type exactly "$PWD"
```

* Libxml2. Unfortunately, this is not included in the standalone ARM toolchain we're using, so we have to add it manually.

```bash
# Grab the latest libxml tarball (tested with 2.9.3)
wget http://xmlsoft.org/sources/libxml2-sources-2.9.3.tar.gz
# Extract it in the current directory
tar xvf libxml2-sources-2.9.3.tar.gz
cd libxml2-2.9.3
# The standalone toolchain uses two sysroots: one for x86 (native) and one for ARM (target)
# We have to compile and install libxml2 twice, for each sysroot
# The manual part here is for you to figure out where the toolchain sysroots are located.
# If you are using the suggested toolchain, they are like this:
export TOOLCHAIN_ARM_SYSROOT=$TOOLCHAIN_PATH/sysroots/cortexa7hf-vfp-neon-fsl-linux-gnueabi
export TOOLCHAIN_X86_SYSROOT=$TOOLCHAIN_PATH/sysroots/x86_64-fsl-linux
# First for x86
./configure --prefix=$TOOLCHAIN_X86_SYSROOT --disable-shared
make -j 4
make install
make clean
# Then for ARM
source $TOOLCHAIN_PATH/environment-setup-cortexa7hf-vfp-neon-fsl-linux-gnueabi
./configure --prefix=$TOOLCHAIN_ARM_SYSROOT --disable-shared --host=arm-fsl-linux
make -j 4
make install
```

* Source code for the sja1105-tool:

```bash
cd <base_dir>
git clone http://sw-stash.freescale.net/scm/~b56933/sja1105-tool.git
cd sja1105-tool
# At this moment, you should make sure that the toolchain path is defined correctly
# inside the "envsetup" file. Here it is set relative to the "base_dir".
# You should still have the TOOLCHAIN_PATH variable set since you extracted the toolchain.
sed -i -e "s/export TOOLCHAIN_PATH=\.\.\/toolchain/export TOOLCHAIN_PATH=$TOOLCHAIN_PATH/g" envsetup
```

Compilation:

```bash
source envsetup
make
```

### Bitbake recipe

To include the sja1105-tool inside a Bitbake layer, please see the
Readme [here](http://sw-stash.freescale.net/users/b56933/repos/sja1105-tool-bitbake/browse).

## Usage

### Switch status

The output of the following commands should be interpreted according to UM10944.pdf, Chapter 5.1 Status Area. Any differences from said document are noted where present.

#### General status

Synopsis:
```bash
$ sja1105-tool status general
```

Description:

This command sends SPI messages to the SJA1105 and reads the following registers:
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

Each field name and value is printed on a separate line.
Field name and field value are space-separated.

Example:

```bash
if [[ $(sja1105-tool status general | grep "CONFIGS" | awk '{print $2}') == "1" ]]; then
	echo "Device configuration is valid"
else
	echo "Device configuration is invalid"
fi
```

#### Ethernet port status

Synopsis:
```bash
$ sja1105-tool status port [<port-number>]
```

Description:

This command sends SPI messages to the SJA1105 and reads the following registers:
  * Ethernet port status - MAC-level diagnostic counters (relative address 0h)
  * Ethernet port status - diagnostic flags (relative address 1h)
  * Ethernet high-level port status diagnostic counters part 1
  * Ethernet high-level port status diagnostic counters part 2

Each field name and value is printed on a separate line.
Field name and field value are space-separated.

The readout is done for the port specified as `<port-number>`. If no parameter is supplied for the port number, the command prints the status of all 5 ports, each port on its own vertical column.

Differences from UM10944.pdf:
  * `N_ERG_DISABLED` is called `N_EGR_DISABLED` - this is a typo in the user manual
  * `POLIECERR` is called `POLICEERR` - this is a typo in the user manual
  * `N_RXFRMSH` and `N_RXFRM` have been concatenated into a single 64-bit field named `N_RXFRM`
  * `N_RXBYTESH` and `N_RXBYTE` have been concatenated into a single 64-bit field named `N_RXBYTE`
  * `N_TXFRMSH` and `N_TXFRM` have been concatenated into a single 64-bit field named `N_TXFRM`
  * `N_TXBYTESH` and `N_TXBYTE` have been concatenated into a single 64-bit field named `N_TXBYTE`

Example:

```bash
$ export status=`sja1105-tool status port 2`
$ port2_sent=`echo "$status" | grep "N_TXFRM"  | awk '{print $2}'`
$ port2_recv=`echo "$status" | grep "N_RXFRM"  | awk '{print $2}'`
$ port2_drop=`echo "$status" | grep "N_POLERR" | awk '{print $2}'`
$ echo "Port 2 status: $port2_sent packets sent, $port2_recv packets received, $port2_drop packets dropped"
```

Bugs:
  * `sja1105-tool status port` does not take into account the screen width specified in sja1105.conf.

### Features currently unsupported

* The following configuration tables:
    * VL Lookup Table
    * VL Policing Table
    * VL Forwarding Table
    * VL Forwarding Parameters Table
    * Clock Synchronization Parameters Table
    * AVB Parameters Table
    * Retagging Table
* The following status areas:
    * Clock synchronization
    * Memory partitioning
    * Virtual link
* The following control areas:
    * General control
    * Dynamic reconfiguration
    * Auxilliary Configuration Unit (ACU)

### Features currently supported

Features that are not included in the "Unsupported" section, but are not employed by this demo, were NOT tested!
The only features that were tested are the ones included in the TSN demo:
* Configuration tables:
    * Schedule Table
    * Schedule Entry Points Table
    * L2 Policing Table
    * VLAN Lookup Table
    * L2 Forwarding Table
    * MAC Configuration Table
    * Schedule Parameters Table
    * Schedule Entry Points Parameters Table
    * L2 Address Lookup Parameters Table
    * L2 Forwarding Parameters Table
    * General Parameters Table
    * xMII Mode Parameters Table
* Status areas:
    * General Status
    * Port Status
* Control areas:
    * Reset Generation Unit (RGU)
    * Clock Generation Unit (CGU) - this is handled automatically (internally) by the tool

Note: Since the actual configuration area of the SJA1105 switch is write-only, there is no way for the `sja1105-tool` to actually retrieve that from the switch, over SPI. Instead, all operations (`config show`, `config modify`, etc) work with a local "staging area" where a binary form of the switch configuration is held. The configuration held in this staging area is only uploaded to the switch via `sja1105-tool config upload`.
Note: The `sja1105-tool` supports non-ambiguous shorthand names for every word, much in the style of `iproute2`. So if `ip address show` can be abbreviated as `ip a s`, same can be `sja1105-tool config upload` written as `sja1105-tool co u`.

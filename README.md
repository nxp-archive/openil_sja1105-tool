SJA1105 Tool
============

Supported Platforms
-------------------

* SJA1105E (Gen. 1, No TT-Ethernet)
* SJA1105T (Gen. 1, TT-Ethernet)
* SJA1105P (Gen. 2, No SGMII, No TT-Ethernet)
* SJA1105Q (Gen. 2, No SGMII, TT-Ethernet)
* SJA1105R (Gen. 2, SGMII, No TT-Ethernet)
* SJA1105S (Gen. 2, SGMII, TT-Ethernet)

Features
--------

The sja1105-tool is a Linux userspace application for configuring the NXP
SJA1105 family of Automotive Ethernet L2 switches. The tool supports:
* Importing a configuration for the SJA1105 switch from an XML file
* Exporting the current SJA1105 configuration as an XML file
* Uploading the current SJA1105 configuration to the switch through its
  SPI interface
* Inspecting the current SJA1105 configuration
* On-the-fly modification of the current SJA1105 configuration through command
  line or scripting interface

Standalone build
----------------

### Prerequisites

To compile this demo you need to have setup a Linux machine with the
following:

* An ARMv7 toolchain for the LS1021. You can get one from [Linaro](
https://releases.linaro.org/components/toolchain/binaries/latest/arm-linux-gnueabihf/)
(version used here is 6.3.1).

```bash
cd <base_dir>
mkdir linaro-armv7-toolchain
cd linaro-armv7-toolchain
wget https://releases.linaro.org/components/toolchain/binaries/latest/arm-linux-gnueabihf/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
wget https://releases.linaro.org/components/toolchain/binaries/latest/arm-linux-gnueabihf/sysroot-glibc-linaro-2.23-2017.05-arm-linux-gnueabihf.tar.xz
tar xvf gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
tar xvf sysroot-glibc-linaro-2.23-2017.05-arm-linux-gnueabihf.tar.xz
# rm -rf gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf.tar.xz
# rm -rf sysroot-glibc-linaro-2.23-2017.05-arm-linux-gnueabihf.tar.xz
# Copy and paste this command down until (and including) the EOF
cat << 'EOF' > envsetup
#!/bin/bash

export TOPDIR=$(cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd)
export SYSROOT_PATH="${TOPDIR}/sysroot-glibc-linaro-2.23-2017.05-arm-linux-gnueabihf"
export PKG_CONFIG_PATH="${SYSROOT_PATH}/usr/lib/pkgconfig"
export PATH="${TOPDIR}/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin:$PATH"
export CC="arm-linux-gnueabihf-gcc  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a7 --sysroot=${SYSROOT_PATH}"
export CXX="arm-linux-gnueabihf-g++  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a7 --sysroot=${SYSROOT_PATH}"
export CPP="arm-linux-gnueabihf-gcc -E  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a7 --sysroot=${SYSROOT_PATH}"
export AS="arm-linux-gnueabihf-as "
export LD="arm-linux-gnueabihf-ld  --sysroot=${SYSROOT_PATH}"
export GDB="arm-linux-gnueabihf-gdb"
export STRIP="arm-linux-gnueabihf-strip"
export RANLIB="arm-linux-gnueabihf-ranlib"
export OBJCOPY="arm-linux-gnueabihf-objcopy"
export OBJDUMP="arm-linux-gnueabihf-objdump"
export AR="arm-linux-gnueabihf-ar"
export NM="arm-linux-gnueabihf-nm"
export M4="m4"
export TARGET_PREFIX="arm-linux-gnueabihf-"
export CONFIGURE_FLAGS="--target=arm-linux-gnueabihf --host=arm-linux-gnueabihf --build=x86_64-linux --with-libtool-sysroot=${SYSROOT_PATH}"
export CFLAGS="-O2 -pipe -g -feliminate-unused-debug-types -I${SYSROOT_PATH}/include"
export CXXFLAGS="-O2 -pipe -g -feliminate-unused-debug-types -I${SYSROOT_PATH}/include"
export LDFLAGS="-Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed -L${SYSROOT_PATH}/lib"
export ARCH="arm"
export CROSS_COMPILE="arm-linux-gnueabihf-"

EOF

# Keep a note of this folder, because we'll be referring it later in the tutorial
export TOOLCHAIN_PATH="${PWD}"
```

* Libxml2. Unfortunately, this is not included in the standalone ARM toolchain
  we're using, so we have to add it manually:

```bash
cd <base_dir>
# Grab the latest libxml tarball (I used 2.9.3)
wget http://xmlsoft.org/sources/libxml2-sources-2.9.3.tar.gz
# Extract it in the current directory
tar xvf libxml2-sources-2.9.3.tar.gz
cd libxml2-2.9.3
# Remember the $TOOLCHAIN_PATH we set just above
source "${TOOLCHAIN_PATH}/envsetup"
./configure \
	--prefix="${SYSROOT_PATH}" \
	--host=arm-linux-gnueabihf \
	--includedir="${SYSROOT_PATH}/usr/include" \
	--libdir="${SYSROOT_PATH}/usr/lib" \
	--without-python \
	--without-lzma
make -j 128
make install
# When compiling sja1105-tool, it will use pkg-config to find the proper
# location of libxml2, which is the --libdir parameter from above.
```


### Compilation:

```bash
cd <base_dir>
git clone git@github.com:openil/sja1105-tool.git
cd sja1105-tool
# Again, remember your $TOOLCHAIN_PATH
source "${TOOLCHAIN_PATH}/envsetup"
make -j 128
# To build the manpages, run "make man" or "make all"
# However, this step requires the "pandoc" package to be installed.
DESTDIR=out make install
```

Documentation
-------------

The following manual pages are provided:

```bash
cd docs/man
man -l ./sja1105-tool.1                # General command overview
man -l ./sja1105-tool-config.1         # Detailed usage of sja1105-tool config
man -l ./sja1105-tool-status.1         # Detailed usage of sja1105-tool status
man -l ./sja1105-tool-reset.1          # Detailed usage of sja1105-tool reset
man -l ./sja1105-conf.5                # File format for sja1105-tool configuration
man -l ./sja1105-tool-config-format.5  # File format for XML switch configuration tables
```

Known issues
------------

1. Clock synchronization via TTEthernet (SAE AS6802) is not supported.


Reporting issues
----------------

When creating a new issue in the Github tracker, it may be useful to
describe the usage scenario, as well as provide an XML configuration and
an SPI dump of the configuration you are sending to the switch.

To get an XML configuration:

```bash
sja1105-tool config save bug.xml
```


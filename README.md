SJA1105 Tool
============

Supported Platforms
-------------------

* LS1021ATSN

Features
--------

The sja1105-tool is a Linux userspace application for configuring the NXP
SJA1105 Automotive Ethernet L2 switch. The tool supports:
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

* An ARM toolchain for the LS1021. You can get one from
  [here](http://sun.ap.freescale.net/images).

```bash
cd <base_dir>
mkdir toolchain
cd toolchain
# Keep a note of this folder, because we'll be referring it later in
the tutorial
export TOOLCHAIN_PATH=$PWD
wget http://sun.ap.freescale.net/images/fsl-sdk-v2.0/fsl-toolchain/fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
chmod +x fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
./fsl-qoriq-glibc-x86_64-cortexa7hf-vfp-neon-toolchain-2.0.sh
# When prompted to "Enter target directory for SDK (default: /opt/fsl-qoriq/2.0):",
# please type exactly "$PWD"
```

* Libxml2. Unfortunately, this is not included in the standalone ARM toolchain
  we're using, so we have to add it manually:

```bash
# Grab the latest libxml tarball (I used 2.9.3)
wget http://xmlsoft.org/sources/libxml2-sources-2.9.3.tar.gz
# Extract it in the current directory
tar xvf libxml2-sources-2.9.3.tar.gz
cd libxml2-2.9.3
# The standalone toolchain uses two sysroots: one for x86 (native) and one for
# ARM (target). We have to cross-compile and install libxml2 for ARM.
# This location might differ according to your toolchain:
export TOOLCHAIN_ARM_SYSROOT=$TOOLCHAIN_PATH/sysroots/cortexa7hf-vfp-neon-fsl-linux-gnueabi
source $TOOLCHAIN_PATH/environment-setup-cortexa7hf-vfp-neon-fsl-linux-gnueabi
./configure --prefix=$TOOLCHAIN_ARM_SYSROOT --host=arm-fsl-linux --libdir=/usr/lib --includedir=/include --without-python
make -j 4
make install
# When compiling sja1105-tool, it will use pkg-config to find the proper
# location of libxml2, which is the --libdir parameter from above.
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

### Compilation:

```bash
source envsetup
make
# To build the manpages, run "make man" or "make all"
# However, this step requires the "pandoc" package to be installed.
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

To also get PDF versions:

```bash
cd docs/md
sudo apt-get install pandoc texlive
pandoc --standalone -t latex sja1105-tool-reset.1.md -o ../pdf/sja1105-tool-reset.1.pdf
pandoc --standalone -t latex sja1105-tool-status.1.md -o ../pdf/sja1105-tool-status.1.pdf
pandoc --standalone -t latex sja1105-tool-config-format.5.md -o ../pdf/sja1105-tool-config-format.5.pdf
pandoc --standalone -t latex sja1105-tool-config.1.md -o ../pdf/sja1105-tool-config.1.pdf
pandoc --standalone -t latex sja1105-tool.1.md -o ../pdf/sja1105-tool.1.pdf
pandoc --standalone -t latex sja1105-conf.5.md -o ../pdf/sja1105-conf.5.pdf
```

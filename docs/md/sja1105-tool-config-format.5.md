% sja1105-tool-config-format(5) | SJA1105-TOOL

NAME
====

sja1105-tool-config-format - XML Configuration Table Format for NXP SJA1105

DESCRIPTION
===========

Configuration tables can be imported and exported from the staging area of the
_sja1105-tool_ (which is a binary file) into human-readable XML descriptions.
The syntax of the XML description is closely related to chapter "4.2. Switch
Configuration Tables" of UM10944.pdf.

This manual is not intended as a replacement for UM10944.pdf. The
signification of the fields (children) of each entry is not discussed here,
only the relationship between the XML description and tables found in the PDF.
For detailed information about the table entries and fields, please consult
UM10944.pdf.

The data language used to describe the SJA1105 configuration tables in XML
format is NETCONF-compliant. It follows the structure of the YANG model
sja1105.yang from package yang-sja1105. For brevity, this has not been
included here. However, several points are noted:

* There is exactly one *sja1105* root element (container)
* Zero or more children elements of *sja1105* (also containers). Each of
  these elements describes a configuration table. Possible names are described
  in [CONFIGURATION TABLE NAMES]().
* Each table element has zero or more children elements named *entry* that
  represent table entries. Elements of this type have children that are
  highly specific to the name of their *table* parent element.

To view an example of XML configuration that is always guaranteed to be
correct:

```bash
sja1105-tool config default ls1021atsn
sja1105-tool config save default.xml
cat default.xml
```

The numeric values for each entry field (attribute) are stored in the XML as
strings and interpreted by _sja1105-tool_ as unsigned 64-bit numbers.
They must be specified inside quotes, and can be expressed in either
base 10 (e.g. "15"), base 16 (e.g. "0xf"), base 2 (e.g. "0b1111") or
base 8 (e.g. "017"), whichever is more convenient for representation.
If a value larger than the bit width *B* of the specific field is used,
the field gets truncated to the least significant *B* bits
of the value. If a numeric value contains the ':' character, it is
assumed to be a MAC address and is interpreted as such
("aa:bb:cc:dd:ee:ff").

Fields which are defined as arrays (e.g. "vlan_pmap") should be expressed as a
list of space-separated numbers, encased in square brackets. The restrictions
above still apply, e.g. "[0x1 2 0b11 0x3 04]" can be used to describe the
array {1, 2, 3, 4}.

The sja1105-tool performs a series of basic validity checks on the
configuration described here. Although these are spelled out in Table 2.
Configuration Tables of UM10944.pdf, these checks are listed here as
well:

* if the number of entries in the schedule-table is larger than zero,
  then:
    * number of entries in schedule-entry-points-table must also be
      larger than zero
    * number of entries in schedule-parameters-table must also be
      larger than zero
    * number of entries in schedule-entry-points-parameters-table must
      be 1
* if the number of entries in the vl-lookup-table is larger than zero,
  then:
    * number of entries in vl-policing-table must also be
      larger than zero
    * number of entries in vl-forwarding-table must also be
      larger than zero
    * number of entries in vl-forwarding-parameters-table must be 1
* number of entries in l2-policing-table must be larger than zero
* number of entries in vlan-lookup-table must be larger than zero
* number of entries in l2-forwarding-table must be 13
* number of entries in l2-forwarding-parameters-table must be 1
* number of entries in general-parameters-table must be 1
* sum of all l2-forwarding-parameters-table.part_spc array elements plus
  sum of all vl-forwarding-parameters-table.partspc array elements
  should not be larger than 929 or 910, if retagging is used.

Note that the first two conditions have implications on the order in
which configuration tables must be created, if the flush condition is
set to true.

CONFIGURATION TABLE NAMES
=========================

Possible table names:

* "schedule-table"
* "schedule-entry-points-table"
* "vl-lookup-table"
* "vl-policing-table"
* "vl-forwarding-table" 
* "l2-address-lookup-table"
* "l2-policing-table"
* "vlan-lookup-table"
* "l2-forwarding-table"
* "mac-configuration-table"
* "schedule-parameters-table"
* "schedule-entry-points-parameters-table" 
* "vl-forwarding-parameters-table"
* "l2-address-lookup-parameters-table"
* "l2-forwarding-parameters-table"
* "clock-synchronization-parameters-table"
* "avb-parameters-table"
* "general-parameters-table"
* "retagging-table"

SCHEDULE TABLE
--------------

Table name "schedule-table" refers to "Chapter 4.2.1 Schedule Table" in
UM10944.pdf. It contains 0-1024 entries with the following children:

* "winstindex", bits 63-54
* "winend", bits 53-53
* "winst", bits 52-52
* "destports", bits 51-47
* "setvalid", bits 46-46
* "txen", bits 45-45
* "resmedia_en", bits 44-44
* "resmedia", bits 43-36
* "vlindex", bits 35-26
* "delta", bits 25-8

Differences from UM10944.pdf:

* In UM10944.pdf, **resmedia** is a 9-bit field, where the 8
  least significant bits are one-hot, and the most significant
  bit is an "enable" bit. In _sja1105-tool_, this field has been
  broken into **resmedia** (the 8 least significant bits: 43-36)
  and **resmedia_en** (the most significant bit: 44-44).

SCHEDULE ENTRY POINTS TABLE
---------------------------

Table name "schedule-entry-points-table" refers to "Chapter 4.2.2 Schedule
Entry Points Table" in UM10944.pdf. It contains 0-2048 entries with the
following children:

* "subschindx", bits 31-29
* "delta", bits 28-11
* "address", bits 10-1

VL LOOKUP TABLE
---------------

Table name "vl-lookup-table" refers to "Chapter 4.2.3 VL Lookup Table"
in UM10944.pdf. It contains 0-1024 entries.

If general-parameters-table.vllupformat == 0, it has the following
children:

* "destports", bits 95-91
* "iscritical", bits 90-90
* "macaddr", bits 89-42
* "vlanid", bits 41-30
* "port", bits 29-27
* "vlanprior", bits 26-24

If general-parameters-table.vllupformat == 1, it has the following
children:

* "egrmirr", bits 95-91
* "ingrmirr", bits 90-90
* "vlid", bits 57-42
* "port", bits 29-27

Differences from UM10944.pdf:

* In UM10944.pdf, **vlid** is called **vlld**.

VL POLICING TABLE
-----------------

Table name "vl-policing-table" refers to "Chapter 4.2.4 VL Policing
Table" in UM10944.pdf. It contains 0-1024 entries. Entry i in the
vl-policing-table is of the type (rate-constrained or time-triggered)
specified in vl-forwarding-table[i].type.

If vl-forwarding-table[i].type == 1, it has the following children:

* "type", bits 63-63, must be set to 1
* "maxlen", bits 62-52
* "sharindx", bits 51-42

If vl-forwarding-table[i].type == 0, it has the following children:

* "type", bits 63-63, must be set to 1
* "maxlen", bits 62-52
* "sharindx", bits 51-42
* "bag", bits 41-28
* "jitter", bits 27-18

VL FORWARDING TABLE
-------------------

Table name "vl-forwarding-table" refers to "Chapter 4.2.5 VL Forwarding
Table" in UM10944.pdf. It contains 0-1024 entries with the
following children:

* "type", bits 31-31
* "priority", bits 30-28
* "partition", bits 27-25
* "destports", bits 24-20

L2 ADDRESS LOOKUP TABLE
-----------------------

Table name "l2-address-lookup-table" refers to "Chapter 4.2.6 L2
Address Lookup Table" in UM10944.pdf. It contains 0-1024 entries with the
following children:

* "vlanid", bits 95-84
* "macaddr", bits 83-36
* "destports", bits 35-31
* "enfport", bits 30-30
* "index", bits 29-20

L2 POLICING TABLE
-----------------

Table name "l2-policing-table" refers to "Chapter 4.2.7 L2 Policing
Table" in UM10944.pdf. It contains 40 or 45 entries with the following
children:

* "sharindx", bits 63-58
* "smax", bits 57-42
* "rate", bits 41-26
* "maxlen", bits 25-15
* "partition", bits 14-12

VLAN LOOKUP TABLE
-----------------

Table name "vlan-lookup-table" refers to "Chapter 4.2.8 VLAN Lookup
Table" in UM10944.pdf. It contains 0-4096 entries with the following
children:

* "ving_mirr", bits 63-59
* "vegr_mirr", bits 58-54
* "vmemb_port", bits 53-49
* "vlan_bc", bits 48-44
* "tag_port", bits 43-39
* "vlanid", bits 38-27

L2 FORWARDING TABLE
-------------------

Table name "l2-forwarding-table" refers to "Chapter 4.2.9 L2 Forwarding
Table" in UM10944.pdf. It contains 13 entries with the following
children:

* "bc_domain", bits 63-59
* "reach_port", bits 58-54
* "fl_domain", bits 53-49
* "vlan_pmap", bits 48-25

MAC CONFIGURATION TABLE
-----------------------

Table name "mac-configuration-table" refers to "Chapter 4.2.10 MAC
Configuration Table" in UM10944.pdf. It contains 5 entries with the
following children:

* "top", bits 223-215, ..., 90-82
* "base", bits 214-206, ..., 81-73
* "enabled", bits 205-205, ..., 72-72
* "ifg", bits 71-67
* "tp_delin", bits 64-49
* "tp_delout", bits 48-33
* "maxage", bits 32-25
* "vlanprio", bits 24-22
* "vlanid", bits 21-10
* "ing_mirr", bits 9-9
* "egr_mirr", bits 8-8
* "drpnona664", bits 7-7
* "drpdtag", bits 6-6
* "drpuntag", bits 5-5
* "retag", bits 4-4
* "dyn_learn", bits 3-3
* "egress", bits 2-2
* "ingress", bits 1-1

Note that the "speed" field is hidden and not configurable from the
tool, since the kernel module determines it automatically from the DTS
bindings.

SCHEDULE PARAMETERS TABLE
-------------------------

Table name "schedule-parameters-table" refers to "Chapter 4.2.11 Schedule
Parameters" in UM10944.pdf. It contains 1 entry with the following
children:

* "subscheind", bits 95-16

SCHEDULE ENTRY POINTS PARAMETERS TABLE
--------------------------------------

Table name "schedule-entry-points-parameters-table" refers to "Chapter 4.2.12
Schedule Entry Points Parameters" in UM10944.pdf. It contains 1 entry with the
following children:

* "clksrc", bits 31-30
* "actsubsch", bits 29-27

VL FORWARDING PARAMETERS TABLE
------------------------------

Table name "vl-forwarding-parameters-table" refers to "Chapter 4.2.13 VL
Forwarding Parameters Table" in UM10944.pdf. It contains 0-1024 entries with the
following children:

* "partspc", bits 25-16, 35-26, 45-36, 55-46, 65-56, 75-66, 85-76, 95-86
* "debugen", bits 24-20

L2 LOOKUP PARAMETERS TABLE
--------------------------

Table name "l2-lookup-parameters-table" refers to "Chapter 4.2.14 L2 Lookup
Parameters" in UM10944.pdf. It contains 1 entry with the following
children:

* "maxage", bits 31-17
* "dyn_tbsz", bits 16-14
* "poly", bits 13-6
* "shared_learn", bits 5-5
* "no_enf_hostprt", bits 4-4
* "no_mgmt_learn", bits 3-3

L2 FORWARDING PARAMETERS TABLE
------------------------------

Table name "l2-forwarding-parameters-table" refers to "Chapter 4.2.15 L2
Forwarding Parameters" in UM10944.pdf. It contains 1 entry with the following
children:

* "max_dynp", bits 95-93
* "part_spc", bits 92-13

AVB PARAMETERS TABLE
--------------------

Table name "avb-parameters-table" refers to "Chapter 4.2.17 AVB
Parameters" in UM10944.pdf. It contains 1 entry with the following
children:

* "destmeta", bits 95-48
* "srcmeta", bits 47-0

GENERAL PARAMETERS TABLE
------------------------

Table name "general-parameters-table" refers to "Chapter 4.2.18 General
Parameters" in UM10944.pdf. It contains 1 entry with the following
children:

* "vllupformat", bits 319-319
* "mirr_ptacu", bits 318-318
* "switchid", bits 317-315
* "hostprio", bits 314-312
* "mac_fltres1", bits 311-264
* "mac_fltres0", bits 263-216
* "mac_flt1", bits 215-168
* "mac_flt0", bits 167-120
* "incl_srcpt1", bits 119-119
* "incl_srcpt0", bits 118-118
* "send_meta1", bits 117-117
* "send_meta0", bits 116-116
* "casc_port", bits 115-113
* "host_port", bits 112-110
* "mirr_port", bits 109-107
* "vlmarker", bits 106-75
* "vlmask", bits 74-43
* "tpid", bits 42-27
* "ignore2stf", bits 26-26
* "tpid2", bits 25-10

Differences from UM10944.pdf:

* In UM10944.pdf, **vlmarker** is called **vimarker** and **vlmask** is
  called **vimask**.

XMII MODE PARAMETERS TABLE
--------------------------

This table is deliberately hidden from the user and not configurable
from the tool, since the kernel module determines MII parameters
automatically from the DTS bindings.

EXAMPLES
========

To get a minimal SJA1105 configuration for the LS1021ATSN board with only the
mandatory configuration tables loaded:

```bash
#!/bin/bash

# Create the staging area
sja1105-tool conf default ls1021atsn
# Export the staging area to XML
sja1105-tool conf save default.xml
```

Here is a piece (the L2 Forwarding Table) extracted from the LS1021ATSN
default configuration:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<config>
  <table name="l2-forwarding-table">
    <entry index="0" bc_domain="0x1E" reach_port="0x1E" fl_domain="0x1E"
           vlan_pmap="[0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 ]"/>
    <entry index="1" bc_domain="0x1D" reach_port="0x1D" fl_domain="0x1D"
           vlan_pmap="[0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 ]"/>
    <entry index="2" bc_domain="0x1B" reach_port="0x1B" fl_domain="0x1B"
           vlan_pmap="[0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 ]"/>
    <entry index="3" bc_domain="0x17" reach_port="0x17" fl_domain="0x17"
           vlan_pmap="[0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 ]"/>
    <entry index="4" bc_domain="0xF"  reach_port="0xF"  fl_domain="0xF"
           vlan_pmap="[0x0 0x1 0x2 0x3 0x4 0x5 0x6 0x7 ]"/>
    <entry index="5" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 ]"/>
    <entry index="6" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x1 0x1 0x1 0x1 0x1 0x0 0x0 0x0 ]"/>
    <entry index="7" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x2 0x2 0x2 0x2 0x2 0x0 0x0 0x0 ]"/>
    <entry index="8" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x3 0x3 0x3 0x3 0x3 0x0 0x0 0x0 ]"/>
    <entry index="9" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x4 0x4 0x4 0x4 0x4 0x0 0x0 0x0 ]"/>
    <entry index="10" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x5 0x5 0x5 0x5 0x5 0x0 0x0 0x0 ]"/>
    <entry index="11" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x6 0x6 0x6 0x6 0x6 0x0 0x0 0x0 ]"/>
    <entry index="12" bc_domain="0x0"  reach_port="0x0"  fl_domain="0x0"
           vlan_pmap="[0x7 0x7 0x7 0x7 0x7 0x0 0x0 0x0 ]"/>
  </table>
</config>
```

The following facts are noted:

* The "index" attribute of each entry is not required nor is it
  interpreted by _sja1105-tool_; it is simply for ease of reading.
* The first 5 entries in the L2 Forwarding Table are per-port.
    * *bc_domain* indicates the Broadcast Domain. The only limitation
      imposed by default is that broadcast frames received on an
      interface should not be forwarded on the port it came from.
      Thus, the broadcast domain of Port 0 is 0b11110 (0x1E) (Ports 1 to 4),
      broadcast domain of Port 1 is 0b11101, etc.
    * *reach_port* indicates the Port Reachability for frames received
      on each ingress port: "Are frames received from ingress port i
      allowed to be forwarded on egress port j?" The same restriction applies
      as above.
    * *fl_domain* indicates the Flood Domain: "Which switch ports should
      receive a packet coming from ingress port i whose MAC destination
      is unknown? (not present in the L2 Address Lookup table)"
      Same restriction as above applies.
    * *vlan_pmap* can be used to remap VLAN priorities of ingress packets
      to different values on egress. In this example the remapping feature
      is not used, so vlan_pmap[i] = i.
* The last 8 entries in the L2 Forwarding Table are per-VLAN egress
  priority (determined from the first 5 entries).
    * *vlan_pmap* is the only meaningful attribute for these entries,
      however it carries a different meaning than for the first 5.
      It is used to map the pair (egress VLAN priorities x ingress port) to
      one of the 8 physical Priority Queues per egress port. The egress VLAN
      priority of the port is implied from the entry index (must subtract 5),
      and the ingress port number is the "i" in vlan_pmap[i].
      Since there are only 5 ports, the last 3 values in each vlan_pmap[i]
      do not carry any meaning.
* The same L2 Forwarding Table configuration described above can be obtained
  through the following shell script:

```bash
#!/bin/bash

sja1105-tool config new
for i in {0..4}; do
	mask=$((0x1f - 2**$i))
	sja1105-tool conf mod l2-forwarding-table[$i] bc_domain $mask
	sja1105-tool conf mod l2-forwarding-table[$i] reach_port $mask
	sja1105-tool conf mod l2-forwarding-table[$i] fl_domain $mask
	sja1105-tool conf mod l2-forwarding-table[$i] vlan_pmap \
	             "[0 1 2 3 4 5 6 7]"
done
for i in {0..7}; do
	sja1105-tool conf mod l2-forwarding-table[$((5 + $i))] vlan_pmap \
	             "[$i $i $i $i $i 0 0 0]"
done
sja1105-tool conf save example.xml
```

BUGS
====

The following configuration tables are currently not implemented:

* "clock-synchronization-parameters-table"
* "retagging-table"

All fields, irrespective of their type (including e.g. MAC addresses,
but excepting the "index" field, which needs to be decimal for NETCONF
compliance), are saved by the sja1105-tool to XML as hexadecimal values.
However, the values in the XML can still be edited manually in any
format.

AUTHOR
======

_sja1105-tool_ was written by Vladimir Oltean <vladimir.oltean@nxp.com>

SEE ALSO
========

sja1105-conf(5),
sja1105-tool-config(1),
sja1105-tool(1)

COMMENTS
========

This man page was written using [pandoc](http://pandoc.org/) by the same author.


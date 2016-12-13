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
signification of the fields (attributes) of each entry is not discussed here,
only the relationship between the XML description and tables found in the PDF.
For detailed information about the table entries and fields, please consult
UM10944.pdf.

Below is an approximate description of the Document Type Definition (DTD) for
the XML language used to describe the SJA1105 configuration tables:

```xml
<!ELEMENT config (table*)>
<!ELEMENT table (entry*)>
<!ATTLIST table name CDATA #REQUIRED>
<!ELEMENT entry EMPTY>
<!ATTLIST entry ...>
```

The DTD above shows there is one root node, of type "config", with zero or more
children nodes of type "table". Each table is distinguished through its "name"
attribute, which is a string that can take the values described in section
[CONFIGURATION TABLE NAMES].

The attribute list of an entry varies according to what table it is a member of.

The numeric values for each entry field (attribute) are stored by _sja1105-tool_
as unsigned 64-bit numbers. They must be specified inside quotes, and can be
expressed in either base 10 (e.g. "15"), base 16 (e.g. "0xf"), base 2
(e.g. "0b1111") or base 8 (e.g. "017"), whichever is more convenient for
representation. If 

Fields which are defined as arrays (e.g. "vlan-pmap") should be expressed as a
list of space-separated numbers, encased in square brackets. The restrictions
above still apply, e.g. "[0x1 2 0b11 0x3 04]" can be used to describe the
array {1, 2, 3, 4}.

One can still define entries with attributes that are not explicitly listed
below. The XML parser will simply ignore them. They can be used to aide a
human reader, for example numbering the entries by using an "index" attribute.

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
* "xmii-mode-parameters-table"

SCHEDULE TABLE
--------------

Table name "schedule-table" refers to "Chapter 4.2.1 Schedule Table" in
UM10944.pdf. It contains 0-1024 entries with the following attributes:

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
following attributes:

* "subschindx", bits 31-29
* "delta", bits 28-11
* "address", bits 10-1

L2 ADDRESS LOOKUP TABLE
-----------------------

Table name "l2-address-lookup-table" refers to "Chapter 4.2.6 L2
Address Lookup Table" in UM10944.pdf. It contains 0-1024 entries with the
following attributes:

* "vlanid", bits 95-84
* "macaddr", bits 83-36
* "destports", bits 35-31
* "enfport", bits 30-30
* "index", bits 29-20

L2 POLICING TABLE
-----------------

Table name "l2-policing-table" refers to "Chapter 4.2.7 L2 Policing
Table" in UM10944.pdf. It contains 40 or 45 entries with the following
attributes:

* "sharindx", bits 63-58
* "smax", bits 57-42
* "rate", bits 41-26
* "maxlen", bits 25-15
* "partition", bits 14-12

VLAN LOOKUP TABLE
-----------------

Table name "vlan-lookup-table" refers to "Chapter 4.2.8 VLAN Lookup
Table" in UM10944.pdf. It contains 0-4096 entries with the following
attributes:

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
attributes:

* "bc_domain", bits 63-59
* "reach_port", bits 58-54
* "fl_domain", bits 53-49
* "vlan_pmap", bits 48-25

MAC CONFIGURATION TABLE
-----------------------

Table name "mac-configuration-table" refers to "Chapter 4.2.10 MAC
Configuration Table" in UM10944.pdf. It contains 5 entries with the
following attributes:

* "top", bits 223-215, ..., 90-82
* "base", bits 214-206, ..., 81-73
* "enabled", bits 205-205, ..., 72-72
* "ifg", bits 71-67
* "speed", bits 66-65
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

SCHEDULE PARAMETERS TABLE
-------------------------

Table name "schedule-parameters-table" refers to "Chapter 4.2.11 Schedule
Parameters" in UM10944.pdf. It contains 1 entry with the following attributes:

* "subscheind", bits 95-16

SCHEDULE ENTRY POINTS PARAMETERS TABLE
--------------------------------------

Table name "schedule-entry-points-parameters-table" refers to "Chapter 4.2.12
Schedule Entry Points Parameters" in UM10944.pdf. It contains 1 entry with the
following attributes:

* "clksrc", bits 31-30
* "actsubsch", bits 29-27

L2 LOOKUP PARAMETERS TABLE
--------------------------

Table name "l2-lookup-parameters-table" refers to "Chapter 4.2.12 L2 Lookup
Parameters" in UM10944.pdf. It contains 1 entry with the following attributes:

* "maxage", bits 31-17
* "dyn_tbsz", bits 16-14
* "poly", bits 13-6
* "shared_learn", bits 5-5
* "no_enf_hostprt", bits 4-4
* "no_mgmt_learn", bits 3-3

L2 FORWARDING PARAMETERS TABLE
------------------------------

Table name "l2-forwarding-parameters-table" refers to "Chapter 4.2.12 L2
Forwarding Parameters" in UM10944.pdf. It contains 1 entry with the following
attributes:

* "max_dynp", bits 95-93
* "part_spc", bits 92-13

GENERAL PARAMETERS TABLE
------------------------

Table name "general-parameters-table" refers to "Chapter 4.2.18 General
Parameters" in UM10944.pdf. It contains 1 entry with the following attributes:

* "vllupformat", bits 95-93
* "part_spc", bits 92-13

XMII MODE PARAMETERS TABLE
--------------------------

Table name "xmii-mode-parameters-table" refers to "Chapter 4.2.20 xMII
Mode Parameters" in UM10944.pdf. It contains 1 entry with the following
attributes:

* "phy_mac", bits 31, 28, 25, 22, 19
* "xmii_mode", bits 30-29, 27-26, 24-23, 21-20, 18-17

EXAMPLES
========

To get a minimal SJA1105 configuration for the LS1021ATSN board with only the
mandatory configuration tables loaded:

```bash
#!/bin/bash

# Create the staging area
sja1105-tool conf default ls1021atsn-standard
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
    * "bc_domain" indicates the Broadcast Domain. The only limitation
      imposed by default is that broadcast frames received on an
      interface are not to be forwarded on the port it came from.
      Thus, the broadcast domain of Port 0 is 0b11110 (0xF, Ports 1 to 4), etc.
    * "reach_port" indicates the Port Reachability for frames received
      on each ingress port. The same restriction applies as above.
    * "fl_domain" indicates the Flood Domain (which switch ports should receive
      a packet whose destination is unknown (not present in the L2
      Address Lookup table). Same restriction as above applies.
    * "vlan_pmap" can be used to remap VLAN priorities of ingress packets
      to different values on egress. Since this feature is unused,
      vlan_pmap[i] = i.
* The last 8 entries in the L2 Forwarding Table are per-VLAN egress
  priority (determined from the first 5 entries).
    * "vlan_pmap" is the only meaningful attribute for these entries,
      and is used to map the pair (egress VLAN priorities x ingress port) to
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

* "vl-lookup-table"
* "vl-policing-table"
* "vl-forwarding-table" 
* "vl-forwarding-parameters-table"
* "clock-synchronization-parameters-table"
* "avb-parameters-table"
* "retagging-table"

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


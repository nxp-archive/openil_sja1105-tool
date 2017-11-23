LS1021ATSN Configuration Helpers
================================

This directory provides a set of wrapper scripts over `sja1105-tool`
that make it easier to configure the 802.1Qbv engine.
For this purpose, sja1105-tool is intended to be run as a host
application, just for generating a configuration in XML format.
This can be then transferred (SSH, NETCONF etc) to the development board
and loaded by the `sja1105-tool` running on the target.

Instructions
------------

```bash
[host] $ source envsetup
[host] $ ./setup.sh
[host] $ ./configs/3-board-demo.sh --board 1
[host] $ scp ./configs/3-board-demo-board1.xml root@172.15.0.1:.
[target] $ sja1105-tool config load -f 3-board-demo-board1.xml
```

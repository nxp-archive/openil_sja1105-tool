#!/bin/bash

set -e -u -o pipefail

usage() {
	echo "Usage:"
	echo "$0 -h|--help"
	echo "$0 -b|--board {1|2|3}"
}

[ -z "${TOPDIR+x}" ] && { echo "Please source envsetup before running this script."; exit 1; }

# Chassis ETH2: Switch port RGMII 1
# Chassis ETH3: Switch port RGMII 2
# Chassis ETH4: Switch port RGMII 3
# Chassis ETH5: Switch port RGMII 0
# To LS1021:    Switch port RGMII 4
#
sja1105-tool config default ls1021atsn

O=`getopt -l help,board: -- hb: "$@"` || exit 1
eval set -- "$O"
while true; do
	case "$1" in
	-h|--help)
		usage; exit 0;;
	-b|--board)
		board="$2"; shift 2;;
	--)
		shift; break;;
	*)
		echo "error parsing $1"; exit 1;;
	esac
done
[ "$#" -gt 0 ] && { echo "error: trailing arguments: $@"; exit 1; }
[ -z "${board+x}" ] && { echo "please provide an argument to --board"; exit 1; }

case ${board} in
1)	fw_port="1"; bw_port="4"; start_time="1";;
2)	fw_port="1"; bw_port="2"; start_time="2";;
3)	fw_port="4"; bw_port="2"; start_time="3";;
*)	echo "invalid board index ${board}."
	exit 1
esac

# Extend ingress policer MTU to include VLAN tag
for port in $(seq 0 4); do
	for prio in $(seq 0 7); do
		policer-limit --port ${port} --prio ${prio} --mtu 1522 --rate-mbps 1000
	done
done

sja1105-tool config modify vlan-lookup-table entry-count 2
# Create VLAN ID 100 on switch (configured in
# /etc/init.d/S45vlan in OpenIL)
sja1105-tool config modify vlan-lookup-table[1] vmemb_port 0x1F
sja1105-tool config modify vlan-lookup-table[1] vlan_bc    0x1F
sja1105-tool config modify vlan-lookup-table[1] vlanid     100
sja1105-tool config modify vlan-lookup-table[1] tag_port   0x1F

policer-limit --port ${bw_port} --prio 0 --mtu 1522 --rate-mbps 500

if [[ ${board} -eq 1 ]]; then
scheduler-create << EOF
{
	"clksrc": "standalone",
	"cycles": [
		{
			"start-time-ms": "${start_time}",
			"timeslots": [
				{
					"duration-ms": "45",
					"ports": [${fw_port}],
					"gates-open": [0, 1, 2, 3, 4, 5, 6],
					"comment": "regular traffic"
				},
				{
					"duration-ms": "0.1",
					"ports": [${fw_port}],
					"gates-open": [],
					"comment": "guard-band"
				},
				{
					"duration-ms": "0.000001",
					"ports": [${fw_port}],
					"gates-open": [7],
					"comment": "protected ping"
				}
			]
		}
	]
}
EOF
fi

xml_name="${BASH_SOURCE[0]/.sh}-board${board}.xml"
rm -f ${xml_name}
sja1105-tool config save ${xml_name}
echo "Configuration saved as ${xml_name}."
echo "View with: \"sja1105-tool config load ${xml_name}; sja1105-tool config show | less\""

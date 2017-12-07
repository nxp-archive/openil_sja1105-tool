#!/bin/bash

set -e -u -o pipefail

usage() {
	echo "Usage:"
	echo "$0 -h|--help"
	echo "$0 -f|--flow1-prio {0..7}  -F|--flow2-prio {0..7}"
}

[ -z "${TOPDIR+x}" ] && { echo "Please source envsetup before running this script."; exit 1; }

# Chassis ETH2: Switch port RGMII 1
# Chassis ETH3: Switch port RGMII 2
# Chassis ETH4: Switch port RGMII 3
# Chassis ETH5: Switch port RGMII 0
# To LS1021:    Switch port RGMII 4
#
sja1105-tool config default ls1021atsn

O=`getopt -l help,flow1-prio:,flow2-prio:,flow1-rate-mbps: -- hf:F:r: "$@"` || exit 1
eval set -- "$O"
while true; do
	case "$1" in
	-h|--help)
		usage; exit 0;;
	-f|--flow1-prio)
		flow1_prio="$2"; shift 2;;
	-F|--flow2-prio)
		flow2_prio="$2"; shift 2;;
	-r|--flow1-rate-mbps)
		flow1_rate="$2"; shift 2;;
	--)
		shift; break;;
	*)
		echo "error parsing $1"; exit 1;;
	esac
done
[ "$#" -gt 0 ] && { echo "error: trailing arguments: $@"; exit 1; }
[ -z "${flow1_prio+x}" ] && { echo "please provide an argument to --flow1-prio"; exit 1; }
[ -z "${flow2_prio+x}" ] && { echo "please provide an argument to --flow2-prio"; exit 1; }
[ -z "${flow1_rate+x}" ] && { echo "please provide an argument to --flow1-rate"; exit 1; }

sja1105-tool config default ls1021atsn

# Content starts here

# Flow 1 is coming from Board 1. From Board 2's perspective, this
# is connected to chassis port ETH3 (RGMII 2).
# Flow 2 is coming from Board 2. This is the internal port RGMII4.
sja1105-tool config modify mac-config[2] vlanprio ${flow1_prio}
sja1105-tool config modify mac-config[4] vlanprio ${flow2_prio}

policer-limit --port 2 --prio ${flow1_prio} --mtu 1518 --rate-mbps ${flow1_rate}

# Content ends here

xml_name="${BASH_SOURCE[0]/.sh}.xml"
rm -f ${xml_name}
sja1105-tool config save ${xml_name}
echo "Configuration saved as ${xml_name}."
echo "View with: \"sja1105-tool config load ${xml_name}; sja1105-tool config show | less\""


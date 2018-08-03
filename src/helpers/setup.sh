#!/bin/bash

set -e -u -o pipefail

[ -z "${TOPDIR+x}" ] && { echo "Please source envsetup before running this script."; exit 1; }

SJA1105_TOPDIR="${TOPDIR}/../.."
BUILD_DIR="${TOPDIR}/_build"
OBJ_DIR="${BUILD_DIR}/obj"
if [ -d ${BUILD_DIR} ]; then
	echo "sja1105-tool already built, exiting"
	exit
fi
make -C ${SJA1105_TOPDIR} clean
DESTDIR=${BUILD_DIR} O=${OBJ_DIR} make -C ${SJA1105_TOPDIR} -j $(nproc) \
	install-binaries \
	install-configs \
	install-manpages \
|| { echo "Build failed, cleaning up"; rm -rf ${BUILD_DIR}; exit 1; }

# Fix up staging area path
sed -i "s%staging_area.*%staging_area = ${TOPDIR}/sja1105.bin%" \
	${BUILD_DIR}/etc/sja1105/sja1105.conf
# Remove unnecessary init script
rm -rf ${BUILD_DIR}/etc/init.d

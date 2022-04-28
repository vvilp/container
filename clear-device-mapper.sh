PARAM_ON_SOURCE="/home/nextdc/source/image-data/images"
DATAFile="/home/nextdc/source/image-data/data"
DEVICE_MAPPER_NAME="test-mapper"
MAPPER_DIR="/dev/mapper/"
MOUNTING_POINT="/home/nextdc/source/image-box"
MOUNTING_POINT_TEMP="/home/nextdc/source/data-box"

umount ${MOUNTING_POINT}
umount ${MOUNTING_POINT_TEMP}

rm -rf ${MOUNTING_POINT}

rm -rf ${MOUNTING_POINT_TEMP}

dmsetup remove ${DEVICE_MAPPER_NAME}

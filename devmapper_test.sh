

IMAGEFile="/home/nextdc/source/image-data/images"
DATAFile="/home/nextdc/source/image-data/data"
DEVICE_MAPPER_NAME="test-mapper"
MAPPER_DIR="/dev/mapper/"
MOUNTING_POINT="/home/nextdc/source/image-box"
MOUNTING_POINT_TEMP="/home/nextdc/source/data-box"

mkdir ${MOUNTING_POINT}

LOOP_DEVICE=`losetup --show --find ${IMAGEFile}`

DEVICE_SIZE=`blockdev --getsz ${IMAGEFile}`

SANDBOX_FILE_DEVICE=`losetup --show --find ${DATAFile}`

dmsetup create "${DEVICE_MAPPER_NAME}" --table "0 ${DEVICE_SIZE} snapshot ${LOOP_DEVICE} ${SANDBOX_FILE_DEVICE} P 16"

mount ${MAPPER_DIR}${DEVICE_MAPPER_NAME} ${MOUNTING_POINT}

# -----------------------------------------------------------------------------------------------------------------
# some other commands
# -----------------------------------------------------------------------------------------------------------------
#dd if=/dev/zero of=${tempSwapFile} bs=1 count=0 seek=`expr 512 \* ${DEVICE_SIZE}`
#dd if=/dev/zero of=${tempSwapFile} bs=1M count=10
#mkfs.ext2 -F ${tempSwapFile}

#dmsetup create test-snapshot-base-real --table "0 ${DEVICE_SIZE} linear ${LOOP_DEVICE} 0"

#dmsetup create test-snapshot-snap-cow --table "0 ${DEVICE_SIZE} linear ${SANDBOX_FILE_DEVICE} 0"

#dmsetup create test-snapshot-base --table "0 ${DEVICE_SIZE} snapshot-origin /dev/mapper/test-snapshot-base-real"

#dmsetup create test-snapshot-cow --table "0 ${DEVICE_SIZE} snapshot /dev/mapper/test-snapshot-base-real /dev/mapper/test-snapshot-snap-cow P 8"


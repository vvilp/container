
mkdir image-data

#make image file
dd if=/dev/zero of=./image-data/images bs=500M count=1

mkfs.ext2 -F ./image-data/images

mkdir tmp-images

mount -o loop ./image-data/images tmp-images/

cp -rf ./arch-root/* tmp-images/

umount tmp-images

#make container file
dd if=/dev/zero of=./image-data/data bs=10M count=10

mkfs.ext2 -F ./image-data/data

rm -rf tmp-images/

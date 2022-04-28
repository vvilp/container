All these files can be found and test in fedora(virtualbox)
username: nextdc
password: nextdc
folder: /home/nextdc/source

Attention:
If you copy all these files to your Linux system, please change the the path according your Linux path in all these scripts and codes

Script and Code List:

make-devicemapper-files.sh
use this script to make Docker image file and container file.
Docker image file is made from a common Linux root folder.
arch_root is a archlinux folder(your can extract arch_root.tar.gz by yourself)
image-data/image is a Docker image file which is made by this script.  

devmapper_test.sh
test device mapper using current image file and container file.

clear-device-mapper.sh
clean the device mapper files in current system

libcontainer_network.sh
test the network creating and settings by Docker.

clone_test.c 
test the clone function in Linux

test_pivot_root.c
test pivot_root function in Linux

namespace_test.c
test namespace features in Linux

NextdcContainer.c
A full functional Linux container programme.
Use Method:
	./NexdcContainer image-data/images image-data/container1



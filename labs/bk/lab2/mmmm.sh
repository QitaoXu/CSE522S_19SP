module add raspberry
KERNEL=kernel7
LINUX_SOURCE=/tmp/compile/liu433/linux_source/linux-raspberrypi-kernel_1.20180817-1/
make -C $LINUX_SOURCE ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- SUBDIRS=$PWD modules

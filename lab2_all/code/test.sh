module add raspberry
KERNEL=kernel7
LINUX_SOURCE=/tmp/compile/zhe.wang/linux_source/linux
make -C $LINUX_SOURCE ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- SUBDIRS=$PWD modules

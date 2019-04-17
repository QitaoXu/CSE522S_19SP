#!/bin/bash

module add raspberry
KERNEL=kernel7
echo $KERNEL
LINUX_SOURCE=/tmp/compile/qitao/linux_source/linux
echo $LINUX_SOURCE
make -C $LINUX_SOURCE ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- SUBDIRS=$PWD modules
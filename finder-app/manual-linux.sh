#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

#A Script to cross compile linux
#Author: Daniel Mendez
#Course : ECEN 5713: Advanced Embedded Software Development

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

  make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper
  make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
  make -j8 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
  make -j8 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
  make -j8 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs

fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

#Create the rootfs directory if it doesnt exist
if [ ! -d "${OUTDIR}/rootfs" ]; then
    mkdir -p "${OUTDIR}/rootfs"
fi

#Create the required directories
# List of directories to create
directories=("bin" "dev" "etc" "home" "conf" "lib" "lib64" "proc" "sbin" "sys" "lib/modules/${KERNEL_VERSION}" "tmp" "usr/bin" "usr/sbin" "var/log")

# Loop through the directories and create them if they don't exist
for dir in "${directories[@]}"; do
    if [ ! -d "${OUTDIR}/rootfs/${dir}" ]; then
        mkdir -p "${OUTDIR}/rootfs/${dir}"
    fi
done


# COnfigure and compiliation of busybox
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
else
    cd busybox
fi

make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX="${OUTDIR}/rootfs" ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install


# Copy library dependencies required for busybox (Thanks Aamir)
echo "Library dependencies"


SYSROOT=`aarch64-none-linux-gnu-gcc -print-sysroot`
echo "SYSROOT IS ${SYSROOT}"


cd "$OUTDIR/rootfs"


#${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
#${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

#Copy the interpreter
sudo cp "${SYSROOT}/lib/ld-linux-aarch64.so.1" lib

#Copy the shared libraries
sudo cp "${SYSROOT}/lib64/libm.so.6" lib64
sudo cp "${SYSROOT}/lib64/libresolv.so.2" lib64
sudo cp "${SYSROOT}/lib64/libc.so.6" lib64

#Make the device Nodes

sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

#Build the writer application
cd "${FINDER_APP_DIR}"
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

#Copy the findet and related files to the home directory
cp "./finder.sh" "$OUTDIR/rootfs/home"
cp -r "../conf" "$OUTDIR/rootfs/home"
cp "./finder-test.sh" "$OUTDIR/rootfs/home"
cp "./writer" "$OUTDIR/rootfs/home"
cp "./autorun-qemu.sh" "$OUTDIR/rootfs/home"


#Chown the root directory
cd "$OUTDIR/rootfs"
sudo chown -R root:root ./*

#Create the .cpio file and zip it
find . | cpio -H newc -ov --owner root:root > ../initramfs.cpio
gzip -f "${OUTDIR}/initramfs.cpio"
cd "$OUTDIR"

echo "SCRIPT COMPLETE"


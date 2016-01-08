#!/bin/bash

OUT_DIR="./OUT"
TOOLCHAIN="/home/josegalre/Android/cm-11.0/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin/arm-eabi-"
DEFCONFIG="a11ul_defconfig"

echo " Cleaning out dir..."
rm -rf $OUT_DIR
mkdir -p $OUT_DIR/kernel
mkdir -p $OUT_DIR/modules
 
echo " Cleaning kernel (make mrproper)..."
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN mrproper
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN clean

echo " Building kernel (make $DEFCONFIG)..."
make -j4 ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN $DEFCONFIG
make -j4 ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN

echo " Copying kernel (zImage) to $OUT_DIR/kernel/..."
cp arch/arm/boot/zImage $OUT_DIR/kernel/

echo " Copying modules (*.ko) to $OUT_DIR/modules/..."
find . -path "$OUT_DIR" -prune -o -name  \*.ko -exec cp '{}' "$OUT_DIR"/modules/ ';'

echo " Striping modules (reduce size)"
for x in "$OUT_DIR"/modules/* ; do
    "$TOOLCHAIN"strip --strip-debug $x
done

echo " All done!..."

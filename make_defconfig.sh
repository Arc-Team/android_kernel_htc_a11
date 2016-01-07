#!/bin/bash

TOOLCHAIN="/home/josegalre/Android/cm-11.0/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin/arm-eabi-"
DEFCONFIG="a11ul_defconfig"

echo " First Cleaning kernel (make mrproper)..."
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN mrproper
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN clean

echo " Making defconfig..."
make $DEFCONFIG
mv .config arch/arm/configs/$DEFCONFIG

echo " Final Cleaning kernel (make mrproper)..."
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN mrproper
make ARCH=arm SUBARCH=arm CROSS_COMPILE=$TOOLCHAIN clean

echo " All done!..."

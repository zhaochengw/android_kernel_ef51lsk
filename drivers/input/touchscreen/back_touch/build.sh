#!/bin/bash
###############################################################################
#                           Driver Module Build Script 
###############################################################################


cp -f Makefile Makefile_backup
cp -f Makefile_module Makefile

export ARCH=arm
export CROSS_COMPILE=../../../../../../prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-

make 

mv -f Makefile_backup Makefile

#
# Makefile for the back touch
#

obj-$(CONFIG_TOUCHSCREEN_PP8360) += tchkey_pt.o
#obj-m += tchkey_pt.o

ANDROID_ROOT_DIR:=../../../../../
KDIR := $(ANDROID_ROOT_DIR)out/target/product/msm8960/obj/KERNEL_OBJ/
PWD := $(shell pwd)
ARCH:=arm
CROSS_COMPILE:=$(ANDROID_ROOT_DIR)../prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-

#EXTRA_CFLAGS += -include ../../../../../../../BOARD_REV.h

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -f -r *.o *.ko .*cmd *.mod.c .tmp* core *.i modules.order Module.symvers modules.builtin

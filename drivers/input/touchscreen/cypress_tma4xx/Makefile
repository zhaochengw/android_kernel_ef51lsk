#
# Makefile for the earjack drivers.
#

obj-$(CONFIG_TOUCHSCREEN_CYTTSP_GEN4) += cyttsp4_core.o cyttsp4_i2c.o cyttsp4_pm8921.o
#obj-m += cyttsp4_core.o cyttsp4_i2c.o cyttsp4_pm8921.o

ANDROID_ROOT_DIR:=../../../../../
KDIR := $(ANDROID_ROOT_DIR)out/target/product/msm8960/obj/KERNEL_OBJ/
PWD := $(shell pwd)
ARCH:=arm
CROSS_COMPILE:=$(ANDROID_ROOT_DIR)../prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-

#EXTRA_CFLAGS += -include ../../../../../../../BOARD_REV.h

CYTTSP_GEN4_DEBUG=n

ifeq ($(CYTTSP_GEN4_DEBUG),y)
EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DVERBOSE_DEBUG#VERBOSE_DEBUG
endif

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -f -r *.o *.ko .*cmd *.mod.c .tmp* core *.i modules.order Module.symvers modules.builtin

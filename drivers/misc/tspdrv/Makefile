#=========================================================================
# File:
#    Makefile
#
# Description: 
#    Makefile for TouchSense Linux Kernel Module
#    Targets Kernel 2.6
#
# Portions Copyright (c) 2008-2009 Immersion Corporation. All Rights Reserved. 
# 
# This file contains Original Code and/or Modifications of Original Code 
# as defined in and that are subject to the GNU Public License v2 - 
# (the 'License'). You may not use this file except in compliance with the 
# License. You should have received a copy of the GNU General Public License 
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact 
# TouchSenseSales@immersion.com.
# 
# The Original Code and all software distributed under the License are 
# distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
# EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
# INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
# FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
# the License for the specific language governing rights and limitations 
# under the License.
# =========================================================================

obj-$(CONFIG_PIEZO) += tspdrv.o
#obj-m += tspdrv.o

ANDROID_ROOT_DIR:=../../../../
KDIR := $(ANDROID_ROOT_DIR)out/target/product/msm8960/obj/KERNEL_OBJ/
PWD := $(shell pwd)
ARCH:=arm
CROSS_COMPILE:=$(ANDROID_ROOT_DIR)../../prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi-

EXTRA_CFLAGS += -DDEBUG
EXTRA_CFLAGS += -DVERBOSE_DEBUG
EXTRA_CFLAGS += -DVIBE_DEBUG

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	rm -f -r *.o *.ko .*cmd *.mod.c .tmp* core *.i modules.order Module.symvers modules.builtin

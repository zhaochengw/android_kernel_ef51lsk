/* drivers/misc/pantech_restart.c
 *
 * Copyright (C) 2011 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <mach/msm_iomap.h>
#include <linux/io.h>
#include <linux/console.h>
#include "smd_private.h"



static int ramdump = 0;
static int usbdump = 0;
static int errortest = 0;
static int mdmdump = 0;

typedef enum {
    MODEM_ERR_FATAL=10,
    MODEM_WATCHDOG=11,
    MODEM_EXCP_SWI=12,
    MODEM_EXCP_UNDEF=13,
    MODEM_EXCP_MIS_ALIGN=14,  
    MODEM_EXCP_PAGE_FAULT=15,
    MODEM_EXCP_EXE_FAULT=16,
    MODEM_EXCP_DIV_BY_Z=17,
    LINUX_ERR_FATAL=20,
    LINUX_WATCHDOG=21,
    LINUX_EXCP_SWI=22,
    LINUX_EXCP_UNDEF=23,
    LINUX_EXCP_MIS_ALIGN=24,  
    LINUX_EXCP_PAGE_FAULT=25,
    LINUX_EXCP_EXE_FAULT=26,
    LINUX_EXCP_DIV_BY_Z=27,
    SUBSYSTEM_ERR_MAX_NUM
}subsystem_err_type;

typedef struct
{
  unsigned long magic_1;  /**< Most-significant word of the predefined magic value. */
  unsigned long magic_2;  /**< Least-significant word of the predefined magic value. */
} smem_hw_reset_id_type;

typedef smem_hw_reset_id_type err_reset_id_type;
static err_reset_id_type* dump_mode_check_ptr = NULL;



#define PANTECH_RAMDUMP_MAGIC         0xFF91901C
#define PANTECH_USBDUMP_MAGIC         0xC10919FF

static int pantech_prep_restart(struct notifier_block *this, unsigned long event, void *ptr) {
    return NOTIFY_DONE;
}

static struct notifier_block pantech_restart_blk = {
    .notifier_call    = pantech_prep_restart,
};

void dump_mode_init(void)
{
    unsigned smem_size = sizeof(smem_hw_reset_id_type);
    dump_mode_check_ptr = (err_reset_id_type *)smem_get_entry(SMEM_HW_RESET_DETECT, &smem_size);

    printk(KERN_INFO "magic_1 : 0x%08X\n", (unsigned int)dump_mode_check_ptr->magic_1);
    printk(KERN_INFO "magic_2 : 0x%08X\n", (unsigned int)dump_mode_check_ptr->magic_2);

    ramdump = 0;
    usbdump = 0;
	mdmdump = 0;	
}

typedef void (*func_ptr)(void);
static const int _undef_inst = 0xFF0000FF;
static int div0_y=0;
static void diag_error_data_abort(void)
{
    *(int *)0 = 0;
}

static void diag_error_prefetch_abort(void)
{
    func_ptr func;
    func = (func_ptr)0xDEADDEAD;
    (*func)();
}

static void diag_error_undefined_instruction(void)
{
    func_ptr func;
    func = (func_ptr)(&_undef_inst);
    (*func)();
}

static DEFINE_SPINLOCK(state_lock);

static void diag_error_sw_watchdog(void)
{
    unsigned long irqflags;
    unsigned long  value = 0;  

    spin_lock_irqsave(&state_lock, irqflags);
    while(1){
        value = value ? 0 : 1;
    }
    spin_unlock_irqrestore(&state_lock, irqflags);
}

static void diag_error_div0(void)
{
    int x = 1234567;
    int *y;
    y = &div0_y;
    x = x / *y;
}

static void linux_crash_test(int sel)
{
    switch(sel)
    {
        case LINUX_ERR_FATAL:
            BUG_ON(1);
            break;
        case LINUX_WATCHDOG:
            diag_error_sw_watchdog();
            break;
        case LINUX_EXCP_SWI:
            break;
        case LINUX_EXCP_UNDEF:
            diag_error_undefined_instruction();
            break;
        case LINUX_EXCP_MIS_ALIGN:
            break;            
        case LINUX_EXCP_PAGE_FAULT:
            diag_error_data_abort();
            break;
        case LINUX_EXCP_EXE_FAULT:
            diag_error_prefetch_abort();
            break;
        case LINUX_EXCP_DIV_BY_Z:
            diag_error_div0();
            break;
        default:
            break;
    }
}

static ssize_t ramdump_show(struct kobject *kobj, struct kobj_attribute *attr,
            char *buf)
{
    printk(KERN_INFO "%s : , ramdump : %d\n", __func__, ramdump);
    return sprintf(buf, "%d\n", ramdump);
}

static ssize_t ramdump_store(struct kobject *kobj, struct kobj_attribute *attr,
             const char *buf, size_t count)
{
    sscanf(buf, "%du", &ramdump);
    printk(KERN_INFO "%s : , ramdump : %d\n", __func__, ramdump);
    if(ramdump == 1)
        dump_mode_check_ptr->magic_1 = PANTECH_RAMDUMP_MAGIC;
    else
        dump_mode_check_ptr->magic_1 = 0;
    return count;
}

static struct kobj_attribute ramdump_attribute =
    __ATTR(ramdump, 0664, ramdump_show, ramdump_store);

static ssize_t usbdump_show(struct kobject *kobj, struct kobj_attribute *attr,
              char *buf)
{
    printk(KERN_INFO "%s : , usbdump : %d\n", __func__, usbdump);
    return sprintf(buf, "%d\n", usbdump);
}

static ssize_t usbdump_store(struct kobject *kobj, struct kobj_attribute *attr,
               const char *buf, size_t count)
{
    sscanf(buf, "%du", &usbdump);
    printk(KERN_INFO "%s : , usbdump : %d\n", __func__, usbdump);
    if(usbdump == 1)
        dump_mode_check_ptr->magic_2 = PANTECH_USBDUMP_MAGIC;
    else
        dump_mode_check_ptr->magic_2 = 0;
    return count;
}

static struct kobj_attribute usbdump_attribute =
    __ATTR(usbdump, 0664, usbdump_show, usbdump_store);

static ssize_t errortest_show(struct kobject *kobj, struct kobj_attribute *attr,
              char *buf)
{
    return sprintf(buf, "%d\n", errortest);
}

static ssize_t errortest_store(struct kobject *kobj, struct kobj_attribute *attr,
               const char *buf, size_t count)
{
    sscanf(buf, "%du", &errortest);
    linux_crash_test(errortest);

    return count;
}

static struct kobj_attribute errortest_attribute =
    __ATTR(errortest, 0664, errortest_show, errortest_store);

static ssize_t mdmdump_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if(mdmdump != 0 && mdmdump != 1)
		return sprintf(buf, "error unknown value : %d\n", mdmdump);
	else
		return sprintf(buf, "%d\n", mdmdump);
}

static ssize_t mdmdump_store(struct kobject *kobj, struct kobj_attribute *attr,
							   const char *buf, size_t count)
{
	sscanf(buf, "%d", &mdmdump);
	printk(KERN_INFO "%s : , mdmdump : %d\n", __func__, mdmdump);
		
	return count;
}

static struct kobj_attribute mdmdump_attribute =
		__ATTR(mdmdump, 0664, mdmdump_show, mdmdump_store);

static struct attribute *attrs[] = {
    &ramdump_attribute.attr,
    &usbdump_attribute.attr,
    &errortest_attribute.attr,
	&mdmdump_attribute.attr,
    NULL,
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject *pantech_restart_kobj;

int __init pantech_restart_init(void)
{
    int retval;
#if !defined(CONFIG_QC_ABNORMAL_DEBUG_CODE) || defined(CONFIG_PANTECH_USER_BUILD)
    void *phy_log_buf;
#endif
    pantech_restart_kobj = kobject_create_and_add("pantech_restart", kernel_kobj);
    if (!pantech_restart_kobj)
        return -ENOMEM;
    retval = sysfs_create_group(pantech_restart_kobj, &attr_group);
    if (retval)
        kobject_put(pantech_restart_kobj);

    atomic_notifier_chain_register(&panic_notifier_list, &pantech_restart_blk);

#if !defined(CONFIG_QC_ABNORMAL_DEBUG_CODE) || defined(CONFIG_PANTECH_USER_BUILD)
    phy_log_buf = (void*)virt_to_phys((void*)get_log_buf_addr());
    writel(phy_log_buf, MSM_IMEM_BASE + 0x65C + 0xC);
#else
    writel(0x88b00000, MSM_IMEM_BASE + 0x65C + 0xC);
#endif

    dump_mode_init();
    
    return retval;
}

arch_initcall(pantech_restart_init);

MODULE_AUTHOR("Pantech ls4 part1>");
MODULE_DESCRIPTION("Pantech Restart driver");
MODULE_LICENSE("GPL v2");

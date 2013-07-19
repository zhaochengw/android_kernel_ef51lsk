/*===========================================================================

               KSKY_RAWDATA.C

DESCRIPTION

Copyright (c) 2011 by Pantech All Rights Reserved.

===========================================================================*/

/*===========================================================================
                        EDIT HISTORY FOR MODULE

when                who                   what,where,why
----------  ------------     ------------------------------------------------
2012-03-13  Changju 		Init
============================================================================*/

/*******************************************************************************
* INCLUDE FILES
*******************************************************************************/
#include <linux/ctype.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/memory.h>
#include <linux/printk.h>
#include <linux/fs.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>

#include "ksky_rawdata.h"


/*******************************************************************************
* DEFINITIONS
*******************************************************************************/


/*******************************************************************************
* MACROS
*******************************************************************************/
#define LOG_TAG "KSKY_RAWDATA"
#define KLOGI(tag, fmt, ...) \
	printk( KERN_INFO tag " %s(%d): " fmt, __func__,__LINE__, ##__VA_ARGS__)

#define KLOGE(tag, fmt, ...) \
	printk( KERN_ERR tag " %s(%d): " fmt, __func__, __LINE__, ##__VA_ARGS__)


// from skytest_socket_srv_thread.c
#define BLK_PATH "/dev/block/mmcblk0p12"

/*******************************************************************************
* TYPE DEFINITIONS
*******************************************************************************/

/*******************************************************************************
* GLOBAL VARIABLES
*******************************************************************************/


/*******************************************************************************
* INTERNAL VARIABLES
*******************************************************************************/

/*******************************************************************************
* GLOBAL FUNCTIONS
*******************************************************************************/
/*******************************************************************************
* FUNCTIONS
*******************************************************************************/

int ksky_rawdata_read( unsigned int address, unsigned int size, char *buf )
{
	int fd=-1;
	int rd=0;
	int ret=0;

	mm_segment_t old_fs = get_fs();
	set_fs(KERNEL_DS);

        fd = sys_open(BLK_PATH, O_RDONLY, 0);
        if (fd < 0 ) {
		KLOGE( LOG_TAG, "file open fail %s\n", BLK_PATH) ;
		ret = -1;
		goto fin_;
	}

	sys_lseek( fd, (off_t)address, SEEK_SET);

	rd = sys_read(fd, buf, size);
	if( (int)rd < 0 ) {
		KLOGE(LOG_TAG, "read failed size=%d, ret=%d\n", size, rd);
		ret=-1;
		goto fin_;
	}
	
fin_:
	if( fd != -1) {
		sys_close(fd);
	}
	set_fs(old_fs);

	return ret;
}

#if 0
int ksky_rawdata_read( unsigned int address, unsigned int size, char *buf )
{
	struct file* fp=NULL;
	int rd=0;
	int ret=0;

        fp = filp_open(BLK_PATH, O_RDONLY, 0);
        if (fp == NULL) {
		KLOGE( LOG_TAG, "file open fail %s\n", BLK_PATH) ;
		ret = -1;
		goto fin_;
	}

	rd = kernel_read(fp, address, buf, size);
	if( (int)rd < 0 ) {
		KLOGE(LOG_TAG, "read failed size=%d, ret=%d\n", size, rd);
		ret=-1;
		goto fin_;
	}
	
fin_:
	if( fp != NULL) {
		filp_close(fp, NULL);
	}

	return ret;
}
#endif // 0

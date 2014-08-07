/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <mach/gpio.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/semaphore.h>
#include <linux/i2c.h>
#include <linux/wakelock.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_i2c.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/ti_drv2665.h>

static struct ti_drv2665_data* piezo = NULL;
static struct buffer_data sample_buffer[NUM_ACTUATORS] = {{0}}; 
/*
 * PROTO FUNC
 */
static int ti_drv2665_save_data(const char *outputDataBuffer, int count);
static bool ti_drv2665_send_data(void);
static void ti_drv2665_reset_data(void);
static void ti_drv2665_start_timer(void);
static void ti_drv2665_stop_timer(void);
static inline int sem_is_locked(struct semaphore *lock);
static enum hrtimer_restart ti_drv2665_timer_expire_func(struct hrtimer *timer);
static int ti_drv2665_process_data(void* data);
static void ti_drv2665_init_timer(void);
static void ti_drv2665_terminate_timer(void);
static int32_t ti_drv2665_init(void);
static int32_t ti_drv2665_terminate(void);
static int32_t ti_drv2665_sampling(u_int8_t index, u_int16_t nOutputSignalBitDepth, u_int16_t nBufferSizeInBytes, int8_t* pForceOutputBuffer);
static int __devinit ti_drv2665_probe(struct i2c_client* client, const struct i2c_device_id* id);
static int ti_drv2665_remove(struct i2c_client *client);
static int ti_drv2665_suspend(struct device *dev);
static int ti_drv2665_resume(struct device *dev);
static void platform_release(struct device *dev);
static int ti_drv2665_pm(bool enable);
static int open(struct inode *inode, struct file *file);
static int release(struct inode *inode, struct file *file);
static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos);
static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos);
static long unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static int suspend(struct device *dev);
static int resume(struct device *dev);

static struct file_operations fops = 
{
    .owner =            THIS_MODULE,
    .read =             read,
    .write =            write,
    .unlocked_ioctl =   unlocked_ioctl,
    .open =             open,
    .release =          release,
    .llseek =           default_llseek
};
static struct miscdevice miscdev = 
{
    .minor =    MISC_DYNAMIC_MINOR,
    .name =     MODULE_NAME,
    .fops =     &fops
};

static const struct dev_pm_ops ti_drv2665_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(suspend, resume)
};
static struct platform_driver platdrv = 
{
    .driver = 
    {		
        .name = MODULE_NAME,	
        .pm = &ti_drv2665_pm_ops,
    },	
};

static struct platform_device platdev = 
{	
    .name =     MODULE_NAME,	
    .id =       -1,                   
    .dev = 
    {
        .platform_data = NULL, 		
        .release = platform_release,  
    },
};

static struct i2c_board_info info = {
    I2C_BOARD_INFO(DRV2665_BOARD_NAME, DEVICE_ADDR),
};

static const struct i2c_device_id ti_drv2665_id[] =
{
    {DRV2665_BOARD_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, ti_drv2665_id);

static const struct dev_pm_ops ti_drv2665_i2c_pm_ops = {
    SET_SYSTEM_SLEEP_PM_OPS(ti_drv2665_suspend, ti_drv2665_resume)
};
static struct i2c_driver ti_drv2665_driver =
{
    .probe = ti_drv2665_probe,
    .remove = __devexit_p(ti_drv2665_remove),
    .id_table = ti_drv2665_id,
    .driver =
    {
        .name = DRV2665_BOARD_NAME,
        .owner = THIS_MODULE,
        .pm = &ti_drv2665_i2c_pm_ops,
    },
};

static int ti_drv2665_save_data(const char *outputDataBuffer, int count)
{
    int i = 0;
    if ((count < SPI_HEADER_SIZE) || (count > SPI_BUFFER_SIZE))
    {
        printk("ti_drv2665_save_data: invalid write buffer size.\n");
        return 0;
    }
    if (0 == outputDataBuffer)
    {
        printk("ti_drv2665_save_data: outputDataBuffer invalid.\n");
        return 0;
    }
    while (i < count)
    {
        struct buffer_data* pInputBuffer = (struct buffer_data*)(&outputDataBuffer[i]);
        if ((i + SPI_HEADER_SIZE) > count)
        {
            printk("ti_drv2665_save_data: invalid buffer index.\n");
        }
        if (8 != pInputBuffer->depth)
        {
            printk("ti_drv2665_save_data: invalid bit depth. Use default value (8).\n");
        }
        if ((i + SPI_HEADER_SIZE + pInputBuffer->size) > count)
        {
            printk("ti_drv2665_save_data: invalid data size.\n");
        }
        if (NUM_ACTUATORS <= pInputBuffer->index)
        {
            printk("ti_drv2665_save_data: invalid actuator index.\n");
            i += (SPI_HEADER_SIZE + pInputBuffer->size);
            continue;
        }
        if ((count == SPI_HEADER_SIZE) && (0 == pInputBuffer->size))
        {
            break;
        }
        memcpy(&(sample_buffer[pInputBuffer->index]), &outputDataBuffer[i], (SPI_HEADER_SIZE + pInputBuffer->size));
        i += (SPI_HEADER_SIZE + pInputBuffer->size);
    }

    return 1;
}

static bool ti_drv2665_send_data(void)
{
    int i;
    bool ret = 0;

    for (i = 0; i < NUM_ACTUATORS; i++) 
    {
        struct buffer_data *sample = &(sample_buffer[i]);
        if (sample->size)
        {
            ti_drv2665_sampling(i, sample->depth, sample->size,sample->dataBuffer);
            ret = 1;
            sample->size = 0;
        }
    }
    return ret;
}
static void ti_drv2665_reset_data(void)
{
    int i;
    for (i = 0; i < NUM_ACTUATORS; i++)
    {
        sample_buffer[i].size = 0;
    }
}

static inline int sem_is_locked(struct semaphore *lock)
{
    return (lock->count) < 1;
}

static enum hrtimer_restart ti_drv2665_timer_expire_func(struct hrtimer *timer)
{
    if (!piezo->timerStarted)
    {
        if (sem_is_locked(&piezo->sem))
        {
            up(&piezo->sem);
        }
        return  HRTIMER_NORESTART;
    }
    if (++piezo->wdCnt < WATCHDOG_TIMEOUT)
    {
        hrtimer_forward_now(timer, piezo->period);
    }		

    if (sem_is_locked(&piezo->sem))
    {
        up(&piezo->sem);
    }
    if (piezo->wdCnt < WATCHDOG_TIMEOUT)
    {
        return HRTIMER_RESTART;
    }
    else
    {
        ti_drv2665_stop_timer();
        return HRTIMER_NORESTART;
    }
}

static int ti_drv2665_process_data(void* data)
{
    ti_drv2665_send_data();
    piezo->wdCnt = 0;
    return 0;
}

static void ti_drv2665_init_timer(void)
{
    piezo->period = ktime_set(0, piezo->timerPeriodMs * 950000);
    hrtimer_init(&piezo->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    piezo->timer.function = ti_drv2665_timer_expire_func;
}

static void ti_drv2665_start_timer(void)
{
    piezo->wdCnt = 0;
    if (!piezo->timerStarted)
    {
        sema_init(&piezo->sem, NUM_EXTRA_BUFFERS);
        piezo->timerStarted = true;
        piezo->period = ktime_set(0, piezo->timerPeriodMs * 950000);
        hrtimer_start(&piezo->timer, piezo->period, HRTIMER_MODE_REL);
    }
    else
    {
        int res;  
        if(piezo->timerStarted)
        {            
            res = down_timeout(&piezo->sem , msecs_to_jiffies(20)); 
            if (res != 0)
            {
                printk("ti_drv2665_start_timer: down_interruptible interrupted by a signal.\n");
            }
        }

    }
    if(piezo->nackDetected == true)
    {
        int res;  
        if(piezo->timerStarted)
        {            		
            res = down_timeout(&piezo->sem , msecs_to_jiffies(20));
            if (res != 0)
            {
                printk("ti_drv2665_start_timer: down_interruptible interrupted by a signal.\n");
            }
        }		
    }
    ti_drv2665_process_data(NULL);
}

static void ti_drv2665_stop_timer(void)
{
    if (piezo->timerStarted)
    {
        piezo->timerStarted = false;
        if (sem_is_locked(&piezo->sem))
            up(&piezo->sem);
    }
    ti_drv2665_reset_data();
    piezo->isPlaying = false;
} 

static void ti_drv2665_terminate_timer(void)
{
    ti_drv2665_stop_timer();
    hrtimer_cancel(&piezo->timer);
    if (sem_is_locked(&piezo->sem))
        up(&piezo->sem);
}
static int32_t ti_drv2665_init(void)
{
    struct i2c_adapter* adapter; 
    struct i2c_client* client; 
    struct i2c_msg msg[2]; 
    printk("ti_drv2665_init.\n");
    adapter = i2c_get_adapter(DEVICE_BUS);
    if (adapter) {
        printk("found device bus %d\n",DEVICE_BUS);
        client = i2c_new_device(adapter, &info);
        if (client) {
            int retVal = i2c_add_driver(&ti_drv2665_driver);
            if (retVal) {
                printk("ti_drv2665_driver: Cannot add driver.\n");
                return -4;
            }
            printk("added driver. retVal=%d addr=0x%02x name=%s\n",retVal, client->addr, client->name);
        } else {
            printk("ti_drv2665_driver: Cannot create new device.\n");
            return -4;
        }

    } else {
        printk("ti_drv2665_driver: I2C Adapter not found.\n");
        return -4;
    }
    piezo->buf[0] = 0x02;
    piezo->buf[1] = DRV2665_TIMEOUT ; 

    msg[0].addr = piezo->client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;
    msg[0].buf = piezo->buf;
    i2c_transfer(piezo->client->adapter, msg, 1);

    piezo->buf[0] = 0x01;
    piezo->buf[1] = DRV2665_GAIN ;

    msg[0].addr = piezo->client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;
    msg[0].buf = piezo->buf;

    i2c_transfer(piezo->client->adapter, msg, 1);
    return 0;
}

static int32_t ti_drv2665_terminate(void)
{
    printk("ti_drv2665_terminate.\n");
    i2c_del_driver(&ti_drv2665_driver);
    i2c_unregister_device(piezo->client);
    return 0;
}
static int32_t ti_drv2665_sampling(u_int8_t index, u_int16_t nOutputSignalBitDepth, u_int16_t nBufferSizeInBytes, int8_t* pForceOutputBuffer)
{
    int txRes;              
    struct i2c_msg msg[2];  
    int i;              
    int32_t status = -4;

    //struct qup_i2c_dev * dev = i2c_get_adapdata(piezo->client->adapter);

    if (!piezo->client) return -4;
    if (nBufferSizeInBytes > VIBE_OUTPUT_SAMPLE_SIZE) return -4;
    if (index > 0) return -4;
    piezo->buf[0] = PLAY_CMD_PIEZO;
    if(piezo->nackDetected)
    {
        for (i=0; i< piezo->lastPacket_size; i++)
        {
            piezo->buf[i+1] = piezo->lastPacket[i];
        }
        piezo->nackDetected = false;
    }
    else
    {
        piezo->lastPacket_size = 0;
    }

    msg[0].addr = piezo->client->addr;
    msg[0].flags = 0;

    for (i=0; i< nBufferSizeInBytes; i++)
    {
        piezo->buf[i+piezo->lastPacket_size+1] = pForceOutputBuffer[i];
    }

    msg[0].len = nBufferSizeInBytes + piezo->lastPacket_size + 1;
    msg[0].buf = piezo->buf;

    if ((txRes = i2c_transfer(piezo->client->adapter, msg, 1)) >= 0)
    {
        return 0;
    }

    piezo->nackDetected = true;
    //piezo->lastPacket_size = dev->sent_cnt + 1;
    piezo->lastPacket_size = get_lastPacket()+1;
    //printk("%d\n" , piezo->lastPacket_size);
    if( piezo->lastPacket_size > 80)
        piezo->nackDetected = false;

    for(i=0; i < piezo->lastPacket_size ; i++)
    {
        piezo->lastPacket[i] = pForceOutputBuffer[nBufferSizeInBytes - piezo->lastPacket_size + i];
    }

    return status;
}
static int __devinit ti_drv2665_probe(struct i2c_client* client, const struct i2c_device_id* id)
{
    int nRet = 0;
    struct ti_drv2665_data* data;

    data = kzalloc(sizeof(struct ti_drv2665_data) , GFP_KERNEL);
    if(data == NULL){
        printk("fail to allocate mem\n");
        return -ENOMEM;
    }

    printk("ti_drv2665_probe start\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        nRet = -ENODEV;
        printk("ti_drv2665_probe: i2c_check_functionality failed.\n");
    }
    else
    {
        data->client = client;
        printk("addr=0x%02x name=%s\n",data->client->addr, data->client->name);
    }
    data->timerPeriodMs = 5;
    data->nackDetected = false;
    data->isPlaying = false;
    data->timerStarted = false;
    data->wdCnt = 0;
    data->lastPacket_size = 0;

    piezo = data;
    printk("ti_drv2665_probe end.\n");
    return nRet;
}

static int ti_drv2665_remove(struct i2c_client *client)
{
    printk("ti_drv2665_remove.\n");
    piezo->client = NULL;
    kfree(piezo);
    return 0;
}

static int ti_drv2665_suspend(struct device *dev)
{
    printk("ti_drv2665_suspend\n");
    return 0;
}

static int ti_drv2665_resume(struct device *dev)
{
    printk("ti_drv2665_resume\n");
    return 0;
}

static int open(struct inode *inode, struct file *file) 
{
    printk("ti_drv2665: open.\n");
    if (!try_module_get(THIS_MODULE)) return -ENODEV;
    return 0; 
}

static int release(struct inode *inode, struct file *file) 
{
    printk("ti_drv2665: release.\n");
    ti_drv2665_stop_timer();
    file->private_data = (void*)NULL;
    module_put(THIS_MODULE);
    return 0; 
}

static ssize_t read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
    *ppos = 0; 
    /*
    printk("%x\n" , (unsigned int)file->private_data);
    if (file->private_data != (void*)TI_DRV2665_MAGIC_NUMBER) 
    {
        printk("ti_drv2665: unauthorized write.\n");
        return 0;
    }*/
    if ((count < SPI_HEADER_SIZE) || (count > MAX_SPI_BUFFER_SIZE))
    {
        printk("ti_drv2665: invalid buffer size.\n");
        return 0;
    }
    if (0 != copy_from_user(piezo->writeBuffer, buf, count))
    {
        printk("ti_drv2665: copy_from_user failed.\n");
        return 0;
    }
    if (!ti_drv2665_save_data(piezo->writeBuffer, count))
    {
        printk("ti_drv2665: ti_drv2665_save_data failed.\n");
        return 0;
    }
    piezo->isPlaying = true;
    ti_drv2665_start_timer();

    return count;
}

static long unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static int suspend(struct device *dev)
{
    if (piezo->isPlaying)
    {
        printk("ti_drv2665: can't suspend, still playing effects.\n");
        return -EBUSY;
    }
    else
    {
        printk("ti_drv2665: suspend.\n");
        return 0;
    }
}

static int resume(struct device *dev) 
{	
    printk("ti_drv2665: resume.\n");
    return 0;  
}

static void platform_release(struct device *dev) 
{	
    printk("ti_drv2665: platform_release.\n");
}
static int ti_drv2665_pm(bool enable)
{
    int nRet;
    struct regulator *vreg_lvs4_1p8;

    vreg_lvs4_1p8 = regulator_get(NULL, "8921_lvs4");
    if(IS_ERR(vreg_lvs4_1p8)) {
        nRet = PTR_ERR(vreg_lvs4_1p8);
        return -EIO;
    }
    if(enable)
        nRet = regulator_enable(vreg_lvs4_1p8);
    else
        nRet = regulator_disable(vreg_lvs4_1p8);
    msleep(50);
    if(nRet<0) {
        return -EIO;
    }
    regulator_put(vreg_lvs4_1p8);

    return nRet;
}
static int __init drv2665_init(void)
{
    int nRet;
    printk("ti_drv2665: init_module.\n");
    ti_drv2665_pm(true);
    nRet = misc_register(&miscdev);
    if (nRet) 
    {
        printk("ti_drv2665: misc_register failed.\n");
        return nRet;
    }
    nRet = platform_device_register(&platdev);
    if (nRet) 
    {
        printk("ti_drv2665: platform_device_register failed.\n");
    }

    nRet = platform_driver_register(&platdrv);
    if (nRet) 
    {
        printk("ti_drv2665: platform_driver_register failed.\n");
    }
    ti_drv2665_init();
    ti_drv2665_init_timer();
    return 0;
}

static void __exit drv2665_exit(void)
{
    ti_drv2665_terminate_timer();
    ti_drv2665_terminate();

    platform_driver_unregister(&platdrv);
    platform_device_unregister(&platdev);

    misc_deregister(&miscdev);
}

module_init(drv2665_init);
module_exit(drv2665_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("TI DRV2665 chip driver");

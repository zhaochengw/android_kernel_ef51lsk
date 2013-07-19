#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include <mach/gpio.h>
#include <mach/gpiomux.h>

#include <linux/mfd/pm8xxx/max17058_battery.h>

#define MAX17058_VCELL_MSB	0x02
#define MAX17058_VCELL_LSB	0x03
#define MAX17058_SOC_MSB	0x04
#define MAX17058_SOC_LSB	0x05
#define MAX17058_MODE_MSB	0x06
#define MAX17058_MODE_LSB	0x07
#define MAX17058_VER_MSB	0x08
#define MAX17058_VER_LSB	0x09
#define MAX17058_RCOMP_MSB	0x0C
#define MAX17058_RCOMP_LSB	0x0D

#if defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define SKY_SOC_EMPTY	4000	// 3 X 1000
//#define SKY_SOC_FULL	100000	// 100 X 1000
#define SKY_SOC_FULL	97000	// 99 X 1000
#define SKY_DEFAULT_RCOMP	0x5F // 95 : 20 C
#define SKY_TEMP_CO_HOT		0
#define SKY_TEMP_CO_COLD	-3750 // -3.75
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define SKY_SOC_EMPTY	6390	// 6.39 X 1000
#define SKY_SOC_FULL	99000	// 99 X 1000
#define SKY_DEFAULT_RCOMP	0x48
#define SKY_TEMP_CO_HOT		125 // -0.125 X 1000
#define SKY_TEMP_CO_COLD	-4350 // -4.35 X 1000
#else
#define SKY_SOC_EMPTY	2800	// 2.8 X 1000
#define SKY_SOC_FULL	98200	// 98.2 X 1000
#define SKY_DEFAULT_RCOMP	0x60
#define SKY_TEMP_CO_HOT		125 // -0.125
#define SKY_TEMP_CO_COLD	-3425 // -3.425
#endif

#define SKY_MULT_10(x) 	(x*10)
#define SKY_MULT_100(x) 	(x*100)
#define SKY_MULT_1000(x) 	(x*1000)

#if defined(CONFIG_MACH_APQ8064_EF48S) || defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define FG_SDA	0
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define FG_SDA	37
#endif
#define FG_SCL	1

static struct gpiomux_setting output_pu_cfg = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
};

static struct msm_gpiomux_config max17058_gpio_configs[] = {
	{
		.gpio = FG_SDA,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pu_cfg,
		},
	},
	{
		.gpio = FG_SCL,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pu_cfg,
		},
	},
};

static struct i2c_client *max17058_client;

static int max17058_write_reg(struct i2c_client *client, u8 reg, u16 val)
{
	int err;
	struct i2c_msg msg[1];
	unsigned char data[3];

	if (!client->adapter)
		return -ENODEV;

	msg->addr = client->addr;
	msg->flags = 0;
	msg->len = 3;
	msg->buf = data;
	data[0] = reg;
	data[1] = (val>>8);
	data[2] = (val&0xFF);

	err = i2c_transfer(client->adapter, msg, 1);
	if (err >= 0)
		return 0;
	return err;
}

//20120830 ugot2do. 
static int max17058_read_reg(struct i2c_client *client, u8 reg)
{
	int err;
	struct i2c_msg msg[2];
	u8 reg_buf[] = { reg };
	u8 data_buf[2];

	if (!client->adapter)
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = reg_buf;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = data_buf;

	err = i2c_transfer(client->adapter, msg, 2);

	if (err < 0) {
		pr_err("%s: I2C err: %d\n", __func__, err);
		return err;
	}

	return ((data_buf[0]<<8)|data_buf[1]);
}

static int max17058_get_version(void)
{
	if(!max17058_client)
		return -EIO;

	return max17058_read_reg(max17058_client, MAX17058_VER_MSB);
}

void max17058_set_rcomp(u8 msb, u8 lsb)
{	
	u16 data;
	
	if(!max17058_client)
		return;

	data = (msb<<8)|lsb;
	max17058_write_reg(max17058_client, MAX17058_RCOMP_MSB, data);
}

int max17058_calc_rcomp(unsigned int batt_temp)
{
	int newRcomp;
//	int tempCoHot, tempCoCold;

//	tempCoHot = 125;	// -0.125
//	tempCoCold = -3425;	// -3.425

	if(batt_temp > 20) {
		newRcomp = SKY_DEFAULT_RCOMP - (((batt_temp-20) * SKY_TEMP_CO_HOT)/1000);
		}
	else if(batt_temp < 20) {
		newRcomp = SKY_DEFAULT_RCOMP + (((batt_temp-20) * SKY_TEMP_CO_COLD)/1000);
		}
	else
		newRcomp = SKY_DEFAULT_RCOMP;

	if(newRcomp > 255)
		newRcomp = 255;
	else if(newRcomp < 0)
		newRcomp = 0;

	return newRcomp;
}

int max17058_get_vcell(void)
{
	u8 msb;
	u8 lsb;
	int data;
	int uvolt=0;
	int voltage=0;

	if(!max17058_client)
		return -EIO;
	
	data = max17058_read_reg(max17058_client, MAX17058_VCELL_MSB);
	if(data < 0)
		return uvolt;
	
	msb = (data>>8);
	lsb = (data&0xFF);
	
	voltage=(msb<<4)|((lsb&0xf0)>>4);
	uvolt=(voltage*1250);

	return uvolt;
}

int max17058_get_soc(void)
{
	u8 msb;
	u8 lsb;
	int data;
	int adj_soc=0;
	int soc=0;
	int retry=5;

	if(!max17058_client)
		return -EIO;
	
	do {
	data = max17058_read_reg(max17058_client, MAX17058_SOC_MSB);
	}while(data < 0 && retry-- > 0);
	
	if(data < 0)
		return 1;	// if i2c failed, return 1%
	
	msb = (data>>8);
	lsb = (data&0xFF);

	soc = SKY_MULT_1000(((msb * 256) +lsb)) / 512;
	adj_soc = (soc-SKY_SOC_EMPTY)*100 / (SKY_SOC_FULL-SKY_SOC_EMPTY);

#if 0
	if(adj_soc < 0)
		adj_soc = 0;

	if(adj_soc >= 100)
		adj_soc = 100;
#endif	
	return adj_soc;
}

static int __devinit max17058_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);

	msm_gpiomux_install(max17058_gpio_configs, ARRAY_SIZE(max17058_gpio_configs));
	
	if (!i2c_check_functionality(adapter, I2C_FUNC_I2C))
		return -EIO;	

	max17058_client = client;

	printk("### MAX17058 Ver:%d ###\n", max17058_get_version());
	
	return 0;
}

static int __devexit max17058_remove(struct i2c_client *client)
{
	max17058_client = NULL;
	return 0;
}

static int max17058_suspend(struct device *dev)
{
	return 0;
}

static int max17058_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops max17058_pm_ops = {
	.suspend		= max17058_suspend,
	.resume		= max17058_resume,
};

static const struct i2c_device_id max17058_id[] = {
	{ "max17058-i2c", 0 },
};

static struct i2c_driver max17058_i2c_driver = {
	.driver	= {
		.name	= "max17058-i2c",
		.pm	= &max17058_pm_ops,
	},
	.probe		= max17058_probe,
	.remove		= __devexit_p(max17058_remove),
	.id_table	= max17058_id,
};

static int __init max17058_init(void)
{
	int rc;
	
	rc = i2c_add_driver(&max17058_i2c_driver);
	if(rc) {
		printk(KERN_ERR "max17058_battery driver add failed.\n");
	}
	return rc;
}

static void __exit max17058_exit(void)
{
	i2c_del_driver(&max17058_i2c_driver);
}

module_init(max17058_init);
module_exit(max17058_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MAX17058 battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" "max17058_battery");

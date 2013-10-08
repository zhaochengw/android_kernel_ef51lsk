#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/mfd/pm8xxx/smb347_charger.h>
#include <linux/mfd/pm8xxx/pm8921-charger.h>
#include <linux/mfd/pm8xxx/pm8921-bms.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <linux/mfd/pm8xxx/ccadc.h>
#include <linux/mfd/pm8xxx/core.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/workqueue.h>
#include <linux/debugfs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/usb/composite.h>
#include <mach/msm_xo.h>
#include <mach/msm_hsusb.h>
#include <mach/gpiomux.h>
//20120830 ugot2do. changed.
//#include <linux/gpio.h>
#include <mach/gpio-v1.h>
#include <mach/restart.h>

//20120830 ugot2do. compile err
#define PANTECH_HW_REV_TEST
#define PANTECH_CHARGER_TEST_MENU
//#define DEBUG_TEMP_COMPENSATION_ALGO
//#define PANTECH_BATT_TEMP_COMPENSATION

#if defined(PANTECH_HW_REV_TEST)
#include <linux/proc_fs.h>
#endif
#if defined(PANTECH_CHARGER_TEST_MENU)
#include <linux/input.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#define CHARER_IOCTL_MAGIC 'p'
#define CHARGER_MONITOR_TEST_SET_SMB	_IOR(CHARER_IOCTL_MAGIC, 1, int[3])//#4648 charging
#define CHARGER_DISCHARGING_TEST_SET_SMB _IOWR(CHARER_IOCTL_MAGIC, 2, int)//#8378522 Charging/Discharging Test
#define CHARGER_DISCHARGING_TEST_GET_SMB _IOR(CHARER_IOCTL_MAGIC, 3, int)//#8378522 Charging/Discharging Test
#define CHARGER_CHARGING_TEST_SET_SMB _IOR(CHARER_IOCTL_MAGIC, 4, int[18])//#8378522 Charging/Discharging Test

#endif

#include <linux/mfd/pm8xxx/max17058_battery.h>

#if 1 // P11220_pmic_irq_handler
#include <linux/reboot.h>
#include <mach/system.h>
#endif

#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#include <linux/switch.h>

struct switch_dev *docking_speaker_sdev;
extern void docking_enable_gpio(int enable);
#endif
#endif

#define PM8921_GPIO_BASE		NR_GPIO_IRQS
#define PM8921_GPIO_PM_TO_SYS(pm_gpio)	(pm_gpio - 1 + PM8921_GPIO_BASE)
#define PM8921_MPP_BASE			(PM8921_GPIO_BASE + PM8921_NR_GPIOS)
#define PM8921_MPP_PM_TO_SYS(pm_mpp)	(pm_mpp - 1 + PM8921_MPP_BASE)
#define PM8921_IRQ_BASE			(NR_MSM_IRQS + NR_GPIO_IRQS)

#define PBL_ACCESS1		0x04
#define PBL_ACCESS2		0x05
#define SYS_CONFIG_1		0x06
#define SYS_CONFIG_2		0x07
#define CHG_CNTRL		0x204
#define CHG_IBAT_MAX		0x205
#define CHG_TEST		0x206
#define CHG_BUCK_CTRL_TEST1	0x207
#define CHG_BUCK_CTRL_TEST2	0x208
#define CHG_BUCK_CTRL_TEST3	0x209
#define COMPARATOR_OVERRIDE	0x20A
#define PSI_TXRX_SAMPLE_DATA_0	0x20B
#define PSI_TXRX_SAMPLE_DATA_1	0x20C
#define PSI_TXRX_SAMPLE_DATA_2	0x20D
#define PSI_TXRX_SAMPLE_DATA_3	0x20E
#define PSI_CONFIG_STATUS	0x20F
#define CHG_IBAT_SAFE		0x210
#define CHG_ITRICKLE		0x211
#define CHG_CNTRL_2		0x212
#define CHG_VBAT_DET		0x213
#define CHG_VTRICKLE		0x214
#define CHG_ITERM		0x215
#define CHG_CNTRL_3		0x216
#define CHG_VIN_MIN		0x217
#define CHG_TWDOG		0x218
#define CHG_TTRKL_MAX		0x219
#define CHG_TEMP_THRESH		0x21A
#define CHG_TCHG_MAX		0x21B
#define USB_OVP_CONTROL		0x21C
#define DC_OVP_CONTROL		0x21D
#define USB_OVP_TEST		0x21E
#define DC_OVP_TEST		0x21F
#define CHG_VDD_MAX		0x220
#define CHG_VDD_SAFE		0x221
#define CHG_VBAT_BOOT_THRESH	0x222
#define USB_OVP_TRIM		0x355
#define BUCK_CONTROL_TRIM1	0x356
#define BUCK_CONTROL_TRIM2	0x357
#define BUCK_CONTROL_TRIM3	0x358
#define BUCK_CONTROL_TRIM4	0x359
#define CHG_DEFAULTS_TRIM	0x35A
#define CHG_ITRIM		0x35B
#define CHG_TTRIM		0x35C
#define CHG_COMP_OVR		0x20A

enum pmic_chg_interrupts {
	USBIN_VALID_IRQ = 0,
	USBIN_OV_IRQ,
	BATT_INSERTED_IRQ,
	VBATDET_LOW_IRQ,
	USBIN_UV_IRQ,
	VBAT_OV_IRQ,
	CHGWDOG_IRQ,
	VCP_IRQ,
	ATCDONE_IRQ,
	ATCFAIL_IRQ,
	CHGDONE_IRQ,
	CHGFAIL_IRQ,
	CHGSTATE_IRQ,
	LOOP_CHANGE_IRQ,
	FASTCHG_IRQ,
	TRKLCHG_IRQ,
	BATT_REMOVED_IRQ,
	BATTTEMP_HOT_IRQ,
	CHGHOT_IRQ,
	BATTTEMP_COLD_IRQ,
	CHG_GONE_IRQ,
	BAT_TEMP_OK_IRQ,
	COARSE_DET_LOW_IRQ,
	VDD_LOOP_IRQ,
	VREG_OV_IRQ,
	VBATDET_IRQ,
	BATFET_IRQ,
	PSI_IRQ,
	DCIN_VALID_IRQ,
	DCIN_OV_IRQ,
	DCIN_UV_IRQ,
	PM_CHG_MAX_INTS,
};

#if defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define AUTO_RECHARGE_THRESHOLD_SOC		100	
#else
#define AUTO_RECHARGE_THRESHOLD_SOC		102	
#endif

struct bms_notify {
	int			is_battery_full;
	int			is_charging;
	struct	work_struct	work;
};

#ifdef PANTECH_HW_REV_TEST
struct proc_dir_entry *pm8921_charger_dir;
#endif /* PANTECH_HW_REV_TEST */

static u8 smb347_cmd_regs[16];
static u8 smb347_status_regs[16];
/**
 * struct smb347_chg_chip -device information
 * @dev:			device pointer to access the parent
 * @usb_present:		present status of usb
 * @dc_present:			present status of dc
 * @usb_charger_current:	usb current to charge the battery with used when
 *				the usb path is enabled or charging is resumed
 * @update_time:		how frequently the userland needs to be updated
 * @max_voltage_mv:		the max volts the batt should be charged up to
 * @min_voltage_mv:		the min battery voltage before turning the FETon
 * @cool_temp_dc:		the cool temp threshold in deciCelcius
 * @warm_temp_dc:		the warm temp threshold in deciCelcius
 * @resume_voltage_delta:	the voltage delta from vdd max at which the
 *				battery should resume charging
 * @term_current:		The charging based term current
 *
 */
struct smb347_chg_chip {
	struct device		*dev;
	struct i2c_client 		*smb347_client;
	unsigned int			usb_present;
	unsigned int			dc_present;
	unsigned int 			batt_present;
	unsigned int 			factory_cable_present;
	unsigned int			usb_charger_current;
	unsigned int			max_bat_chg_current;
	unsigned int			pmic_chg_irq[PM_CHG_MAX_INTS];
	unsigned int			wakeup_irq;
	unsigned int			ttrkl_time;
	unsigned int			update_time;
	unsigned int			max_voltage_mv;
	unsigned int			min_voltage_mv;
	int					cool_temp_dc;
	int					warm_temp_dc;
	int					charge_output_voltage;
	int					cable_adc;
	unsigned int			temp_check_period;
	unsigned int			cool_bat_chg_current;
	unsigned int			warm_bat_chg_current;
	unsigned int			cool_bat_voltage;
	unsigned int			warm_bat_voltage;
	unsigned int			is_bat_cool;
	unsigned int			is_bat_warm;
	unsigned int			resume_voltage_delta;
	unsigned int			term_current;
	unsigned int			vbat_channel;
	unsigned int			batt_temp_channel;
	unsigned int			batt_id_channel;
	unsigned int			batt_id;
	int					batt_soc;
	unsigned int			batt_vcell;
	unsigned int 			batt_status;
	int 					batt_temp;
	unsigned int			rcomp;
	unsigned int			charge_type;
	enum pantech_cable_type 	pantech_cable;
	enum battery_thermal_trip_type therm_type;
	struct power_supply		usb_psy;
	struct power_supply		dc_psy;
	struct power_supply		*ext_psy;
	struct power_supply		batt_psy;
	struct dentry			*dent;
	struct bms_notify		bms_notify;
	bool				keep_btm_on_suspend;
	bool				ext_charging;
	bool				ext_charge_done;
	DECLARE_BITMAP(enabled_irqs, PM_CHG_MAX_INTS);
	struct work_struct		battery_id_valid_work;
	int64_t				batt_id_min;
	int64_t				batt_id_max;
	int				trkl_voltage;
	int				weak_voltage;
	int				trkl_current;
	int				weak_current;
	int				vin_min;
	unsigned int			*thermal_mitigation;
	int				thermal_levels;
	struct delayed_work		update_heartbeat_work;
#ifdef USE_USBPHY_CABLE_DETECTION	
	struct delayed_work		update_cable_work;
#endif
	struct delayed_work		batt_check_work;
	struct delayed_work		cable_recheck_work;
//	struct delayed_work		eoc_work;
//	struct delayed_work		unplug_wrkarnd_restore_work;
//	struct delayed_work		unplug_check_work;
//	struct wake_lock		unplug_wrkarnd_restore_wake_lock;
	struct wake_lock		heartbeat_wake_lock;
	struct wake_lock		eoc_wake_lock;
	enum pm8921_chg_cold_thr	cold_thr;
	enum pm8921_chg_hot_thr		hot_thr;
};

static struct smb347_chg_chip *the_chip;

#if 0
static struct pm8xxx_adc_arb_btm_param btm_config;
#endif

#ifdef USE_USBPHY_CABLE_DETECTION
#define FACT_CABLE_MIN	1200000
#define FACT_CABLE_MAX 1400000
#define DOCK_STATION_MIN 1450000
#define DOCK_STATION_MAX 1600000
static unsigned int chg_usb_type = USB_INVALID_CHARGER;
#endif

#ifdef CONFIG_PANTECH_ANDROID_OTG
bool get_pmic_status(void);
#endif

/* SMB347 gpio config */
#define GPIO_HIGH_VALUE 1
#define GPIO_LOW_VALUE  0

//20120810 so2firethread. changed.
#if defined(CONFIG_MACH_APQ8064_EF48S) || defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define SC_INOK	77
#define SC_STAT 	7
#define SC_USB5_1_HC	14
#define SC_EN	15
#define SC_SUSP	23
#define SC_SDA	26
#define SC_SCL	69
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define SC_INOK	77
#define SC_STAT	7
#define SC_USB5_1_HC 69
#define SC_EN	12
#define SC_SUSP	23
#define SC_SDA	53
#define SC_SCL	54
#endif

static struct gpiomux_setting input_pu_cfg = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
		.dir = GPIOMUX_IN,
};

#if 0
static struct gpiomux_setting input_pd_cfg = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
		.dir = GPIOMUX_IN,
};
#endif

static struct gpiomux_setting output_pu_cfg = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_UP,
};

static struct gpiomux_setting output_pd_cfg = {
		.func = GPIOMUX_FUNC_GPIO,
		.drv = GPIOMUX_DRV_2MA,
		.pull = GPIOMUX_PULL_DOWN,
};

static struct msm_gpiomux_config smb347_gpio_configs[] = {
	{
		.gpio = SC_STAT,
		.settings = {
			[GPIOMUX_ACTIVE]    = &input_pu_cfg,
			[GPIOMUX_SUSPENDED] = &input_pu_cfg,
		},
	},
	{
		.gpio = SC_USB5_1_HC,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pd_cfg,
		},
	},
	{
		.gpio = SC_EN,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pd_cfg,
		},
	},
	{
		.gpio = SC_INOK,
		.settings = {
			[GPIOMUX_ACTIVE]    = &input_pu_cfg,
			[GPIOMUX_SUSPENDED] = &input_pu_cfg,
		},
	},
	{
		.gpio = SC_SDA,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pu_cfg,
		},
	},
	{
		.gpio = SC_SCL,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pu_cfg,
		},
	},	
#if 0	// remove 
	{
		.gpio = SC_SUSP,
		.settings = {
			[GPIOMUX_ACTIVE]    = &output_pu_cfg,
			[GPIOMUX_SUSPENDED] = &output_pu_cfg,
		},
	},	
#endif
};

static enum power_supply_property pm_power_props[] = {
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
};

static char *pm_power_supplied_to[] = {
	"battery",
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
//	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
//	POWER_SUPPLY_PROP_ENERGY_FULL,
};

#if defined(PANTECH_CHARGER_TEST_MENU)
static struct input_dev *bms_input_dev;
static atomic_t bms_input_flag;
#endif

#if 1 // P11220_pmic_irq_handler
static irqreturn_t batt_removed_irq_handler(int irq, void *data);

struct pm_chg_irq_init_data {
	unsigned int	irq_id;
	char		*name;
	unsigned long	flags;
	irqreturn_t	(*handler)(int, void *);
};


#define PMIC_IRQ(_id, _flags, _handler) \
{ \
	.irq_id		= _id, \
	.name		= #_id, \
	.flags		= _flags, \
	.handler	= _handler, \
}

struct pm_chg_irq_init_data pmic_chg_irq_data[] = {
	PMIC_IRQ(USBIN_VALID_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 	NULL),
	PMIC_IRQ(USBIN_OV_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(BATT_INSERTED_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(VBATDET_LOW_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 	NULL),
	PMIC_IRQ(USBIN_UV_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(VBAT_OV_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(CHGWDOG_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(VCP_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(ATCDONE_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(ATCFAIL_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(CHGDONE_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(CHGFAIL_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(CHGSTATE_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(LOOP_CHANGE_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(FASTCHG_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(TRKLCHG_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(BATT_REMOVED_IRQ, IRQF_TRIGGER_RISING, batt_removed_irq_handler),
	PMIC_IRQ(BATTTEMP_HOT_IRQ, IRQF_TRIGGER_RISING, 	NULL),
	PMIC_IRQ(CHGHOT_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(BATTTEMP_COLD_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(CHG_GONE_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(BAT_TEMP_OK_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(COARSE_DET_LOW_IRQ, IRQF_TRIGGER_RISING, 	NULL),
	PMIC_IRQ(VDD_LOOP_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(VREG_OV_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(VBATDET_IRQ, IRQF_TRIGGER_RISING, NULL),
	PMIC_IRQ(BATFET_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(DCIN_VALID_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(DCIN_OV_IRQ, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, NULL),
	PMIC_IRQ(DCIN_UV_IRQ, IRQF_TRIGGER_RISING, NULL),
};

static int pm_chg_get_rt_status(struct smb347_chg_chip *chip, int irq_id)
{
	return pm8xxx_read_irq_stat(chip->dev->parent,
					chip->pmic_chg_irq[irq_id]);
}

static void pm_chg_disable_irq(struct smb347_chg_chip *chip, int interrupt)
{
	if (__test_and_clear_bit(interrupt, chip->enabled_irqs)) {
		dev_dbg(chip->dev, "%d\n", chip->pmic_chg_irq[interrupt]);
		disable_irq_nosync(chip->pmic_chg_irq[interrupt]);
	}
}

static void pm_chg_enable_irq(struct smb347_chg_chip *chip, int interrupt)
{
	if (!__test_and_set_bit(interrupt, chip->enabled_irqs)) {
		dev_dbg(chip->dev, "%d\n", chip->pmic_chg_irq[interrupt]);
		enable_irq(chip->pmic_chg_irq[interrupt]);
	}
}

static void free_irqs(struct smb347_chg_chip *chip)
{
		if (chip->pmic_chg_irq[BATT_REMOVED_IRQ]) {
			free_irq(chip->pmic_chg_irq[BATT_REMOVED_IRQ], chip);
			chip->pmic_chg_irq[BATT_REMOVED_IRQ] = 0;
  }
}

static int __devinit request_irqs(struct smb347_chg_chip *chip, struct platform_device *pdev)
{
	struct resource *res;
	int ret;

	ret = 0;
	bitmap_fill(chip->enabled_irqs, PM_CHG_MAX_INTS);

	res = platform_get_resource_byname(pdev, IORESOURCE_IRQ, pmic_chg_irq_data[BATT_REMOVED_IRQ].name);
	if (res == NULL) {
		pr_err("couldn't find %s\n", pmic_chg_irq_data[BATT_REMOVED_IRQ].name);
		goto err_out;
	}

	chip->pmic_chg_irq[pmic_chg_irq_data[BATT_REMOVED_IRQ].irq_id] = res->start;
	ret = request_irq(res->start, pmic_chg_irq_data[BATT_REMOVED_IRQ].handler,
	              pmic_chg_irq_data[BATT_REMOVED_IRQ].flags, pmic_chg_irq_data[BATT_REMOVED_IRQ].name, chip);

	if (ret < 0) {
		pr_err("couldn't request %d (%s) %d\n", res->start, pmic_chg_irq_data[BATT_REMOVED_IRQ].name, ret);
		chip->pmic_chg_irq[pmic_chg_irq_data[BATT_REMOVED_IRQ].irq_id] = 0;
		goto err_out;
	}

	pm_chg_disable_irq(chip, pmic_chg_irq_data[BATT_REMOVED_IRQ].irq_id);
	return 0;

err_out:
	free_irqs(chip);
	return -EINVAL;
}
#endif

int get_cable_id_adc_value(void)
{
	struct pm8xxx_adc_chan_result result;
	int rc, try_max = 0;

	do{
		rc = pm8xxx_adc_mpp_config_read(3, ADC_MPP_1_AMUX6, &result);
		if(rc == -EINVAL)
			return -EINVAL;
		try_max++;
	}while(rc && (try_max < 20));

	if(!rc){
		return result.physical;
	}else{
		return 0;
	}
}
EXPORT_SYMBOL_GPL(get_cable_id_adc_value);

int get_hw_rev_adc_value(void)
{
	struct pm8xxx_adc_chan_result result;
	int rc, try_max = 0;

	do{
		rc = pm8xxx_adc_mpp_config_read(4, ADC_MPP_1_AMUX6, &result);
		if(rc == -EINVAL)
			return -EINVAL;
		try_max++;
	}while(rc && (try_max < 20));

	if(!rc){
		return result.physical;
	}else{
		return 0;
	}
}
EXPORT_SYMBOL_GPL(get_hw_rev_adc_value);

static int smb347_gpio_init(void)
{
	int rc=0;

	msm_gpiomux_install(smb347_gpio_configs, ARRAY_SIZE(smb347_gpio_configs));

	rc = gpio_request(SC_INOK, "sc_inok");
	if (rc) {
		pr_err("request gpio SC_INOK failed, rc=%d\n", rc);
	}


#if defined(CONFIG_MACH_APQ8064_EF50L)
	rc = gpio_request(SC_SUSP, "sc_susp");

	if(rc) {
		pr_err("request gpio SC_SUSP failed, rc=%d\n", rc);
	}

	gpio_direction_output(SC_SUSP, GPIO_HIGH_VALUE);  
#endif
#if 0	
	gpio_request(SC_SUSP, "sc_susp");
	if (rc) {
		printk("request gpio SC_SUSP failed, rc=%d\n", rc);
	}

	rc = gpio_get_value_cansleep(SC_SUSP);
	if(!rc) {
		printk("SMB347 suspended\n");
		
		/* wake up smb347 */
		gpio_direction_output(SC_SUSP, GPIO_HIGH_VALUE);  
		msleep(200);
	}
#endif	 
	return rc;
}

static int smb347_read_reg(u8 reg, unsigned char *val)
{
	s32 ret;

	if(!the_chip->smb347_client)
		return -EIO;
	
	ret = i2c_smbus_read_byte_data(the_chip->smb347_client, reg);
	if (ret < 0) {
		pr_err("smb347 i2c read fail: can't read from %02x: %d\n", reg, ret);
		return ret;
	} else
		*val = ret;

	return 0;
}

static int smb347_write_reg(u8 reg, unsigned char val)
{
	s32 ret;

	if(!the_chip->smb347_client)
		return -EIO;
	
	ret = i2c_smbus_write_byte_data(the_chip->smb347_client, reg, val);
	if (ret < 0) {
		pr_err("smb347 i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, ret);
		return ret;
	}
	return 0;
}

/* Treat OverVoltage/UnderVoltage as source missing */
static int is_chg_plugged_in(struct smb347_chg_chip *chip)
{
#if 0
	unsigned char rdData=0;
	
	smb347_read_reg(0x08, &rdData);
	if(rdData&0x1)
		return gpio_get_value_cansleep(SC_INOK);
	else
		return !gpio_get_value_cansleep(SC_INOK);
#else
	return !gpio_get_value_cansleep(SC_INOK);
#endif
}

// [LCD], kkcho, 20120919, : #CONFIG_F_SKYDISP_LCD_SHUTDOWN
int is_chg_notify_lcd(void)
{
	int state;
	state = is_chg_plugged_in(the_chip);
	return state;
}

static int pm_chg_masked_write(struct smb347_chg_chip *chip, u16 addr,
							u8 mask, u8 val)
{
	int rc;
	u8 reg;

	rc = pm8xxx_readb(chip->dev->parent, addr, &reg);
	if (rc) {
		pr_err("pm8xxx_readb failed: addr=%03X, rc=%d\n", addr, rc);
		return rc;
	}
	reg &= ~mask;
	reg |= val & mask;
	rc = pm8xxx_writeb(chip->dev->parent, addr, reg);
	if (rc) {
		pr_err("pm8xxx_writeb failed: addr=%03X, rc=%d\n", addr, rc);
		return rc;
	}
	return 0;
}

void smb347_regs_init(void)
{
	smb347_write_reg(REG_CMD_REG_A, DEFAULT_CMD_REG_A);
	smb347_write_reg(REG_CMD_REG_B, HC_MODE);
	smb347_write_reg(REG_CHG_CURRENT, DEFAULT_CHG_CURRENT);
	smb347_write_reg(REG_INPUT_CURRENT_LIMIT, DEFAULT_INPUT_CURRENT_LIMIT);
	smb347_write_reg(REG_VARIOUS_FUNCTIONS, DEFAULT_VARIOUS_FUNCTIONS);
	smb347_write_reg(REG_FLOAT_VOLTAGE, DEFAULT_FLOAT_VOLTAGE);
	smb347_write_reg(REG_CHG_CTRL, DEFAULT_CHG_CTRL);
	smb347_write_reg(REG_STAT_TIMERS_CTRL, DEFAULT_STAT_TIMERS_CTRL);
	smb347_write_reg(REG_PIN_ENABLE_CTRL, DEFAULTG_PIN_ENABLE_CTRL);
	smb347_write_reg(REG_SYSTEM_CTRL, DEFAULT_SYSTEM_CTRL);
	smb347_write_reg(REG_SYSOK_USB_SEL, DEFAULT_SYSOK_USB_SEL);
	smb347_write_reg(REG_OTHER_CTRL, DEFAULT_OTHER_CTRL);
	smb347_write_reg(REG_OTG_TLIM_THERM_CTRL, DEFAULT_OTG_TLIM_THERM_CTRL);
	smb347_write_reg(REG_TRIP_POINT, DEFAULT_TRIP_POINT);
	smb347_write_reg(REG_FAULT_INTERRUPT, DEFAULT_FAULT_INTERRUPT);
	smb347_write_reg(REG_STAT_INTERRUPT, DEFAULT_STAT_INTERRUPT);	
}

void print_smb347_regs(void)
{
	int i;

	printk("====== SMB347 Reg =====\n");
	for(i=0; i<15; i++) 
		printk("[R%02X] : 0x%02X\n", i, smb347_cmd_regs[i]);

	for(i=0; i<15; i++) 
		printk("[R%02X] : 0x%02X\n", (REG_CMD_REG_A+i), smb347_status_regs[i]);
	
	printk("==============================\n");
}

void smb347_dump_regs(void)
{
	int i;

	// dump command registers
	for(i=0; i<16; i++)
		smb347_read_reg((REG_CHG_CURRENT+i), &smb347_cmd_regs[i]);

	// dump status registers
	for(i=0; i<16; i++) 
		smb347_read_reg((REG_CMD_REG_A+i), &smb347_status_regs[i]);

	print_smb347_regs();
}

static void (*notify_vbus_state_func_ptr)(int);
static int usb_chg_current;
static DEFINE_SPINLOCK(vbus_lock);

int smb347_charger_register_vbus_sn(void (*callback)(int))
{
	pr_debug("%p\n", callback);
	notify_vbus_state_func_ptr = callback;
	return 0;
}
EXPORT_SYMBOL_GPL(smb347_charger_register_vbus_sn);

/* this is passed to the hsusb via platform_data msm_otg_pdata */
void smb347_charger_unregister_vbus_sn(void (*callback)(int))
{
	pr_debug("%p\n", callback);
	notify_vbus_state_func_ptr = NULL;
}
EXPORT_SYMBOL_GPL(smb347_charger_unregister_vbus_sn);

int smb347_set_usb_power_supply_type(enum power_supply_type type)
{
	pr_err("%s: type:%d\n", __func__, type);
	if (!the_chip) {
		pr_err("called before init\n");
		return -EINVAL;
	}

	if (type < POWER_SUPPLY_TYPE_USB)
		return -EINVAL;

//	the_chip->usb_psy.type = type;
	return 0;
}
EXPORT_SYMBOL_GPL(smb347_set_usb_power_supply_type);

#ifdef USE_USBPHY_CABLE_DETECTION
static void __smb347_charger_vbus_draw(unsigned int mA)
{
	if (!the_chip)
		return;

	usb_chg_current = mA;
	__cancel_delayed_work(&the_chip->update_cable_work);
	schedule_delayed_work(&the_chip->update_cable_work, 0);
}
#endif

void smb347_charger_vbus_draw(unsigned int mA, unsigned int chg_type)
{
	unsigned long flags;

	pr_err("Enter charge=%d\n", mA);
	
	spin_lock_irqsave(&vbus_lock, flags);
	
#ifdef USE_USBPHY_CABLE_DETECTION
	chg_usb_type = chg_type;
#endif
	
	if (the_chip) {
#ifdef USE_USBPHY_CABLE_DETECTION		
		__smb347_charger_vbus_draw(mA);
#endif
	} else {
		/*
		 * called before pmic initialized,
		 * save this value and use it at probe
		 */
		usb_chg_current = mA;
	}
	spin_unlock_irqrestore(&vbus_lock, flags);
}
EXPORT_SYMBOL_GPL(smb347_charger_vbus_draw);

static void notify_usb_of_the_plugin_event(int plugin)
{
	
	plugin = !!plugin;
	if (notify_vbus_state_func_ptr) {
		pr_err("notifying plugin\n");
		(*notify_vbus_state_func_ptr) (plugin);
	} else {
		pr_err("unable to notify plugin\n");
	}
}


static int smb347_subdevices_register(void)
{
	int rc;

#if defined(PANTECH_CHARGER_TEST_MENU)
	atomic_set(&bms_input_flag, 0);
	

	bms_input_dev = input_allocate_device();
	if (!bms_input_dev) {
	    	pr_err("BMS: Unable to input_allocate_device \n");
	    	rc = -ENXIO;
	    	return rc;
	}

	set_bit(EV_REL, bms_input_dev->evbit);
	input_set_capability(bms_input_dev, EV_REL, REL_RX);	// SOC
	input_set_capability(bms_input_dev, EV_REL, REL_RY); 	// Volt
	input_set_capability(bms_input_dev, EV_REL, REL_RZ);    // TEMP
	input_set_capability(bms_input_dev, EV_REL, REL_X);	// VCHG
	bms_input_dev->name="bms_app";
	rc =input_register_device(bms_input_dev);
	if (rc) {
	    	pr_err("BMS: Unable to register input_register_device device\n");
	    	return rc;
	}
#endif

#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	docking_speaker_sdev = kzalloc(sizeof(docking_speaker_sdev), GFP_KERNEL);
	docking_speaker_sdev->name = "docking_speaker";

	rc = switch_dev_register(docking_speaker_sdev);
	if (rc)
		pr_err("docking_speaker switch registration failed\n");
	else
		pr_debug("docking_speaker detected\n");
#endif
#endif

	return rc;

}

#ifdef CONFIG_PANTECH_ANDROID_OTG
void smb347_otg_power(int on)
{
	printk("%s Enter on=%d\n", __func__, on);
	
	if (!the_chip)
		return;

	if (on) {
		smb347_regs_init();
		smb347_write_reg(REG_CMD_REG_A, OTG_ENABLE);
		smb347_write_reg(REG_CMD_REG_B, OTG_MODE);
		if(the_chip)
			the_chip->pantech_cable = PANTECH_OTG;
	}
	else {
		smb347_regs_init();
		smb347_write_reg(REG_CMD_REG_A, DEFAULT_CMD_REG_A);
		smb347_write_reg(REG_CMD_REG_B, HC_MODE);
		if(the_chip)
			the_chip->pantech_cable = PANTECH_CABLE_NONE;
	}
}

 void external_pmic_id_status(bool value);
#endif


static int get_pantech_cable_type(struct smb347_chg_chip *chip)
{
	int i;
	int adc_val;
	
	adc_val = get_cable_id_adc_value();

	chip->cable_adc = adc_val;

	for(i=0; i < ARRAY_SIZE(pantech_cable_info); i++) 
	{
		if(adc_val >= pantech_cable_info[i].min_adc && adc_val <= pantech_cable_info[i].max_adc)
			return pantech_cable_info[i].cable;
	}

	return PANTECH_CABLE_MAX;
}

#ifdef USE_SMB347_CABLE_DETECTION	
enum RUNTIME_LEVEL {
	DRIVER_BOOTING=0,
	DRIVER_BOOT_COMPLETED
};
static int get_cable_type(struct smb347_chg_chip *chip, int level)
{
	int rc=0;
	unsigned char rdData=0;
	unsigned char cnt=0;
	unsigned char chg_type;
	
	chg_type = get_pantech_cable_type(chip);

	if(chg_type != PANTECH_CABLE_MAX)
		return chg_type;

	if(level == DRIVER_BOOT_COMPLETED) {
		/* APSD reset */
		smb347_write_reg(REG_CHG_CTRL, DISABLE_APSD);
		msleep(1);
		smb347_write_reg(REG_CHG_CTRL, DEFAULT_CHG_CTRL);
		
		/* check APSD Status */
		do {
			rc = smb347_read_reg(REG_STAT_REG_D, &rdData);
			if(rc < 0) 
				break;
			msleep(10);
		}while((cnt++ < 50) && ((rdData&0x8) == 0));
		
	//		printk("APSD Status:0x%x, cnt:%d\n", rdData, cnt);
	}
	else {
		smb347_read_reg(REG_STAT_REG_D, &rdData);
	}

	/* check charger type */
	chg_type = (rdData&0x7);

	if(chg_type == APSD_SDP)
		return PANTECH_USB;
	else 
		return PANTECH_AC;

	return PANTECH_CABLE_NONE;

}

static void set_power_supply_type(struct smb347_chg_chip *chip, enum pantech_cable_type cable)
{
	if(cable == PANTECH_OTG || cable == PANTECH_CABLE_MAX)
		return;

	switch(cable) {
		case PANTECH_USB:
		case PANTECH_FACTORY:
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
			chip->dc_present = false;
			
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
			chip->usb_present = true;
			break;
		case PANTECH_AC:
		case PANTECH_AUDIO_DOCKING_STATION:
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
			chip->usb_present = false;
			
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
			chip->dc_present = true;
			break;
		case PANTECH_CABLE_NONE:
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
			chip->usb_present = false;
			
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
			chip->dc_present = false;
			break;
		default:
			break;
	}
}
#endif

static int is_batt_status_charging(struct smb347_chg_chip *chip)
{
	unsigned char rdData=0;
	
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	/* Check OTG mode */
	smb347_read_reg(REG_CMD_REG_A, &rdData);
	if( (rdData&(1<<4)) == 1)
		return POWER_SUPPLY_STATUS_DISCHARGING;
	
	if(is_chg_plugged_in(chip)) {
		if(chip->batt_soc >= 100)
			return POWER_SUPPLY_STATUS_FULL;

		if(chip->therm_type == BATT_THERM_FATAL_COLD
			|| chip->therm_type == BATT_THERM_FATAL_HOT) 
			return POWER_SUPPLY_STATUS_NOT_CHARGING;
			
		return POWER_SUPPLY_STATUS_CHARGING;
	}
	else {
		return POWER_SUPPLY_STATUS_DISCHARGING;
	}
	
	return POWER_SUPPLY_STATUS_UNKNOWN;
}

#if 0
#define PM8921_CHG_BATT_TEMP_THR_COLD	BIT(1)
#define PM8921_CHG_BATT_TEMP_THR_COLD_SHIFT	1
static int pm_chg_batt_cold_temp_config(struct smb347_chg_chip *chip,
					enum pm8921_chg_cold_thr cold_thr)
{
	u8 temp;

	temp = cold_thr << PM8921_CHG_BATT_TEMP_THR_COLD_SHIFT;
	temp = temp & PM8921_CHG_BATT_TEMP_THR_COLD;
	return pm_chg_masked_write(chip, CHG_CNTRL_2,
					PM8921_CHG_BATT_TEMP_THR_COLD,
					 temp);
}

#define PM8921_CHG_BATT_TEMP_THR_HOT		BIT(0)
#define PM8921_CHG_BATT_TEMP_THR_HOT_SHIFT	0
static int pm_chg_batt_hot_temp_config(struct smb347_chg_chip *chip,
					enum pm8921_chg_hot_thr hot_thr)
{
	u8 temp;

	temp = hot_thr << PM8921_CHG_BATT_TEMP_THR_HOT_SHIFT;
	temp = temp & PM8921_CHG_BATT_TEMP_THR_HOT;
	return pm_chg_masked_write(chip, CHG_CNTRL_2,
					PM8921_CHG_BATT_TEMP_THR_HOT,
					 temp);
}

static void btm_configure_work(struct work_struct *work)
{
	int rc;

	rc = pm8xxx_adc_btm_configure(&btm_config);
	if (rc)
		pr_err("failed to configure btm rc=%d", rc);
}
DECLARE_WORK(btm_config_work, btm_configure_work);

#define TEMP_HYSTERISIS_DEGC 2
static void battery_cool(bool enter)
{
	printk("%s: enter = %d\n", __func__, enter);
	if (enter == the_chip->is_bat_cool)
		return;
	the_chip->is_bat_cool = enter;
	if (enter) {
		btm_config.low_thr_temp =
			the_chip->cool_temp_dc + TEMP_HYSTERISIS_DEGC;
		if(the_chip->dc_present) {
#if defined(CONFIG_MACH_APQ8064_EF50L)			
			smb347_write_reg(0x00,0x1B);
#else
			smb347_write_reg(0x00,0x1A);
#endif
			smb347_write_reg(0x03,0x59);
			smb347_write_reg(0x30,0xC2);
			smb347_write_reg(0x31,0x01);
		}
	} else {
		btm_config.low_thr_temp = the_chip->cool_temp_dc;
		if(the_chip->dc_present) {
#if defined(CONFIG_MACH_APQ8064_EF50L)
	smb347_write_reg(0x00,0xDB);
#else
	smb347_write_reg(0x00,0xDA);
#endif			
			smb347_write_reg(0x03,0x6A);
			smb347_write_reg(0x30,0xC2);
			smb347_write_reg(0x31,0x01);
	}
	}
	schedule_work(&btm_config_work);
}

static void battery_warm(bool enter)
{
	printk("%s: enter = %d\n", __func__, enter);
	if (enter == the_chip->is_bat_warm)
		return;
	the_chip->is_bat_warm = enter;
	if (enter) {
		btm_config.high_thr_temp =
			the_chip->warm_temp_dc - TEMP_HYSTERISIS_DEGC;
		if(the_chip->dc_present) {
#if defined(CONFIG_MACH_APQ8064_EF50L)			
			smb347_write_reg(0x00,0x1B);
#else
			smb347_write_reg(0x00,0x1A);
#endif			
			smb347_write_reg(0x03,0x59);
			smb347_write_reg(0x30,0xC2);
			smb347_write_reg(0x31,0x01);
		}
	
	} else {
		btm_config.high_thr_temp = the_chip->warm_temp_dc;
		if(the_chip->dc_present) {
#if defined(CONFIG_MACH_APQ8064_EF50L)
	smb347_write_reg(0x00,0xDB);
#else
	smb347_write_reg(0x00,0xDA);
#endif			
			smb347_write_reg(0x03,0x6A);
			smb347_write_reg(0x30,0xC2);
			smb347_write_reg(0x31,0x01);
	}
	}
	schedule_work(&btm_config_work);
}

static int configure_btm(struct smb347_chg_chip *chip)
{
	int rc;

	if (chip->warm_temp_dc != INT_MIN)
	btm_config.btm_warm_fn = battery_warm;
	else
		btm_config.btm_warm_fn = NULL;

	if (chip->cool_temp_dc != INT_MIN)
	btm_config.btm_cool_fn = battery_cool;
	else
		btm_config.btm_cool_fn = NULL;

	btm_config.low_thr_temp = chip->cool_temp_dc;
	btm_config.high_thr_temp = chip->warm_temp_dc;
	btm_config.interval = chip->temp_check_period;
	rc = pm8xxx_adc_btm_configure(&btm_config);
	if (rc)
		pr_err("failed to configure btm rc = %d\n", rc);
	rc = pm8xxx_adc_btm_start();
	if (rc)
		pr_err("failed to start btm rc = %d\n", rc);

	return rc;
}
#endif

static int batt_temp_compensation(struct smb347_chg_chip *chip, int adc)
{
#ifdef PANTECH_BATT_TEMP_COMPENSATION
	int i, offset=0, len;
	const struct temp_adc_info *offset_table;

	if(chip->dc_present) {
		offset_table = dc_chg_offset_table;
		len = ARRAY_SIZE(dc_chg_offset_table);
	}
	else { 	 
		offset_table = dischg_offset_table;
		len = ARRAY_SIZE(dischg_offset_table);
	}

	for(i=0; i<len; i++)
	{
		if(adc <= offset_table[i].min_temp) {
			offset = offset_table[i].offset;
			break;
		}
	}
	
	return (adc+offset);
#else
	return adc;
#endif
}


#define TEMP_TRIP_HYSTERISIS_DEGC		20

static int get_curr_therm_trip(struct smb347_chg_chip *chip, int temp)
{
	int i;

	for(i=0; i < ARRAY_SIZE(batt_therm_table); i++) {
		if(temp < batt_therm_table[i].batt_temp)
			return batt_therm_table[i].therm_trip;
	}

	return BATT_THERM_UNKNOWN;
}

static int battery_temp_trip_changed(struct smb347_chg_chip *chip, int temp)
{
	int curr_trip = get_curr_therm_trip(chip, temp);
	int trip_changed=0;
	
	if(curr_trip == chip->therm_type || curr_trip > BATT_THERM_UNKNOWN)
		return 0;

	switch(chip->therm_type) {
		case BATT_THERM_FATAL_COLD:
			if(temp >= (batt_therm_table[chip->therm_type].batt_temp + TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_CRITICAL_COLD:
			if(temp >= (batt_therm_table[chip->therm_type].batt_temp + TEMP_TRIP_HYSTERISIS_DEGC)
				|| temp < batt_therm_table[chip->therm_type-1].batt_temp) 
				trip_changed = 1;
			
			break;
		case BATT_THERM_NORMAL:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < batt_therm_table[chip->therm_type-1].batt_temp) 
				trip_changed = 1;
			break;
		case BATT_THERM_WARM:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < (batt_therm_table[chip->therm_type-1].batt_temp-TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_CRITICAL_HOT:
			if(temp >= batt_therm_table[chip->therm_type].batt_temp
				|| temp < (batt_therm_table[chip->therm_type-1].batt_temp-TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_FATAL_HOT:
			if(temp < (batt_therm_table[chip->therm_type-1].batt_temp - TEMP_TRIP_HYSTERISIS_DEGC)) 
				trip_changed = 1;
			break;
		case BATT_THERM_UNKNOWN:
			chip->therm_type = curr_trip;
			trip_changed = 1;
			break;
		default:
			return 0;
			break;
	}

	if(trip_changed) {
		printk("batt thermal trip changed: %d -> %d\n", chip->therm_type, curr_trip);
		chip->therm_type = curr_trip;
	}
		
	return trip_changed;
}

#ifdef DEBUG_TEMP_COMPENSATION_ALGO
#define DEBUG_BATT_INIT_TEMP		400		// 40 degC
#define DEBUG_BATT_TEMP_MAX		670		// 67 degC
#define DEBUG_BATT_TEMP_MIN		-110	// -11 degC
#define DEBUG_BATT_TEMP_OFFSET	10
#define DEBUG_UPDATE_TIME	1000	// 1s

static int test_temp=DEBUG_BATT_INIT_TEMP;
static int temp_rising_test=1;
static int batt_temp_algo_test(struct smb347_chg_chip *chip)
{	
	if(test_temp >= DEBUG_BATT_TEMP_MAX)	
		temp_rising_test = 0;
	else if(test_temp <= DEBUG_BATT_TEMP_MIN)	
		temp_rising_test = 1;
	
	if(temp_rising_test)
		test_temp += DEBUG_BATT_TEMP_OFFSET;
	else
		test_temp  -= DEBUG_BATT_TEMP_OFFSET;

	return test_temp;
}
#endif

static int start_auto_recharge(struct smb347_chg_chip *chip, int soc)
{
	if( is_chg_plugged_in(chip) && chip->ext_charge_done 
		&& soc <= AUTO_RECHARGE_THRESHOLD_SOC) {
		printk("Start auto recharge\n");
		return 1;
	}

	return 0;
}

static void set_appropriate_battery_current(struct smb347_chg_chip *chip)
{
	const struct smb347_regs_value *reg_ptr;
	
	if(chip->pantech_cable <= PANTECH_OTG || chip->pantech_cable >= PANTECH_CABLE_MAX
		|| chip->therm_type >= BATT_THERM_UNKNOWN) {
		return;
	}
	
	switch(chip->therm_type) {
		case BATT_THERM_NORMAL:
			reg_ptr = therm_normal_val;
			break;

		case BATT_THERM_WARM:
			reg_ptr = therm_warm_val;
			break;
			
		case BATT_THERM_CRITICAL_COLD:
		case BATT_THERM_CRITICAL_HOT:
			reg_ptr = therm_critical_val;
			break;
			
		case BATT_THERM_FATAL_COLD:
		case BATT_THERM_FATAL_HOT:	
			reg_ptr = therm_fatal_val;
			break;

		default:
			reg_ptr = therm_normal_val;
			break;
	}	

	printk("Cable:%d, therm type:%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n",
		chip->pantech_cable, chip->therm_type, reg_ptr[chip->pantech_cable].charge_current,
		reg_ptr[chip->pantech_cable].input_cur_limit, reg_ptr[chip->pantech_cable].float_voltage,
		reg_ptr[chip->pantech_cable].cmd_reg_a, reg_ptr[chip->pantech_cable].cmd_reg_b);

	smb347_write_reg(REG_CMD_REG_A, DEFAULT_CMD_REG_A);
	smb347_write_reg(REG_CMD_REG_B, DEFAULT_CMD_REG_B);
	smb347_write_reg(REG_CHG_CURRENT, reg_ptr[chip->pantech_cable].charge_current);
	smb347_write_reg(REG_INPUT_CURRENT_LIMIT, reg_ptr[chip->pantech_cable].input_cur_limit);
	smb347_write_reg(REG_FLOAT_VOLTAGE, reg_ptr[chip->pantech_cable].float_voltage);
	smb347_write_reg(REG_CMD_REG_A, reg_ptr[chip->pantech_cable].cmd_reg_a);
	smb347_write_reg(REG_CMD_REG_B, reg_ptr[chip->pantech_cable].cmd_reg_b);

	chip->ext_charge_done = false;
}

#define MAX_TOLERABLE_BATT_TEMP_DDC	680
static int get_batt_temp(struct smb347_chg_chip *chip)
{
#ifdef DEBUG_TEMP_COMPENSATION_ALGO
	return batt_temp_algo_test(chip);
#else
	int rc;
	int i;
	int64_t ave=0;
	
	struct pm8xxx_adc_chan_result result;

	rc = pm8xxx_adc_read(chip->batt_temp_channel, &result);
	if (rc) {
		pr_err("error reading adc channel = %d, rc = %d\n",
					chip->vbat_channel, rc);
		return rc;
	}
	pr_debug("batt_temp phy = %lld meas = 0x%llx\n", result.physical,
						result.measurement);

	/* Workaround for BAT_THERM error */
	if(result.physical == 790)
		result.physical = 300;
	
	if(result.physical >= 640) {
		for(i=0; i<3; i++) {
			rc = pm8xxx_adc_read(chip->batt_temp_channel, &result);
			ave += result.physical;
			msleep(100);
		}

		result.physical = (int) ave/3;
	}
		
	if (result.physical > MAX_TOLERABLE_BATT_TEMP_DDC)
		pr_err("BATT_TEMP= %d > 68degC, device will be shutdown\n",
							(int) result.physical);

	return (int)result.physical;
#endif	
}

#if defined(PANTECH_CHARGER_TEST_MENU)
static int get_batt_chg_current(struct smb347_chg_chip *chip)
{
	struct pm8xxx_adc_chan_result result;
	int rc, try_max = 0;

	do{
		rc = pm8xxx_adc_mpp_config_read(2, ADC_MPP_1_AMUX6, &result);
		if(rc == -EINVAL)
			return -EINVAL;
		try_max++;
	}while(rc && (try_max < 20));

	if(!rc){
		if(is_chg_plugged_in(chip))
			return result.physical;
		else
			return 0;
	}else{
		return 0;
	}

}
#endif

static int get_battery_id(struct smb347_chg_chip *chip)
{
	int rc;
	struct pm8xxx_adc_chan_result result;

	rc = pm8xxx_adc_read(chip->batt_id_channel, &result);
	if (rc) {
		pr_err("error reading batt id channel = %d, rc = %d\n", chip->vbat_channel, rc);
		return rc;
	}
	
	pr_debug("batt_id phy = %lld meas = 0x%llx\n", result.physical, 	result.measurement);
	
	return result.physical;
}

static int get_prop_batt_status(struct smb347_chg_chip *chip)
{
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	if(chip->pantech_cable == PANTECH_FACTORY && !chip->batt_present)
		return POWER_SUPPLY_STATUS_DISCHARGING;
	
	return chip->batt_status;
}

static int get_prop_batt_health(struct smb347_chg_chip *chip)
{
	if(chip->therm_type == BATT_THERM_FATAL_COLD)
		return POWER_SUPPLY_HEALTH_COLD;

	if(chip->therm_type == BATT_THERM_FATAL_HOT)
		return POWER_SUPPLY_HEALTH_OVERHEAT;

	return POWER_SUPPLY_HEALTH_GOOD;
}

static int get_prop_charge_type(struct smb347_chg_chip *chip)
{	
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	return chip->charge_type;
}

static int get_prop_batt_present(struct smb347_chg_chip *chip)
{
	/* Check if called before init */
	if (!chip)
		return -EINVAL;
	if(chip->pantech_cable == PANTECH_FACTORY && !chip->batt_present)
		return 1;
	
	return chip->batt_present;
}

static int get_prop_batt_capacity(struct smb347_chg_chip *chip)
{
	int soc;
	
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	if(chip->pantech_cable == PANTECH_FACTORY && !chip->batt_present)
		return 50;

	soc = chip->batt_soc;
	
	if(soc < 0)
		soc = 0;
	
	if(soc > 100)
		soc = 100;
			
	return soc;
}

static int get_prop_battery_uvolts(struct smb347_chg_chip *chip)
{
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	if(chip->pantech_cable == PANTECH_FACTORY && !chip->batt_present)
		return 4000;
	
	return chip->batt_vcell;
}

static int get_prop_batt_temp(struct smb347_chg_chip *chip)
{
	/* Check if called before init */
	if (!chip)
		return -EINVAL;

	if(chip->pantech_cable == PANTECH_FACTORY && !chip->batt_present)
		return 300;
	
	return chip->batt_temp;
}

#define BATT_ID_ADC_THRESHOLD		600000
static int is_battery_present(void)
{
	int adc;

	adc = get_battery_id(the_chip);
	if(adc  > BATT_ID_ADC_THRESHOLD)
		return 0;
	
	return 1;
}

static void __devinit determine_initial_state(struct smb347_chg_chip *chip)
{
#ifdef USE_SMB347_CABLE_DETECTION	
	
	if(is_chg_plugged_in(chip)) {
		
		wake_lock(&chip->eoc_wake_lock);
		
		chip->pantech_cable = get_cable_type(chip, DRIVER_BOOTING);
		pr_info("cable type : %d, ADC[%d]\n", chip->pantech_cable, chip->cable_adc);
		set_power_supply_type(chip, chip->pantech_cable);

		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
		chip->batt_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else {
//20120830 ugot2do. compile err.
 #ifdef CONFIG_PANTECH_ANDROID_OTG
	      if(!get_pmic_status()) {
			chip->pantech_cable = get_pantech_cable_type(chip);
			if(chip->pantech_cable != PANTECH_OTG)
				external_pmic_id_status(true);
			else {
				pr_info("OTG cable pluged in\n");
		  		smb347_otg_power(1);
			}
		}	
#endif
		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_NONE;
	}
#else	// USE_USBPHY_CABLE_DETECTION

	if(usb_chg_current) {		
		
		wake_lock(&chip->eoc_wake_lock);

		smb347_regs_init();
		
		if(chg_usb_type == USB_SDP_CHARGER) {
			chip->cable_adc = get_cable_id_adc_value();
			if( (chip->cable_adc >= FACT_CABLE_MIN) && (chip->cable_adc <= FACT_CABLE_MAX) ) {
				printk("Factory Cable plugged in\n");
				chip->factory_cable_present = true;

				smb347_write_reg(REG_INPUT_CURRENT_LIMIT, FACTORY_INPUT_LIMIT);
				smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
				smb347_write_reg(REG_CMD_REG_B, HC_MODE);
			}
			else {
			printk("USB plugged in\n");
			smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
			smb347_write_reg(REG_CMD_REG_B, USB_MODE);
		}
			chip->usb_present = true;
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
		}
		else if(chg_usb_type >= USB_DCP_CHARGER) {
			printk("AC plugged in\n");
			smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
			smb347_write_reg(REG_CMD_REG_B, HC_MODE);
			
			chip->dc_present = true;
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
		}
			
		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
		chip->batt_status = POWER_SUPPLY_STATUS_CHARGING;
	}
#ifdef CONFIG_PANTECH_ANDROID_OTG
      if(!get_pmic_status())
      {
		chip->cable_adc = get_cable_id_adc_value();
		printk("^^^^ @@@@ initial_state: %d\n", chip->cable_adc);
		
	  	printk("^^^^ otg cable is already connected\n");

		if(chip->cable_adc >= 1000000){
			external_pmic_id_status(true);
		}else{
	      	smb347_otg_power(1);
        }	  	
	  }else{
		chip->cable_adc = get_cable_id_adc_value();
		printk("^^^^ @@@@ initial_state: %d\n", chip->cable_adc);
		printk("^^^^ maybe usb cable is already connected\n");
	  }
#endif	
#endif

  	pm_chg_enable_irq(chip, pmic_chg_irq_data[BATT_REMOVED_IRQ].irq_id);

	chip->batt_present = is_battery_present();
	if(chip->batt_present) {
		chip->batt_soc = max17058_get_soc();
		chip->batt_vcell = max17058_get_vcell();
		chip->batt_temp = get_batt_temp(chip);
		chip->batt_status = is_batt_status_charging(chip);
		
		printk("initial batt soc:%d, vcell:%d, TEMP:%d, status:%d\n", chip->batt_soc, chip->batt_vcell,
			chip->batt_temp, chip->batt_status);
	}
}

static int smb347_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{		
	/* Check if called before init */
	if (!the_chip)
		return -EINVAL;

	switch (psp) {
//		case POWER_SUPPLY_PROP_CURRENT_MAX:
//			break;
		case POWER_SUPPLY_PROP_PRESENT:
		case POWER_SUPPLY_PROP_ONLINE:
			val->intval = 0;

			if (psy->type == POWER_SUPPLY_TYPE_USB ||
				psy->type == POWER_SUPPLY_TYPE_USB_DCP ||
				psy->type == POWER_SUPPLY_TYPE_USB_CDP ||
				psy->type == POWER_SUPPLY_TYPE_USB_ACA) {
				if(the_chip->usb_present) {
					val->intval = 1;
				}

				return 0;
			}

			if(psy->type == POWER_SUPPLY_TYPE_MAINS) {
				if(the_chip->dc_present) {
					val->intval = 1;
				}

				return 0;
			}
			
			pr_err("Unkown POWER_SUPPLY_TYPE %d\n", psy->type);
			
			break;
		default:
		return -EINVAL;
	}

	return 0;
}

static int smb347_batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	/* Check if called before init */
	if (!the_chip)
		return -EINVAL;
	
	switch (psp) {
		case POWER_SUPPLY_PROP_STATUS:
			val->intval = get_prop_batt_status(the_chip);
			break;
		case POWER_SUPPLY_PROP_CHARGE_TYPE:
			val->intval = get_prop_charge_type(the_chip);
			break;
		case POWER_SUPPLY_PROP_HEALTH:
			val->intval = get_prop_batt_health(the_chip);
			break;
		case POWER_SUPPLY_PROP_PRESENT:
			val->intval = get_prop_batt_present(the_chip);
			break;
		case POWER_SUPPLY_PROP_TECHNOLOGY:
			val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
			val->intval = the_chip->max_voltage_mv * 1000;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
			val->intval = the_chip->min_voltage_mv * 1000;
			break;
		case POWER_SUPPLY_PROP_VOLTAGE_NOW:
			val->intval = get_prop_battery_uvolts(the_chip);
			break;
		case POWER_SUPPLY_PROP_CAPACITY:
			val->intval = get_prop_batt_capacity(the_chip);
			break;
//		case POWER_SUPPLY_PROP_CURRENT_NOW:
//			break;
		case POWER_SUPPLY_PROP_TEMP:
			val->intval = get_prop_batt_temp(the_chip);
			break;
//		case POWER_SUPPLY_PROP_ENERGY_FULL:
//			break;
		default:
			return -EINVAL;
	}

	return 0;
}

static void update_heartbeat(struct work_struct *work)
{	
	struct delayed_work *dwork = to_delayed_work(work);
	struct smb347_chg_chip *chip = container_of(dwork,
				struct smb347_chg_chip, update_heartbeat_work);
	
	int curr_soc=50, curr_vcell=0, curr_temp=0, curr_rcomp=0;
	int temp;
	int curr_status=POWER_SUPPLY_STATUS_UNKNOWN;
#if defined(PANTECH_CHARGER_TEST_MENU)
        int enable;
#endif

	wake_lock(&chip->heartbeat_wake_lock);
	
	/* battery check */
	chip->batt_present = is_battery_present();
	if(chip->batt_present) {
		
		curr_soc = max17058_get_soc();	
		curr_vcell = max17058_get_vcell();
		curr_status = is_batt_status_charging(chip);
		chip->batt_id = get_battery_id(chip);
		temp = get_batt_temp(chip);
		curr_temp = batt_temp_compensation(chip, temp);

		if(battery_temp_trip_changed(chip, curr_temp) || start_auto_recharge(chip, curr_soc))
			set_appropriate_battery_current(chip);
		
		curr_rcomp = max17058_calc_rcomp((curr_temp/10));
		if(chip->rcomp != curr_rcomp) {
			chip->rcomp = curr_rcomp;
			max17058_set_rcomp((u8)chip->rcomp, 0x1F);
			//printk("Set New RCOMP:0x%x\n", chip->rcomp);
		}
			
		printk("[SOC:%d, VCELL:%d, TEMP:%d, VCHG:%d]\n", curr_soc, 
				chip->batt_vcell, curr_temp, chip->charge_output_voltage);		

		chip->batt_soc = curr_soc;
		chip->batt_vcell = curr_vcell;
		chip->batt_temp = curr_temp;
		chip->batt_status = curr_status;
	}
	
#if defined(PANTECH_CHARGER_TEST_MENU)

	enable = atomic_read(&bms_input_flag);
	if(enable)
	{            
		
			chip->charge_output_voltage = get_batt_chg_current(chip);
		
		input_report_rel(bms_input_dev, REL_RX, chip->batt_soc);
		input_report_rel(bms_input_dev, REL_RY, (chip->batt_vcell/1000));                
		input_report_rel(bms_input_dev, REL_RZ, chip->batt_temp);	
		input_report_rel(bms_input_dev, REL_X, chip->charge_output_voltage);	
		input_sync(bms_input_dev);
	}
#endif     

#ifdef DEBUG_TEMP_COMPENSATION_ALGO
	chip->update_time = DEBUG_UPDATE_TIME;
#else
	if(!is_chg_plugged_in(chip) && chip->batt_soc < 5) {	
		chip->update_time = 5000;	// 5 sec
	} else {
		chip->update_time = 60000;	// 60 sec
	}
#endif	
	
	power_supply_changed(&chip->batt_psy);

	schedule_delayed_work(&chip->update_heartbeat_work,
			      round_jiffies_relative(msecs_to_jiffies
						     (chip->update_time)));

	wake_unlock(&chip->heartbeat_wake_lock);
}

#ifdef USE_USBPHY_CABLE_DETECTION
static void update_cable_worker(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct smb347_chg_chip *chip = container_of(dwork,
				struct smb347_chg_chip, update_cable_work);
	
	smb347_regs_init();
	
	if(usb_chg_current <= 2) {
		usb_chg_current = 0;
		
		smb347_write_reg(REG_CMD_REG_A, DEFAULT_CMD_REG_A);
		  
		if(chip->usb_present) {
			if(chip->factory_cable_present) {
				printk("## Factory Cable unplug in\n");
				chip->factory_cable_present = false;
			}
			else {
			printk("## USB unplug in\n");
			}
			chip->usb_present = false;
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
			power_supply_changed(&chip->usb_psy);
		}
		else if(chip->dc_present) {
			printk("## AC unplug in\n");
			chip->dc_present = false;
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
			power_supply_changed(&chip->dc_psy);
		}
		
		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_NONE;
		chip->batt_status = POWER_SUPPLY_STATUS_DISCHARGING;	

		power_supply_changed(&chip->batt_psy);
		
		wake_unlock(&chip->eoc_wake_lock);
	}
	else {
		
		wake_lock(&chip->eoc_wake_lock);
		
		if(chg_usb_type == USB_SDP_CHARGER) {
			chip->cable_adc = get_cable_id_adc_value();
			if( (chip->cable_adc >= FACT_CABLE_MIN) && (chip->cable_adc <= FACT_CABLE_MAX) ) {
				printk("## Factory Cable plugged in\n");
				chip->factory_cable_present = true;

				smb347_write_reg(REG_INPUT_CURRENT_LIMIT, FACTORY_INPUT_LIMIT);
				smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
				smb347_write_reg(REG_CMD_REG_B, HC_MODE);
			}
			else if( (chip->cable_adc >= DOCK_STATION_MIN) && (chip->cable_adc <= DOCK_STATION_MAX) ) {
				printk("## Docking Station plugged in\n");

				smb347_write_reg(REG_INPUT_CURRENT_LIMIT, DEFAULT_INPUT_CURRENT_LIMIT);
				smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
				smb347_write_reg(REG_CMD_REG_B, HC_MODE);
#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
				if(chip->pantech_cable == PANTECH_AUDIO_DOCKING_STATION){
					docking_enable_gpio(1);
				switch_set_state(docking_speaker_sdev, 1);
#endif				
#endif			}
			}
			else {
				printk("## USB plugged in\n");
				smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
				smb347_write_reg(REG_CMD_REG_B, USB_MODE);
			}
			chip->usb_present = true;
			chip->usb_psy.type = POWER_SUPPLY_TYPE_USB;
				
			power_supply_changed(&chip->usb_psy);
		}
		else {
			printk("## AC plug in\n");
			smb347_write_reg(REG_CMD_REG_A, CHG_ENABLE);
			smb347_write_reg(REG_CMD_REG_B, HC_MODE);
			
			chip->dc_present = true;
			chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS;
				
			power_supply_changed(&chip->dc_psy);
		}
	
		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
		chip->batt_status = POWER_SUPPLY_STATUS_CHARGING;

		power_supply_changed(&chip->batt_psy);
	}
}
#endif

#ifdef USE_SMB347_CABLE_DETECTION
static irqreturn_t smb347_chg_detection_handler(int irq, void *dev)
{
	struct smb347_chg_chip *chip = dev;
			
	if(is_chg_plugged_in(chip)) {	
		wake_lock(&chip->eoc_wake_lock);

		chip->pantech_cable = get_cable_type(chip, DRIVER_BOOT_COMPLETED);
		set_power_supply_type(chip, chip->pantech_cable);
		printk("## [%d] plug in. ADC [%d]\n", chip->pantech_cable, chip->cable_adc);

		if(chip->pantech_cable == PANTECH_USB) 
			schedule_delayed_work(&chip->cable_recheck_work,
			      round_jiffies_relative(msecs_to_jiffies(30000)));
		
#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
		if(chip->pantech_cable == PANTECH_AUDIO_DOCKING_STATION){
			docking_enable_gpio(1);
			switch_set_state(docking_speaker_sdev, 1);
		}
#endif	
#endif

		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_FAST;
		chip->batt_status = POWER_SUPPLY_STATUS_CHARGING;

		set_appropriate_battery_current(chip);
				
//		notify_usb_of_the_plugin_event(1);
		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->batt_psy);

	}
	else {
		printk("## [%d] unplug in\n", chip->pantech_cable);	
	
		if(chip->pantech_cable == PANTECH_USB)
			cancel_delayed_work(&chip->cable_recheck_work);
// P12125 Bug fix when full charged, and connect, PC couldn't reconize Device	=>		
		if(chip->pantech_cable == PANTECH_AC)
			notify_usb_of_the_plugin_event(0);
// P12125 Bug fix when full charged, and connect, PC couldn't reconize Device	=<		
			
#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
		if(chip->pantech_cable == PANTECH_AUDIO_DOCKING_STATION){
			docking_enable_gpio(0);
			switch_set_state(docking_speaker_sdev, 0);
		}
#endif
#endif

		chip->charge_type = POWER_SUPPLY_CHARGE_TYPE_NONE;
		chip->batt_status = POWER_SUPPLY_STATUS_DISCHARGING;
		chip->pantech_cable = PANTECH_CABLE_NONE;

		chip->ext_charge_done = false;
		
		set_power_supply_type(chip, chip->pantech_cable);

#if defined(PANTECH_CHARGER_TEST_MENU)
		chip->cable_adc = get_cable_id_adc_value();
#endif		

		
		smb347_write_reg(REG_CHG_CURRENT, therm_normal_val[PANTECH_CABLE_NONE].charge_current);
		smb347_write_reg(REG_INPUT_CURRENT_LIMIT, therm_normal_val[PANTECH_CABLE_NONE].input_cur_limit);
		smb347_write_reg(REG_FLOAT_VOLTAGE, therm_normal_val[PANTECH_CABLE_NONE].float_voltage);
		smb347_write_reg(REG_CMD_REG_A, therm_normal_val[PANTECH_CABLE_NONE].cmd_reg_a);
		smb347_write_reg(REG_CMD_REG_B, therm_normal_val[PANTECH_CABLE_NONE].cmd_reg_b);

		power_supply_changed(&chip->usb_psy);
		power_supply_changed(&chip->dc_psy);
		power_supply_changed(&chip->batt_psy);

		wake_unlock(&chip->eoc_wake_lock);
	}

	return IRQ_HANDLED;
}

static irqreturn_t smb347_chg_status_handler(int irq, void *dev)
{
	u8 data;
	
	struct smb347_chg_chip *chip = dev;

	if(chip->ext_charge_done)
		return IRQ_HANDLED;

	smb347_read_reg(REG_INT_STAT_C, &data);
//	printk("37H : 0x%x\n", data);
	if(data&0x02) {
		printk("Termination Charging Current Hit IRQ\n");
#ifdef SMB347_CHARGER_IC	
		chip->ext_charge_done = true;
#endif
	}

	if(data&0x8)
		return IRQ_HANDLED;
	
#ifdef SMB358_CHARGER_IC	
	if(data&0x20) {
		printk("Re-Charging Battery Threshold IRQ\n");
	}
#endif

	smb347_read_reg(REG_STAT_REG_C, &data);
	if(data&0x80) {
		printk("Asserts an Error IRQ signal\n");
		if(data&0x40) {
			pr_err("An error occurred during charging !!!\n");
			return IRQ_HANDLED;
		}
	}
	
	return IRQ_HANDLED;
}

static void chg_recheck_worker(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct smb347_chg_chip *chip = container_of(dwork,
				struct smb347_chg_chip, cable_recheck_work);
	int curr_cable_type=PANTECH_USB;
	
	if(is_chg_plugged_in(chip)) {
//		curr_cable_type = get_cable_type(chip, DRIVER_BOOT_COMPLETED);

		if(!android_get_udc_state())
			curr_cable_type = PANTECH_AC;

		if(chip->pantech_cable != curr_cable_type) {
	
			chip->pantech_cable = curr_cable_type;
			
			printk("[%s] charger type : %d, ADC [%d]\n", __func__, chip->pantech_cable, chip->cable_adc);

			set_power_supply_type(chip, chip->pantech_cable);

			set_appropriate_battery_current(chip);
				
			power_supply_changed(&chip->usb_psy);
			power_supply_changed(&chip->dc_psy);
		}
	}
}
#endif

#ifdef USE_APQ_WAKEUP_MPP
static irqreturn_t smb347_wakeup_handler(int irq, void *dev)
{
	printk("#### %s ####\n", __func__);
	return IRQ_HANDLED;
}
#endif

#define MAX_BATT_CHECK_CNT 3
static void check_batt_present_worker(struct work_struct *work)
{
	static unsigned int batt_check_cnt;
	int cable_type;
	struct delayed_work *dwork = to_delayed_work(work);
	struct smb347_chg_chip *chip = container_of(dwork,
				struct smb347_chg_chip, batt_check_work);

	if(chip->pantech_cable == PANTECH_FACTORY) {
		power_supply_changed(&chip->batt_psy);
	}
	else
	{
		if(is_battery_present()) {
			if(batt_check_cnt > MAX_BATT_CHECK_CNT) {
				batt_check_cnt = 0;
				return;
			}
			else {
				schedule_delayed_work(&chip->batt_check_work, round_jiffies_relative(msecs_to_jiffies(500)));
				batt_check_cnt++;
				return;
			}		
		}

		cable_type = get_pantech_cable_type(chip);

		if(cable_type == PANTECH_FACTORY)
			return;

		printk("%s arch reset~~ \n", __func__);
//20120830 ugot2do. chaged.
//		arch_reset(0, "oem-34");
        msm_restart(0, "oem-34");
	}
}

static irqreturn_t batt_removed_irq_handler(int irq, void *data)
{
	struct smb347_chg_chip *chip = data;
	int status;
	
	status = pm_chg_get_rt_status(chip, BATT_REMOVED_IRQ);
	if(status)
		schedule_delayed_work(&chip->batt_check_work, 0);
	
	return IRQ_HANDLED;
}

#if defined(PANTECH_HW_REV_TEST) 

    

static int proc_debug_pm_chg_get_HW_RevID(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
    int adc;

    if(NULL == the_chip)
    {
        pr_err("[proc_debug_pm_chg_get_CableID] the_chip is NULL(Error) !!!\n");
        return 0;
    }

    adc = get_hw_rev_adc_value();

    *eof = 1;  
    return sprintf(page, "%d\n", adc);
}
static void create_testmenu_entries(struct smb347_chg_chip *chip)
{
 	struct proc_dir_entry *ent;

	chip->cable_adc = 0;

	pm8921_charger_dir = proc_mkdir(CHARGER_DEV_NAME, NULL);

	if (pm8921_charger_dir == NULL) {
		pr_err("Unable to create /proc/%s directory\n", CHARGER_DEV_NAME);
	}

	ent = create_proc_entry("hw_rev", 0, pm8921_charger_dir);
	if (ent == NULL) 
		pr_err("Unable to create /proc/smb347_charger/hw_rev entry\n");
	
	ent->read_proc = proc_debug_pm_chg_get_HW_RevID;


	
}
#endif
#if defined(PANTECH_CHARGER_TEST_MENU)
static int smb347_charger_test_misc_open(struct inode *inode, struct file *file)
{
		return 0;
}

static long smb347_charger_test_misc_ioctl(struct file *file,
		    unsigned int cmd, unsigned long arg)
{
	int rc;
	u8 rdData = 0;
	int i;
	int mval[3];
	int cval[18];
	int enb=0;
	switch (cmd) {
		case CHARGER_MONITOR_TEST_SET_SMB:
			
			// read pantech cable type
			mval[0] = the_chip->pantech_cable;

			// read VCHG
			if(is_chg_plugged_in(the_chip))
				the_chip->charge_output_voltage = get_batt_chg_current(the_chip);
			else
				the_chip->charge_output_voltage = 0;
			
			mval[1] = the_chip->charge_output_voltage;

			smb347_read_reg((0x3D), &rdData);
			mval[2] = rdData & 0x48;

			rc = copy_to_user((void *)arg, mval, sizeof(mval));
			break;

 		case CHARGER_CHARGING_TEST_SET_SMB:
			
			// read smb347 regs
			for(i=0; i<14; i++){ 				
				smb347_read_reg((0x32+i), &rdData);				
				cval[i] = rdData;
			}	

			// read pantech cable type, adc, batttery ID
			cval[14] = the_chip->pantech_cable;			
			cval[15] = the_chip->cable_adc; 			
			cval[16] = the_chip->batt_id;

			// read VCHG
			if(is_chg_plugged_in(the_chip))				
				the_chip->charge_output_voltage = get_batt_chg_current(the_chip);			
			else				
				the_chip->charge_output_voltage = 0;			

			cval[17] = the_chip->charge_output_voltage;			
			rc = copy_to_user((void *)arg, cval, sizeof(cval));
			break;		

		case CHARGER_DISCHARGING_TEST_SET_SMB:
			
			// write input flag
			rc = copy_from_user((void *)&enb, (void *)arg, sizeof(int));
			atomic_set(&bms_input_flag, enb);

		break;

		case CHARGER_DISCHARGING_TEST_GET_SMB:
			
			// read input flag
			enb = atomic_read(&bms_input_flag);
			rc = copy_to_user((void *)arg, &enb, sizeof(int));
			
			break;
			
	default:
		rc = -EINVAL;
	}
	return rc;
}

static int smb347_charger_battery_test_misc_release(struct inode *inode, struct file *file)
{
		return 0;
}
	
static struct file_operations smb347_charger_test_dev_fops = {
	.owner = THIS_MODULE,
	.open = smb347_charger_test_misc_open,
	.unlocked_ioctl	= smb347_charger_test_misc_ioctl,
	.release	= smb347_charger_battery_test_misc_release,
};
	
struct miscdevice smb347_charger_test_misc_device = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "smb347_charger",
	.fops	= &smb347_charger_test_dev_fops,
};
	
int smb347_charger_battery_charging_test_init(void)
{
	return misc_register(&smb347_charger_test_misc_device);
}
#endif

static int __devinit smb347_i2c_probe(struct i2c_client *client,
                                  const struct i2c_device_id *id)
{
	unsigned char data=0;
	int ret_rd=0;
	
	if (!i2c_check_functionality(client->adapter,
	                             I2C_FUNC_SMBUS_BYTE_DATA)) {
	        dev_err(&client->dev, "SMBUS Byte Data not Supported\n");
	        return -EIO;
	}
	
	the_chip->smb347_client = client;

	ret_rd = smb347_read_reg(REG_STAT_REG_E, &data);

	if(ret_rd < 0)
	{
		pr_err("%s smb347 reg 0x3F read failed & retry ~~\n", __func__);
		mdelay(200);
	smb347_read_reg(REG_STAT_REG_E, &data);
	}
	return 0;
}

static int __devexit	smb347_i2c_remove(struct i2c_client *client)
{
        the_chip->smb347_client = NULL;
        return 0;
}

static void smb347_i2c_shutdown(struct i2c_client *client)
{
#ifdef CONFIG_PANTECH_ANDROID_OTG
	smb347_otg_power(0);
#endif
}

static const struct i2c_device_id smb347_id[] = {
        {"smb347-i2c", 0},
        {},
};

MODULE_DEVICE_TABLE(i2c, smb347_id);

static struct i2c_driver smb347_i2c_driver = {
        .driver = {
                   .name = "smb347-i2c",
        },
        .probe = smb347_i2c_probe,
        .remove = __devexit_p(smb347_i2c_remove),
        .shutdown = smb347_i2c_shutdown,
        .id_table = smb347_id,
};

static void pm8921_chg_force_19p2mhz_clk(struct smb347_chg_chip *chip)
{
	int err;
	u8 temp;

	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD3;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD5;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	udelay(183);

	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD0;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
	udelay(32);

	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD3;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
}

static void pm8921_chg_set_hw_clk_switching(struct smb347_chg_chip *chip)
{
	int err;
	u8 temp;

	temp  = 0xD1;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}

	temp  = 0xD0;
	err = pm8xxx_writeb(chip->dev->parent, CHG_TEST, temp);
	if (err) {
		pr_err("Error %d writing %d to addr %d\n", err, temp, CHG_TEST);
		return;
	}
}

#define PM8921_CHG_WD_MASK 0x1F
static int pm_chg_disable_wd(struct smb347_chg_chip *chip)
{
	/* writing 0 to the wd timer disables it */
	return pm_chg_masked_write(chip, CHG_TWDOG, PM8921_CHG_WD_MASK, 0);
}

#define BMS_CONTROL    0x224       
#define EN_BMS_BIT     BIT(7)   

#define ENUM_TIMER_STOP_BIT	BIT(1)
#define BOOT_DONE_BIT		BIT(6)
#define CHG_BATFET_ON_BIT	BIT(3)
#define CHG_VCP_EN		BIT(0)
#define CHG_BAT_TEMP_DIS_BIT	BIT(2)
#define SAFE_CURRENT_MA		1500
#define VREF_BATT_THERM_FORCE_ON	BIT(7)
static int __devinit pm8921_chg_hw_init(struct smb347_chg_chip *chip)
{
	int rc;
	
	pm_chg_masked_write(chip,BMS_CONTROL,EN_BMS_BIT,0);
	
	/* Disable the ENUM TIMER */
	rc = pm_chg_masked_write(chip, PBL_ACCESS2, ENUM_TIMER_STOP_BIT,
			ENUM_TIMER_STOP_BIT);
	if (rc) {
		pr_err("Failed to set enum timer stop rc=%d\n", rc);
		return rc;
	}

	rc = pm_chg_disable_wd(chip);
	if (rc) {
		pr_err("Failed to disable wd rc=%d\n", rc);
		return rc;
	}

#if 0
	rc = pm_chg_masked_write(chip, CHG_CNTRL_2,
				CHG_BAT_TEMP_DIS_BIT, 0);
	if (rc) {
		pr_err("Failed to enable temp control chg rc=%d\n", rc);
		return rc;
	}
#else
	rc = pm_chg_masked_write(chip, CHG_CNTRL_2,
				CHG_BAT_TEMP_DIS_BIT, 1);
	if (rc) {
		pr_err("Failed to disable temp control chg rc=%d\n", rc);
		return rc;
	}
#endif

#if 0
	rc = pm_chg_batt_cold_temp_config(chip, chip->cold_thr);
	if (rc) {
		pr_err("Failed to set cold config %d  rc=%d\n",
						chip->cold_thr, rc);
	}

	rc = pm_chg_batt_hot_temp_config(chip, chip->hot_thr);
	if (rc) {
		pr_err("Failed to set hot config %d  rc=%d\n",
						chip->hot_thr, rc);
	}
#endif
	/* Workarounds for die 1.1 and 1.0 */
	if (pm8xxx_get_revision(chip->dev->parent) < PM8XXX_REVISION_8921_2p0) {
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST2, 0xF1);
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xCE);
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xD8);

		/* software workaround for correct battery_id detection */
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_0, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_1, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_2, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_TXRX_SAMPLE_DATA_3, 0xFF);
		pm8xxx_writeb(chip->dev->parent, PSI_CONFIG_STATUS, 0x0D);
		udelay(100);
		pm8xxx_writeb(chip->dev->parent, PSI_CONFIG_STATUS, 0x0C);
	}

	/* Workarounds for die 3.0 */
	if (pm8xxx_get_revision(chip->dev->parent) == PM8XXX_REVISION_8921_3p0)
		pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xAC);

	pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0xD9);

	/* Disable EOC FSM processing */
	pm8xxx_writeb(chip->dev->parent, CHG_BUCK_CTRL_TEST3, 0x91);

	pm8921_chg_force_19p2mhz_clk(chip);

	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON,
						VREF_BATT_THERM_FORCE_ON);
	if (rc)
		pr_err("Failed to Force Vref therm rc=%d\n", rc);

	return 0;
}

static int smb347_charger_suspend_noirq(struct device *dev)
{
	int rc;
	struct smb347_chg_chip *chip = dev_get_drvdata(dev);

	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON, 0);
	if (rc)
		pr_err("Failed to Force Vref therm off rc=%d\n", rc);
	pm8921_chg_set_hw_clk_switching(chip);
	return 0;
}

static int smb347_charger_resume_noirq(struct device *dev)
{
	int rc;
	struct smb347_chg_chip *chip = dev_get_drvdata(dev);

	pm8921_chg_force_19p2mhz_clk(chip);

	rc = pm_chg_masked_write(chip, CHG_CNTRL, VREF_BATT_THERM_FORCE_ON,
						VREF_BATT_THERM_FORCE_ON);
	if (rc)
		pr_err("Failed to Force Vref therm on rc=%d\n", rc);
	return 0;
}

static int smb347_charger_resume(struct device *dev)
{
	struct smb347_chg_chip *chip = dev_get_drvdata(dev);
	
#ifdef USE_APQ_WAKEUP_MPP
	disable_irq_wake(chip->wakeup_irq);
	disable_irq_nosync(chip->wakeup_irq);
#elif defined(USE_APQ_WAKEUP_GPIO)
	disable_irq_wake(chip->wakeup_irq);
#endif

	schedule_delayed_work(&chip->update_heartbeat_work,
        	round_jiffies_relative(msecs_to_jiffies(0)));
	
	return 0;
}

static int smb347_charger_suspend(struct device *dev)
{
	struct smb347_chg_chip *chip = dev_get_drvdata(dev);
	
	cancel_delayed_work(&chip->update_heartbeat_work);

#ifdef USE_APQ_WAKEUP_MPP	
	enable_irq_wake(chip->wakeup_irq);
	enable_irq(chip->wakeup_irq);
#elif defined(USE_APQ_WAKEUP_GPIO)
	enable_irq_wake(chip->wakeup_irq);
#endif

	return 0;
}

static int __devinit smb347_charger_probe(struct platform_device *pdev)
{
	int rc = 0;

	struct smb347_chg_chip *chip;
	const struct pm8921_charger_platform_data *pdata
				= pdev->dev.platform_data;

	pr_err("## SMB347 Charger probe OK ##\n");
	
	if (!pdata) {
		pr_err("missing platform data\n");
		return -EINVAL;
	}

	chip = kzalloc(sizeof(struct smb347_chg_chip),
					GFP_KERNEL);
	if (!chip) {
		pr_err("Cannot allocate smb347_chg_chip\n");
		return -ENOMEM;
	}
	
	chip->dev = &pdev->dev;
	chip->ttrkl_time = pdata->ttrkl_time;
	chip->update_time = pdata->update_time;
	chip->max_voltage_mv = pdata->max_voltage;
	chip->min_voltage_mv = pdata->min_voltage;
	chip->resume_voltage_delta = pdata->resume_voltage_delta;
	chip->term_current = pdata->term_current;
	chip->vbat_channel = pdata->charger_cdata.vbat_channel;
	chip->batt_temp_channel = pdata->charger_cdata.batt_temp_channel;
	chip->batt_id_channel = pdata->charger_cdata.batt_id_channel;
	chip->batt_id_min = pdata->batt_id_min;
	chip->batt_id_max = pdata->batt_id_max;
	if (pdata->cool_temp != INT_MIN)
		chip->cool_temp_dc = pdata->cool_temp * 10;
	else
		chip->cool_temp_dc = INT_MIN;

	if (pdata->warm_temp != INT_MIN)
		chip->warm_temp_dc = pdata->warm_temp * 10;
	else
		chip->warm_temp_dc = INT_MIN;

	chip->temp_check_period = pdata->temp_check_period;
	chip->max_bat_chg_current = pdata->max_bat_chg_current;
	chip->cool_bat_chg_current = pdata->cool_bat_chg_current;
	chip->warm_bat_chg_current = pdata->warm_bat_chg_current;
	chip->cool_bat_voltage = pdata->cool_bat_voltage;
	chip->warm_bat_voltage = pdata->warm_bat_voltage;
	chip->keep_btm_on_suspend = pdata->keep_btm_on_suspend;
	chip->trkl_voltage = pdata->trkl_voltage;
	chip->weak_voltage = pdata->weak_voltage;
	chip->trkl_current = pdata->trkl_current;
	chip->weak_current = pdata->weak_current;
	chip->vin_min = pdata->vin_min;
	chip->thermal_mitigation = pdata->thermal_mitigation;
	chip->thermal_levels = pdata->thermal_levels;

	chip->cold_thr = pdata->cold_thr;
	chip->hot_thr = pdata->hot_thr;

	chip->therm_type = BATT_THERM_UNKNOWN;
	chip->pantech_cable = PANTECH_CABLE_NONE;

#ifdef USE_APQ_WAKEUP_MPP
	chip->wakeup_irq = gpio_to_irq(PM8921_MPP_PM_TO_SYS(1));	// MPP_01
#elif defined(USE_APQ_WAKEUP_GPIO)
	chip->wakeup_irq = gpio_to_irq(SC_INOK);
#endif

	smb347_gpio_init();
	pm8921_chg_hw_init(chip);
	
	chip->usb_psy.name = "usb",
	chip->usb_psy.type = POWER_SUPPLY_TYPE_USB,
	chip->usb_psy.supplied_to = pm_power_supplied_to,
	chip->usb_psy.num_supplicants = ARRAY_SIZE(pm_power_supplied_to),
	chip->usb_psy.properties = pm_power_props,
	chip->usb_psy.num_properties = ARRAY_SIZE(pm_power_props),
	chip->usb_psy.get_property = smb347_power_get_property,

	chip->dc_psy.name = "ac",
	chip->dc_psy.type = POWER_SUPPLY_TYPE_MAINS,
	chip->dc_psy.supplied_to = pm_power_supplied_to,
	chip->dc_psy.num_supplicants = ARRAY_SIZE(pm_power_supplied_to),
	chip->dc_psy.properties = pm_power_props,
	chip->dc_psy.num_properties = ARRAY_SIZE(pm_power_props),
	chip->dc_psy.get_property = smb347_power_get_property,
	
	chip->batt_psy.name = "battery",
	chip->batt_psy.type = POWER_SUPPLY_TYPE_BATTERY,
	chip->batt_psy.properties = msm_batt_power_props,
	chip->batt_psy.num_properties = ARRAY_SIZE(msm_batt_power_props),
	chip->batt_psy.get_property = smb347_batt_power_get_property,
	chip->batt_psy.external_power_changed = NULL,

	chip->batt_soc = 50;
	
	the_chip = chip;

	rc = i2c_add_driver(&smb347_i2c_driver);
	if (rc < 0) {
		pr_err("SMB347 I2C add driver failed %d", rc);
		goto free_chip;
	}

	rc = power_supply_register(chip->dev, &chip->usb_psy);
	if (rc < 0) {
		pr_err("power_supply_register usb failed rc = %d\n", rc);
		goto free_chip;
	}

	rc = power_supply_register(chip->dev, &chip->dc_psy);
	if (rc < 0) {
		pr_err("power_supply_register usb failed rc = %d\n", rc);
		goto unregister_usb;
	}
	
	rc = power_supply_register(chip->dev, &chip->batt_psy);
	if (rc < 0) {
		pr_err("power_supply_register batt failed rc = %d\n", rc);
		goto unregister_dc;
	}

	platform_set_drvdata(pdev, chip);
	
#ifdef USE_SMB347_CABLE_DETECTION	
	rc = request_threaded_irq(gpio_to_irq(SC_INOK), NULL, smb347_chg_detection_handler, 
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			"smb347_chg_dectect", chip);
	if(rc) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n", __func__, SC_INOK, rc);
		goto unregister_batt;
	}
#endif

	rc = request_threaded_irq(gpio_to_irq(SC_STAT), NULL, smb347_chg_status_handler, 
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "smb347_chg_status", chip);
	if(rc) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n", __func__, SC_STAT, rc);
		goto unregister_batt;
	}

#ifdef USE_APQ_WAKEUP_MPP
	rc = request_threaded_irq(chip->wakeup_irq, NULL, smb347_wakeup_handler, 
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "smb347_wakeup_irq", chip);
	if(rc) {
		pr_err("%s request_threaded_irq failed for %d rc =%d\n", __func__, chip->wakeup_irq, rc);
		goto unregister_batt;
	}

	disable_irq(chip->wakeup_irq);
#endif

#if 1 // P11220_pmic_irq_handler
	rc = request_irqs(chip, pdev);
#endif

	wake_lock_init(&chip->heartbeat_wake_lock, WAKE_LOCK_SUSPEND, "smb347_heartbeat");
	wake_lock_init(&chip->eoc_wake_lock, WAKE_LOCK_SUSPEND, "smb347_eoc");
	
	/*
	 * if both the cool_temp_dc and warm_temp_dc are invalid device doesnt
	 * care for jeita compliance
	 */
#if 0	 
	if (!(chip->cool_temp_dc == INT_MIN && chip->warm_temp_dc == INT_MIN)) {
		rc = configure_btm(chip);
		if (rc) {
			pr_err("couldn't register with btm rc=%d\n", rc);
			goto unregister_batt;
		}
	}
#endif	

#ifdef USE_SMB347_CABLE_DETECTION
	INIT_DELAYED_WORK(&chip->cable_recheck_work, chg_recheck_worker);
#endif

	determine_initial_state(the_chip);
	
	if (chip->update_time) {
		INIT_DELAYED_WORK(&chip->update_heartbeat_work,
							update_heartbeat);
		schedule_delayed_work(&chip->update_heartbeat_work,
				      round_jiffies_relative(msecs_to_jiffies(chip->update_time)));
	}
#ifdef USE_USBPHY_CABLE_DETECTION	
	INIT_DELAYED_WORK(&chip->update_cable_work, update_cable_worker);
#endif

	INIT_DELAYED_WORK(&chip->batt_check_work, check_batt_present_worker);
	if(!chip->batt_present) {
		schedule_delayed_work(&chip->batt_check_work, round_jiffies_relative(msecs_to_jiffies(10000)));
	}
	smb347_subdevices_register();

#ifdef PANTECH_HW_REV_TEST
	create_testmenu_entries(chip);
#endif
#if defined(PANTECH_CHARGER_TEST_MENU)
	smb347_charger_battery_charging_test_init();
#endif

	return 0;

unregister_batt:
	power_supply_unregister(&chip->batt_psy);
unregister_dc:
	power_supply_unregister(&chip->dc_psy);	
unregister_usb:
	power_supply_unregister(&chip->usb_psy);
free_chip:
	kfree(chip);
	return rc;
}

static int __devexit smb347_charger_remove(struct platform_device *pdev)
{
	struct smb347_chg_chip *chip = platform_get_drvdata(pdev);

#if defined(PANTECH_CHARGER_TEST_MENU)
        input_unregister_device(bms_input_dev);
        input_free_device(bms_input_dev);
#endif

#if 1 // P11220_pmic_irq_handler
	free_irqs(chip);
#endif
#ifdef CONFIG_SKY_SND_DOCKING_CRADLE   //20120521 jhsong : report docking speaker event
#if defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	kfree(docking_speaker_sdev);
#endif
#endif
	platform_set_drvdata(pdev, NULL);
	the_chip = NULL;
	kfree(chip);
	return 0;	
}

static const struct dev_pm_ops smb347_pm_ops = {
	.suspend	= smb347_charger_suspend,
	.suspend_noirq  = smb347_charger_suspend_noirq,
	.resume_noirq   = smb347_charger_resume_noirq,
	.resume		= smb347_charger_resume,
};
static struct platform_driver smb347_charger_driver = {
	.probe		= smb347_charger_probe,
	.remove		= __devexit_p(smb347_charger_remove),
	.driver		= {
			.name	= CHARGER_DEV_NAME,
			.owner	= THIS_MODULE,
			.pm	= &smb347_pm_ops,
	},
};

static int __init smb347_charger_init(void)
{
	return platform_driver_register(&smb347_charger_driver);
}

static void __exit smb347_charger_exit(void)
{
	platform_driver_unregister(&smb347_charger_driver);
}

late_initcall(smb347_charger_init);
module_exit(smb347_charger_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("smb347 charger/battery driver");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:" CHARGER_DEV_NAME);


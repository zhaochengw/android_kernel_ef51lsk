/* Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/i2c.h>
#include <linux/i2c/smb349.h>
#include <linux/i2c/sx150x.h>
#include <linux/slimbus/slimbus.h>
#include <linux/mfd/wcd9xxx/core.h>
#include <linux/mfd/wcd9xxx/pdata.h>
#include <linux/mfd/pm8xxx/misc.h>
#include <linux/msm_ssbi.h>
#include <linux/spi/spi.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/qcom_crypto_device.h>
#include <linux/msm_ion.h>
#include <linux/memory.h>
#include <linux/memblock.h>
#include <linux/msm_thermal.h>
#include <linux/i2c/atmel_mxt_ts.h>
#if 0 //!defined(CONFIG_TOUCHSCREEN_CYTTSP_GEN4)
#include <linux/cyttsp.h>
#endif
#include <linux/i2c/isa1200.h>
#include <linux/gpio_keys.h>
#include <linux/epm_adc.h>
#include <linux/i2c/sx150x.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach/mmc.h>
#include <linux/platform_data/qcom_wcnss_device.h>

#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/ion.h>
#include <linux/usb/msm_hsusb.h>
#include <linux/usb/android.h>
#include <mach/socinfo.h>
#include <mach/msm_spi.h>
#include "timer.h"
#include "devices.h"
#include <mach/gpio.h>
#include <mach/gpiomux.h>
#include <mach/rpm.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <mach/msm_memtypes.h>
#include <linux/bootmem.h>
#include <asm/setup.h>
#include <mach/dma.h>
#include <mach/msm_dsps.h>
#include <mach/msm_bus_board.h>
#include <mach/cpuidle.h>
#include <mach/mdm2.h>
#include <linux/msm_tsens.h>
#include <mach/msm_xo.h>
#include <mach/msm_rtb.h>
#include <mach/msm_serial_hs.h>
#include <sound/cs8427.h>
#include <media/gpio-ir-recv.h>
#include <linux/fmem.h>
#include <mach/msm_pcie.h>
#include <mach/restart.h>
#include <mach/msm_iomap.h>

#include "msm_watchdog.h"
#include "board-8064.h"
#include "spm.h"
#include <mach/mpm.h>
#include "rpm_resources.h"
#include "pm.h"
#include "pm-boot.h"
#include "devices-msm8x60.h"
#include "smd_private.h"

#define MHL_GPIO_INT           30
#define MHL_GPIO_RESET         35

#if defined(CONFIG_PN544)
#include <linux/pn544.h>
#endif

#ifdef CONFIG_SKY_DMB_SPI_GPIO
#include <linux/spi/spi_gpio.h>
#endif


#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_PANTECH)
#include <linux/rmi.h>

#define THINSTYK_ADDR	0x20
#define THINSTYK_ATTN	55

struct syna_gpio_data {
	u16 gpio_number;
	char* gpio_name;
};

static int synaptics_touchpad_gpio_setup(void *gpio_data, bool configure)
{
	int retval=0;
	struct syna_gpio_data *data = gpio_data;

	if (configure) {
		retval = gpio_request(data->gpio_number, "rmi4_attn");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			return retval;
		}

		retval = gpio_direction_input(data->gpio_number);
		if (retval) {
			pr_err("%s: Failed to setup attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			gpio_free(data->gpio_number);
		}
	} else {
		pr_warn("%s: No way to deconfigure gpio %d.",
		       __func__, data->gpio_number);
	}

	return retval;
}

static struct syna_gpio_data thinstyk_gpiodata = {
	.gpio_number = THINSTYK_ATTN,	
	.gpio_name = "sdmmc2_clk.gpio_130",
};

static struct rmi_device_platform_data thinstyk_platformdata = {
	.driver_name = "rmi_generic",
	.attn_gpio = THINSTYK_ATTN,
	.attn_polarity = RMI_ATTN_ACTIVE_LOW,
	.gpio_data = &thinstyk_gpiodata,
	.gpio_config = synaptics_touchpad_gpio_setup,
	.sensor_name = "touch",
};

static struct i2c_board_info synaptics_i2c_info[] = {
	{
		I2C_BOARD_INFO("pantech_touch", THINSTYK_ADDR),        
		.platform_data = &thinstyk_platformdata,
	},
};

#endif

#if defined(CONFIG_TOUCHSCREEN_CYTTSP_GEN4)
#define CY_I2C_NAME     "cyttsp4-i2c"
#define CY_I2C_TCH_ADR	0x24
static struct i2c_board_info cyttsp4_i2c_info[] = {
	{
		I2C_BOARD_INFO(CY_I2C_NAME, CY_I2C_TCH_ADR),
		//.platform_data = &cyttsp4_i2c_touch_platform_data,
	},
};
#endif //CONFIG_TOUCHSCREEN_CYTTSP_GEN4

#if defined(CONFIG_TOUCHSCREEN_PP8360)
#include <linux/i2c-gpio.h>
#define GPIO_BACK_TOUCH_SCL  12
#define GPIO_BACK_TOUCH_SDA 13
#define BACK_TOUCH_I2C_BUS_ID    18
#define BACK_TOUCH_I2C_CHIP_ADDR 0x0e

static struct i2c_gpio_platform_data i2c_gpio_backtouch_data = {
	.scl_pin = GPIO_BACK_TOUCH_SCL,
	.sda_pin = GPIO_BACK_TOUCH_SDA,
//	.udelay = 4,
	.udelay = 2,

};

struct platform_device msm_device_i2c_gpio_backtouch = {
	.name = "i2c-gpio",
	.id = BACK_TOUCH_I2C_BUS_ID, // id important value
	.dev = {
		.platform_data = &i2c_gpio_backtouch_data,
	}
};

static struct i2c_board_info i2c_back_touch_devices[]  = {
	{
		I2C_BOARD_INFO("tchkeypt", BACK_TOUCH_I2C_CHIP_ADDR),
	},
};

static void sky_back_touch_i2c_gpio_init(void)
{
	/*int rc = 0;

	rc = gpio_request(GPIO_BACK_TOUCH_SCL,"back_touch_scl");
	if(rc<0) {
		dbg(KERN_ERR "fail to request gpio(back_touch_scl). rc=%d\n",rc);
		return rc;
	}
	gpioConfig = GPIO_CFG(GPIO_BACK_TOUCH_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA);
	rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
	if(rc<0) {
		dbg(KERN_ERR "fail to enable gpio(back_touch_scl). rc=%d\n",rc);
		return rc;
	}
	gpio_set_value(GPIO_BACK_TOUCH_SCL, (int)arg);
	dbg("set gpio(GPIO_BACK_TOUCH_SCL) : config %d\n", (int)arg);
	gpio_free(GPIO_BACK_TOUCH_SCL);

	
	rc = gpio_request(GPIO_BACK_TOUCH_SDA,"back_touch_sda");
	if(rc<0) {
		dbg(KERN_ERR "fail to request gpio(back_touch_sda). rc=%d\n",rc);
		return rc;
	}
	gpioConfig = GPIO_CFG(GPIO_BACK_TOUCH_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA);
	rc = gpio_tlmm_config(gpioConfig, GPIO_CFG_ENABLE);
	if(rc<0) {
		dbg(KERN_ERR "fail to enable gpio(back_touch_sda). rc=%d\n",rc);
		return rc;
	}
	gpio_set_value(GPIO_BACK_TOUCH_SDA, (int)arg);
	dbg("set gpio(GPIO_BACK_TOUCH_SDA) : config %d\n", (int)arg);
	gpio_free(GPIO_BACK_TOUCH_SDA);

	*/
	i2c_register_board_info(BACK_TOUCH_I2C_BUS_ID, i2c_back_touch_devices, ARRAY_SIZE(i2c_back_touch_devices));
}
#endif /* CONFIG_TOUCHSCREEN_PP8360 */

#ifdef CONFIG_PIEZO
#include <linux/i2c-gpio.h>
#define TS5000_I2C_BUS_ID    19
#if (defined(CONFIG_MACH_APQ8064_EF51S)&&(CONFIG_BOARD_VER>=CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51K)&&(CONFIG_BOARD_VER>=CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51L)&&(CONFIG_BOARD_VER>=CONFIG_TP10))
#define GPIO_PIN_TS5000_SCL  54
#define GPIO_PIN_TS5000_SDA  53
#elif (defined(CONFIG_MACH_APQ8064_EF51S)&&(CONFIG_BOARD_VER<CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51K)&&(CONFIG_BOARD_VER<CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51L)&&(CONFIG_BOARD_VER<CONFIG_TP10))
#define GPIO_PIN_TS5000_SCL  85
#define GPIO_PIN_TS5000_SDA  86
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define GPIO_PIN_TS5000_SCL  6 //temp
#define GPIO_PIN_TS5000_SDA  6//temp
#endif
#define TS5000_I2C_UDELAY    2

#define TS5000_I2C_NAME     "ts5000-i2c"
#define TS5000_I2C_ADDR     0x59
#if 0
static struct i2c_board_info ts5000_i2c_info[] = {
	{
		I2C_BOARD_INFO(TS5000_I2C_NAME, TS5000_I2C_ADDR),
	},
};
#endif
#ifndef CONFIG_PIEZO_ECO
static struct i2c_gpio_platform_data ts5000_i2c_gpio_data = {
	.scl_pin = GPIO_PIN_TS5000_SCL,
	.sda_pin = GPIO_PIN_TS5000_SDA,
	.udelay = TS5000_I2C_UDELAY,
};
struct platform_device ts5000_i2c_gpio_device = {
	.name = "i2c-gpio",
	.id = TS5000_I2C_BUS_ID,
	.dev = {
		.platform_data = &ts5000_i2c_gpio_data,
	}
};
#endif // CONFIG_PIEZO_ECO
#if 0
static void ts5000_i2c_gpio_init(void)
{
	i2c_register_board_info(TS5000_I2C_BUS_ID, ts5000_i2c_info, ARRAY_SIZE(ts5000_i2c_info));
}
#endif
#endif /* CONFIG_PIEZO */
#if defined(CONFIG_SKY_DMB_I2C_GPIO)
#include <linux/i2c-gpio.h>
#endif  /* CONFIG_SKY_DMB_I2C_GPIO */

#ifdef CONFIG_SKY_DMB_I2C_CMD
#ifdef CONFIG_SKY_TDMB_INC_BB
#define DMB_I2C_CHIP_ADDR 0x80 // TDMB INC T3900
#elif defined(CONFIG_SKY_TDMB_FCI_BB)
#define DMB_I2C_CHIP_ADDR 0xB0 // TDMB FCI FC8053
#elif defined(CONFIG_SKY_TDMB_RTV_BB)
#define DMB_I2C_CHIP_ADDR 0x86 // TDMB RaonTech MTV350
#elif defined(CONFIG_SKY_ISDBT_SHARP_BB)
#define DMB_I2C_CHIP_ADDR 0xc2 // ISDB-T Sharp (11000010)
#endif

static struct i2c_board_info i2c_dmb_devices[]  = {
	{
		I2C_BOARD_INFO("dmb_i2c", DMB_I2C_CHIP_ADDR >> 1),
	},
};

#ifdef CONFIG_SKY_DMB_I2C_GPIO
//GPIO not QUP
#if (defined(CONFIG_MACH_APQ8064_EF51S)&&(CONFIG_BOARD_VER>=CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51K)&&(CONFIG_BOARD_VER>=CONFIG_TP10)) || (defined(CONFIG_MACH_APQ8064_EF51L)&&(CONFIG_BOARD_VER>=CONFIG_TP10))
#define GPIO_PIN_DMB_SCL  85
#define GPIO_PIN_DMB_SDA  86
#else
#define GPIO_PIN_DMB_SCL  54  //GSBI5
#define GPIO_PIN_DMB_SDA  53
#endif

#define DMB_I2C_UDELAY    2	/* 250 KHz */
#define DMB_I2C_BUS_ID    12

static struct i2c_gpio_platform_data i2c_gpio_dmb_data = {
	.scl_pin = GPIO_PIN_DMB_SCL,
	.sda_pin = GPIO_PIN_DMB_SDA,
	.udelay = DMB_I2C_UDELAY,
};

struct platform_device msm_device_i2c_gpio_dmb = {
	.name = "i2c-gpio",
	.id = DMB_I2C_BUS_ID, // id important value
	.dev = {
		.platform_data = &i2c_gpio_dmb_data,
	}
};

static uint32_t dmb_i2c_gpio_table[] = {
#if (defined(CONFIG_MACH_APQ8064_EF51S) ||defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L))
	GPIO_CFG(GPIO_PIN_DMB_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_PIN_DMB_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA),
#else
	GPIO_CFG(GPIO_PIN_DMB_SCL, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
	GPIO_CFG(GPIO_PIN_DMB_SDA, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
#endif
};

static void sky_dmb_i2c_gpio_init(void)
{
	int ret = 0, i=0;
	for (i = 0; i < ARRAY_SIZE(dmb_i2c_gpio_table); ++i) {
		ret= gpio_tlmm_config(dmb_i2c_gpio_table[i], GPIO_CFG_ENABLE);
		if ( ret ) {
			pr_err( "%s: Failed dmb_i2c_gpio_table i2c gpio_tlmm_config(%d)=%d\n", __func__, i, ret);
			break;
		}
	}

	i2c_register_board_info(DMB_I2C_BUS_ID, i2c_dmb_devices, ARRAY_SIZE(i2c_dmb_devices));
}

#endif /* CONFIG_SKY_DMB_I2C_GPIO */
#endif /* CONFIG_SKY_DMB_I2C_CMD */

#ifdef CONFIG_SKY_DMB_SPI_IF
static struct spi_board_info dmb_spi_board_info[] __initdata = {
	{
		.modalias       = "dmb_spi",
  #if defined(CONFIG_SKY_TDMB_TCC_BB)
		.mode           = SPI_MODE_2,
  #else
		.mode           = SPI_MODE_0,
  #endif
		.bus_num        = 7,
		.chip_select    = 0,
  #ifdef CONFIG_SKY_TDMB_INC_BB
		.max_speed_hz   = 5400000,
  #else
		.max_speed_hz   = 10800000,
  #endif
#ifdef CONFIG_SKY_DMB_SPI_GPIO
		.controller_data = (void *) 84, // chip select
#endif
	}
};

#ifdef CONFIG_SKY_DMB_SPI_HW
static struct msm_spi_platform_data apq8064_qup_spi_gsbi7_pdata = {
  #ifdef CONFIG_SKY_TDMB_INC_BB
	.max_clock_speed = 5400000, // 5.4Mhz
  #else
	.max_clock_speed = 10800000,
  #endif
};
#endif /* CONFIG_SKY_DMB_SPI_HW */

#ifdef CONFIG_SKY_DMB_SPI_GPIO
static struct spi_gpio_platform_data gpio_spi_tdmb_data = {
	.sck = 85,
	.mosi = 82,
	.miso = 83,
	.num_chipselect = 1,
};
struct platform_device gpio_spi_tdmb_device = {
	.name = "spi_gpio",
	.id = 7,
	.dev = {
		.platform_data = &gpio_spi_tdmb_data,
	}
};
#endif /* CONFIG_SKY_DMB_SPI_GPIO */
#endif /* CONFIG_SKY_DMB_SPI_IF */

#if 0//def CONFIG_SKY_DMB_PMIC_19200
static struct msm_xo_voter *xo_handle_a1;

static int configure_dmb_xo(void)
{
	int rc = 0;
	
	xo_handle_a1 = msm_xo_get(MSM_XO_TCXO_A1, "DMB_XTAL");
	if (IS_ERR(xo_handle_a1)) {
		rc = PTR_ERR(xo_handle_a1);
		pr_err("[DMB] Failed to get MSM_XO_TCXO_A1 voter (%d)\n", rc);
		return 0;	//goto fail;
	}

	rc = msm_xo_mode_vote(xo_handle_a1, MSM_XO_MODE_PIN_CTRL);
	if (rc < 0) {
		pr_err("[DMB] Configuring MSM_XO_MODE_PIN_CTRL failed (%d)\n", rc);
		return 0;	//goto msm_xo_vote_fail;
	}
  
  return 1;
}
#endif /* CONFIG_SKY_DMB_PMIC_19200 */

#define MSM_PMEM_ADSP_SIZE         0x7800000
#define MSM_PMEM_AUDIO_SIZE        0x4CF000
#ifdef CONFIG_FB_MSM_HDMI_AS_PRIMARY
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#else
#define MSM_PMEM_SIZE 0x4000000 /* 64 Mbytes */
#endif

#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
#define HOLE_SIZE		0x20000
#define MSM_CONTIG_MEM_SIZE  0x65000
#ifdef CONFIG_MSM_IOMMU
#define MSM_ION_MM_SIZE		0x3800000
#define MSM_ION_SF_SIZE		0
#define MSM_ION_QSECOM_SIZE	0x780000 /* (7.5MB) */
#define MSM_ION_HEAP_NUM	7
#else
#define MSM_ION_MM_SIZE		MSM_PMEM_ADSP_SIZE
#define MSM_ION_SF_SIZE		MSM_PMEM_SIZE
#define MSM_ION_QSECOM_SIZE	0x600000 /* (6MB) */
#define MSM_ION_HEAP_NUM	8
#endif
#define MSM_ION_MM_FW_SIZE	(0x200000 - HOLE_SIZE) /* (2MB - 128KB) */
#define MSM_ION_MFC_SIZE	SZ_8K
#define MSM_ION_AUDIO_SIZE	MSM_PMEM_AUDIO_SIZE
#else
#define MSM_CONTIG_MEM_SIZE  0x110C000
#define MSM_ION_HEAP_NUM	1
#endif

#define APQ8064_FIXED_AREA_START (0xa0000000 - (MSM_ION_MM_FW_SIZE + \
							HOLE_SIZE))
#define MAX_FIXED_AREA_SIZE	0x10000000
#define MSM_MM_FW_SIZE		(0x200000 - HOLE_SIZE)
#define APQ8064_FW_START	APQ8064_FIXED_AREA_START

#define QFPROM_RAW_FEAT_CONFIG_ROW0_MSB     (MSM_QFPROM_BASE + 0x23c)
#define QFPROM_RAW_OEM_CONFIG_ROW0_LSB      (MSM_QFPROM_BASE + 0x220)

/* PCIE AXI address space */
#define PCIE_AXI_BAR_PHYS   0x08000000
#define PCIE_AXI_BAR_SIZE   SZ_128M

/* PCIe pmic gpios */
#define PCIE_WAKE_N_PMIC_GPIO 12
#define PCIE_PWR_EN_PMIC_GPIO 13
#define PCIE_RST_N_PMIC_MPP 1

#ifdef CONFIG_KERNEL_MSM_CONTIG_MEM_REGION
static unsigned msm_contig_mem_size = MSM_CONTIG_MEM_SIZE;
static int __init msm_contig_mem_size_setup(char *p)
{
	msm_contig_mem_size = memparse(p, NULL);
	return 0;
}
early_param("msm_contig_mem_size", msm_contig_mem_size_setup);
#endif

#if defined(CONFIG_SKY_SND_EXTERNAL_AMP) //YDA165
#if ( ( defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD) ) && (CONFIG_BOARD_VER > CONFIG_WS10) ) || \
	defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)
#include <linux/i2c-gpio.h>
#define GPIO_PIN_SUBSYSTEM_SCL	70
#define GPIO_PIN_SUBSYSTEM_SDA	71

static struct i2c_gpio_platform_data yda165_i2c_gpio_data = {
	.scl_pin = GPIO_PIN_SUBSYSTEM_SCL,
	.sda_pin = GPIO_PIN_SUBSYSTEM_SDA,
	.udelay = 5,	/* 100 KHz */
};

struct platform_device yda165_i2c_gpio_device = {
	.name = "i2c-gpio",
	.id = APQ_8960_GSBI7_QUP_I2C_BUS_ID,
	.dev = {
       .platform_data = &yda165_i2c_gpio_data,
	}
};

static struct i2c_board_info __initdata yda165_i2c_boardinfo[] ={
	{
		I2C_BOARD_INFO("yda165-amp", 0x6C),
	},
};
#else
#include <linux/i2c-gpio.h>
#define I2C_SCL_FAB2210      70  
#define I2C_SDA_FAB2210     71  
static struct i2c_gpio_platform_data fab2210_i2c_gpio_data = {
	.scl_pin = I2C_SCL_FAB2210,
	.sda_pin = I2C_SDA_FAB2210,
	.udelay = 5,
	//.udelay = 2,  // 20110908 jmlee 100KHz(udelay 5) --> 400KHz(udelay 2) boot time issue
};
struct platform_device  fab2210_i2c_gpio_device = {
	.name = "i2c-gpio",
	.id = APQ_8960_GSBI7_QUP_I2C_BUS_ID,
	.dev	= {
		.platform_data = &fab2210_i2c_gpio_data,
	},
};

static struct i2c_board_info __initdata fab2210_i2c_boardinfo[] ={
	{
			I2C_BOARD_INFO("fab2210-i2c", 0x4D),
	},
};
#endif
#endif /* CONFIG_SKY_SND_EXTERNAL_AMP */


#ifdef CONFIG_KERNEL_PMEM_EBI_REGION
static unsigned pmem_kernel_ebi1_size = MSM_PMEM_KERNEL_EBI1_SIZE;
static int __init pmem_kernel_ebi1_size_setup(char *p)
{
	msm_contig_mem_size = memparse(p, NULL);
	return 0;
}
early_param("msm_contig_mem_size", msm_contig_mem_size_setup);
#endif

#if defined(CONFIG_TOUCHSCREEN_QT602240)
static struct i2c_board_info __initdata qt602240_i2c_boardinfo[] ={
	{
			I2C_BOARD_INFO("qt602240-i2c", 0x4A),
	},
};
#endif 

#if defined(CONFIG_PANTECH_MAX17058_FG)
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define GPIO_FG_SCL      1    
#define GPIO_FG_SDA      0
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define GPIO_FG_SCL      1    
#define GPIO_FG_SDA      37
#else
#define GPIO_FG_SCL      1    
#define GPIO_FG_SDA      0
#endif 
static struct i2c_gpio_platform_data max17058_i2c_data = {
        .scl_pin = GPIO_FG_SCL,
        .sda_pin = GPIO_FG_SDA,
        .udelay = 5,    /* 100 KHz */
};

struct platform_device max17058_i2c_device = {
        .name = "i2c-gpio",
        .id = APQ_8064_MAX17058_I2C_BUS_ID,
        .dev = {
                .platform_data = &max17058_i2c_data,
        }
};

static struct i2c_board_info max17058_i2c_boardinfo[] __initdata={
	{
			I2C_BOARD_INFO("max17058-i2c", 0x6C>>1),
	},
};
#endif

#if defined(CONFIG_PANTECH_SMB347_CHARGER)
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define GPIO_SC_SCL      69    
#define GPIO_SC_SDA      26
#elif defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
#define GPIO_SC_SCL      54    
#define GPIO_SC_SDA      53
#else
#define GPIO_SC_SCL      69    
#define GPIO_SC_SDA      26
#endif 
static struct i2c_gpio_platform_data i2c_gpio_smb_data = {
        .scl_pin = GPIO_SC_SCL,
        .sda_pin = GPIO_SC_SDA,
        .udelay = 5,    /* 100 KHz */
};

struct platform_device msm_device_i2c_gpio_smb = {
        .name = "i2c-gpio",
        .id = APQ_8064_SMB347_I2C_BUS_ID,
        .dev = {
                .platform_data = &i2c_gpio_smb_data,
        }
};

#if (defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)) && (CONFIG_BOARD_VER < CONFIG_WS20)
static struct i2c_board_info smb347_i2c_boardinfo[] __initdata={
	{
			I2C_BOARD_INFO("smb347-i2c", 0xD4>>1),
	},
};
#else
static struct i2c_board_info smb347_i2c_boardinfo[] __initdata={
	{
			I2C_BOARD_INFO("smb347-i2c", 0x0C>>1),
	},
};
#endif 
#endif 

#ifdef CONFIG_ANDROID_PMEM
static unsigned pmem_size = MSM_PMEM_SIZE;
static int __init pmem_size_setup(char *p)
{
	pmem_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_size", pmem_size_setup);

static unsigned pmem_adsp_size = MSM_PMEM_ADSP_SIZE;

static int __init pmem_adsp_size_setup(char *p)
{
	pmem_adsp_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_adsp_size", pmem_adsp_size_setup);

static unsigned pmem_audio_size = MSM_PMEM_AUDIO_SIZE;

static int __init pmem_audio_size_setup(char *p)
{
	pmem_audio_size = memparse(p, NULL);
	return 0;
}
early_param("pmem_audio_size", pmem_audio_size_setup);
#endif

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.allocator_type = PMEM_ALLOCATORTYPE_ALLORNOTHING,
	.cached = 1,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = {.platform_data = &android_pmem_pdata},
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};
static struct platform_device apq8064_android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

static struct android_pmem_platform_data android_pmem_audio_pdata = {
	.name = "pmem_audio",
	.allocator_type = PMEM_ALLOCATORTYPE_BITMAP,
	.cached = 0,
	.memory_type = MEMTYPE_EBI1,
};

static struct platform_device apq8064_android_pmem_audio_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_audio_pdata },
};
#endif /* CONFIG_MSM_MULTIMEDIA_USE_ION */
#endif /* CONFIG_ANDROID_PMEM */

#ifdef CONFIG_BATTERY_BCL
static struct platform_device battery_bcl_device = {
	.name = "battery_current_limit",
	.id = -1,
};
#endif

struct fmem_platform_data apq8064_fmem_pdata = {
};

static struct memtype_reserve apq8064_reserve_table[] __initdata = {
	[MEMTYPE_SMI] = {
	},
	[MEMTYPE_EBI0] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
	[MEMTYPE_EBI1] = {
		.flags	=	MEMTYPE_FLAGS_1M_ALIGN,
	},
};

static void __init reserve_rtb_memory(void)
{
#if defined(CONFIG_MSM_RTB)
	apq8064_reserve_table[MEMTYPE_EBI1].size += apq8064_rtb_pdata.size;
#endif
}


static void __init size_pmem_devices(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	android_pmem_adsp_pdata.size = pmem_adsp_size;
	android_pmem_pdata.size = pmem_size;
	android_pmem_audio_pdata.size = MSM_PMEM_AUDIO_SIZE;
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
}

#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
static void __init reserve_memory_for(struct android_pmem_platform_data *p)
{
	apq8064_reserve_table[p->memory_type].size += p->size;
}
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/

static void __init reserve_pmem_memory(void)
{
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	reserve_memory_for(&android_pmem_adsp_pdata);
	reserve_memory_for(&android_pmem_pdata);
	reserve_memory_for(&android_pmem_audio_pdata);
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
	apq8064_reserve_table[MEMTYPE_EBI1].size += msm_contig_mem_size;
#endif /*CONFIG_ANDROID_PMEM*/
}

static int apq8064_paddr_to_memtype(unsigned int paddr)
{
	return MEMTYPE_EBI1;
}

#define FMEM_ENABLED 0

#ifdef CONFIG_ION_MSM
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
static struct ion_cp_heap_pdata cp_mm_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MM_CARVEOUT,
	.align = PAGE_SIZE,
	.reusable = FMEM_ENABLED,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_MIDDLE,
};

static struct ion_cp_heap_pdata cp_mfc_apq8064_ion_pdata = {
	.permission_type = IPT_TYPE_MFC_SHAREDMEM,
	.align = PAGE_SIZE,
	.reusable = 0,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_HIGH,
};

static struct ion_co_heap_pdata co_apq8064_ion_pdata = {
	.adjacent_mem_id = INVALID_HEAP_ID,
	.align = PAGE_SIZE,
	.mem_is_fmem = 0,
};

static struct ion_co_heap_pdata fw_co_apq8064_ion_pdata = {
	.adjacent_mem_id = ION_CP_MM_HEAP_ID,
	.align = SZ_128K,
	.mem_is_fmem = FMEM_ENABLED,
	.fixed_position = FIXED_LOW,
};
#endif

/**
 * These heaps are listed in the order they will be allocated. Due to
 * video hardware restrictions and content protection the FW heap has to
 * be allocated adjacent (below) the MM heap and the MFC heap has to be
 * allocated after the MM heap to ensure MFC heap is not more than 256MB
 * away from the base address of the FW heap.
 * However, the order of FW heap and MM heap doesn't matter since these
 * two heaps are taken care of by separate code to ensure they are adjacent
 * to each other.
 * Don't swap the order unless you know what you are doing!
 */
static struct ion_platform_data apq8064_ion_pdata = {
	.nr = MSM_ION_HEAP_NUM,
	.heaps = {
		{
			.id	= ION_SYSTEM_HEAP_ID,
			.type	= ION_HEAP_TYPE_SYSTEM,
			.name	= ION_VMALLOC_HEAP_NAME,
		},
#ifdef CONFIG_MSM_MULTIMEDIA_USE_ION
		{
			.id	= ION_CP_MM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MM_HEAP_NAME,
			.size	= MSM_ION_MM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mm_apq8064_ion_pdata,
		},
		{
			.id	= ION_MM_FIRMWARE_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_MM_FIRMWARE_HEAP_NAME,
			.size	= MSM_ION_MM_FW_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &fw_co_apq8064_ion_pdata,
		},
		{
			.id	= ION_CP_MFC_HEAP_ID,
			.type	= ION_HEAP_TYPE_CP,
			.name	= ION_MFC_HEAP_NAME,
			.size	= MSM_ION_MFC_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &cp_mfc_apq8064_ion_pdata,
		},
#ifndef CONFIG_MSM_IOMMU
		{
			.id	= ION_SF_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_SF_HEAP_NAME,
			.size	= MSM_ION_SF_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
#endif
		{
			.id	= ION_IOMMU_HEAP_ID,
			.type	= ION_HEAP_TYPE_IOMMU,
			.name	= ION_IOMMU_HEAP_NAME,
		},
		{
			.id	= ION_QSECOM_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_QSECOM_HEAP_NAME,
			.size	= MSM_ION_QSECOM_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
		{
			.id	= ION_AUDIO_HEAP_ID,
			.type	= ION_HEAP_TYPE_CARVEOUT,
			.name	= ION_AUDIO_HEAP_NAME,
			.size	= MSM_ION_AUDIO_SIZE,
			.memory_type = ION_EBI_TYPE,
			.extra_data = (void *) &co_apq8064_ion_pdata,
		},
#endif
	}
};

static struct platform_device apq8064_ion_dev = {
	.name = "ion-msm",
	.id = 1,
	.dev = { .platform_data = &apq8064_ion_pdata },
};
#endif

static struct platform_device apq8064_fmem_device = {
	.name = "fmem",
	.id = 1,
	.dev = { .platform_data = &apq8064_fmem_pdata },
};

static void __init reserve_mem_for_ion(enum ion_memory_types mem_type,
				      unsigned long size)
{
	apq8064_reserve_table[mem_type].size += size;
}

static void __init apq8064_reserve_fixed_area(unsigned long fixed_area_size)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	int ret;

	if (fixed_area_size > MAX_FIXED_AREA_SIZE)
		panic("fixed area size is larger than %dM\n",
			MAX_FIXED_AREA_SIZE >> 20);

	reserve_info->fixed_area_size = fixed_area_size;
	reserve_info->fixed_area_start = APQ8064_FW_START;

	ret = memblock_remove(reserve_info->fixed_area_start,
		reserve_info->fixed_area_size);
	BUG_ON(ret);
#endif
}

/**
 * Reserve memory for ION and calculate amount of reusable memory for fmem.
 * We only reserve memory for heaps that are not reusable. However, we only
 * support one reusable heap at the moment so we ignore the reusable flag for
 * other than the first heap with reusable flag set. Also handle special case
 * for video heaps (MM,FW, and MFC). Video requires heaps MM and MFC to be
 * at a higher address than FW in addition to not more than 256MB away from the
 * base address of the firmware. This means that if MM is reusable the other
 * two heaps must be allocated in the same region as FW. This is handled by the
 * mem_is_fmem flag in the platform data. In addition the MM heap must be
 * adjacent to the FW heap for content protection purposes.
 */
static void __init reserve_ion_memory(void)
{
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
	unsigned int i;
	unsigned int reusable_count = 0;
	unsigned int fixed_size = 0;
	unsigned int fixed_low_size, fixed_middle_size, fixed_high_size;
	unsigned long fixed_low_start, fixed_middle_start, fixed_high_start;

	apq8064_fmem_pdata.size = 0;
	apq8064_fmem_pdata.reserved_size_low = 0;
	apq8064_fmem_pdata.reserved_size_high = 0;
	apq8064_fmem_pdata.align = PAGE_SIZE;
	fixed_low_size = 0;
	fixed_middle_size = 0;
	fixed_high_size = 0;

	/* We only support 1 reusable heap. Check if more than one heap
	 * is specified as reusable and set as non-reusable if found.
	 */
	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		const struct ion_platform_heap *heap =
			&(apq8064_ion_pdata.heaps[i]);

		if (heap->type == ION_HEAP_TYPE_CP && heap->extra_data) {
			struct ion_cp_heap_pdata *data = heap->extra_data;

			reusable_count += (data->reusable) ? 1 : 0;

			if (data->reusable && reusable_count > 1) {
				pr_err("%s: Too many heaps specified as "
					"reusable. Heap %s was not configured "
					"as reusable.\n", __func__, heap->name);
				data->reusable = 0;
			}
		}
	}

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		const struct ion_platform_heap *heap =
			&(apq8064_ion_pdata.heaps[i]);

		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;
			int mem_is_fmem = 0;

			switch (heap->type) {
			case ION_HEAP_TYPE_CP:
				mem_is_fmem = ((struct ion_cp_heap_pdata *)
					heap->extra_data)->mem_is_fmem;
				fixed_position = ((struct ion_cp_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			case ION_HEAP_TYPE_CARVEOUT:
				mem_is_fmem = ((struct ion_co_heap_pdata *)
					heap->extra_data)->mem_is_fmem;
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			if (fixed_position != NOT_FIXED)
				fixed_size += heap->size;
			else
				reserve_mem_for_ion(MEMTYPE_EBI1, heap->size);

			if (fixed_position == FIXED_LOW)
				fixed_low_size += heap->size;
			else if (fixed_position == FIXED_MIDDLE)
				fixed_middle_size += heap->size;
			else if (fixed_position == FIXED_HIGH)
				fixed_high_size += heap->size;

			if (mem_is_fmem)
				apq8064_fmem_pdata.size += heap->size;
		}
	}

	if (!fixed_size)
		return;

	if (apq8064_fmem_pdata.size) {
		apq8064_fmem_pdata.reserved_size_low = fixed_low_size +
								HOLE_SIZE;
		apq8064_fmem_pdata.reserved_size_high = fixed_high_size;
	}

	/* Since the fixed area may be carved out of lowmem,
	 * make sure the length is a multiple of 1M.
	 */
	fixed_size = (fixed_size + HOLE_SIZE + SECTION_SIZE - 1)
		& SECTION_MASK;
	apq8064_reserve_fixed_area(fixed_size);

	fixed_low_start = APQ8064_FIXED_AREA_START;
	fixed_middle_start = fixed_low_start + fixed_low_size + HOLE_SIZE;
	fixed_high_start = fixed_middle_start + fixed_middle_size;

	for (i = 0; i < apq8064_ion_pdata.nr; ++i) {
		struct ion_platform_heap *heap = &(apq8064_ion_pdata.heaps[i]);

		if (heap->extra_data) {
			int fixed_position = NOT_FIXED;
			struct ion_cp_heap_pdata *pdata = NULL;

			switch (heap->type) {
			case ION_HEAP_TYPE_CP:
				pdata =
				(struct ion_cp_heap_pdata *)heap->extra_data;
				fixed_position = pdata->fixed_position;
				break;
			case ION_HEAP_TYPE_CARVEOUT:
				fixed_position = ((struct ion_co_heap_pdata *)
					heap->extra_data)->fixed_position;
				break;
			default:
				break;
			}

			switch (fixed_position) {
			case FIXED_LOW:
				heap->base = fixed_low_start;
				break;
			case FIXED_MIDDLE:
				heap->base = fixed_middle_start;
				pdata->secure_base = fixed_middle_start
								- HOLE_SIZE;
				pdata->secure_size = HOLE_SIZE + heap->size;
				break;
			case FIXED_HIGH:
				heap->base = fixed_high_start;
				break;
			default:
				break;
			}
		}
	}
#endif
}

static void __init reserve_mdp_memory(void)
{
	apq8064_mdp_writeback(apq8064_reserve_table);
}

static void __init reserve_cache_dump_memory(void)
{
#ifdef CONFIG_MSM_CACHE_DUMP
	unsigned int total;

	total = apq8064_cache_dump_pdata.l1_size +
		apq8064_cache_dump_pdata.l2_size;
	apq8064_reserve_table[MEMTYPE_EBI1].size += total;
#endif
}

static void __init apq8064_calculate_reserve_sizes(void)
{
	size_pmem_devices();
	reserve_pmem_memory();
	reserve_ion_memory();
	reserve_mdp_memory();
	reserve_rtb_memory();
	reserve_cache_dump_memory();
#if defined(CONFIG_QC_ABNORMAL_DEBUG_CODE) && !defined(CONFIG_PANTECH_USER_BUILD)
	//#define ALRANLOGSIZE (1 << CONFIG_LOG_BUF_SHIFT)
	#define ALRANLOGSIZE (1048576)
	apq8064_reserve_table[MEMTYPE_EBI1].size += (ALRANLOGSIZE + PAGE_SIZE);
#endif
}

static struct reserve_info apq8064_reserve_info __initdata = {
	.memtype_reserve_table = apq8064_reserve_table,
	.calculate_reserve_sizes = apq8064_calculate_reserve_sizes,
	.reserve_fixed_area = apq8064_reserve_fixed_area,
	.paddr_to_memtype = apq8064_paddr_to_memtype,
};

static int apq8064_memory_bank_size(void)
{
	return 1<<29;
}

static void __init locate_unstable_memory(void)
{
	struct membank *mb = &meminfo.bank[meminfo.nr_banks - 1];
	unsigned long bank_size;
	unsigned long low, high;

	bank_size = apq8064_memory_bank_size();
	low = meminfo.bank[0].start;
	high = mb->start + mb->size;

	/* Check if 32 bit overflow occured */
	if (high < mb->start)
		high = -PAGE_SIZE;

	low &= ~(bank_size - 1);

	if (high - low <= bank_size)
		goto no_dmm;

#ifdef CONFIG_ENABLE_DMM
	apq8064_reserve_info.low_unstable_address = mb->start -
					MIN_MEMORY_BLOCK_SIZE + mb->size;
	apq8064_reserve_info.max_unstable_size = MIN_MEMORY_BLOCK_SIZE;

	apq8064_reserve_info.bank_size = bank_size;
	pr_info("low unstable address %lx max size %lx bank size %lx\n",
		apq8064_reserve_info.low_unstable_address,
		apq8064_reserve_info.max_unstable_size,
		apq8064_reserve_info.bank_size);
	return;
#endif
no_dmm:
	apq8064_reserve_info.low_unstable_address = high;
	apq8064_reserve_info.max_unstable_size = 0;
}

static int apq8064_change_memory_power(u64 start, u64 size,
	int change_type)
{
	return soc_change_memory_power(start, size, change_type);
}

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static char ext_panel_name[PANEL_NAME_MAX_LEN];

static int ext_resolution;

static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

static int __init ext_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(ext_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("ext_display", ext_display_setup);

static int __init hdmi_resulution_setup(char *param)
{
	int ret;
	ret = kstrtoint(param, 10, &ext_resolution);
	return ret;
}
early_param("ext_resolution", hdmi_resulution_setup);

static void __init apq8064_reserve(void)
{
	apq8064_set_display_params(prim_panel_name, ext_panel_name,
		ext_resolution);
	msm_reserve();
	if (apq8064_fmem_pdata.size) {
#if defined(CONFIG_ION_MSM) && defined(CONFIG_MSM_MULTIMEDIA_USE_ION)
		if (reserve_info->fixed_area_size) {
			apq8064_fmem_pdata.phys =
				reserve_info->fixed_area_start + MSM_MM_FW_SIZE;
			pr_info("mm fw at %lx (fixed) size %x\n",
				reserve_info->fixed_area_start, MSM_MM_FW_SIZE);
			pr_info("fmem start %lx (fixed) size %lx\n",
				apq8064_fmem_pdata.phys,
				apq8064_fmem_pdata.size);
		}
#endif
	}
}

static void __init place_movable_zone(void)
{
#ifdef CONFIG_ENABLE_DMM
	movable_reserved_start = apq8064_reserve_info.low_unstable_address;
	movable_reserved_size = apq8064_reserve_info.max_unstable_size;
	pr_info("movable zone start %lx size %lx\n",
		movable_reserved_start, movable_reserved_size);
#endif
}

static void __init apq8064_early_reserve(void)
{
	reserve_info = &apq8064_reserve_info;
	locate_unstable_memory();
	place_movable_zone();

}
#ifdef CONFIG_USB_EHCI_MSM_HSIC
/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors hsic_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors hsic_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 60000000,		/* At least 480Mbps on bus. */
		.ib = 960000000,	/* MAX bursts rate */
	},
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_SPS,
		.ab = 0,
		.ib = 512000000, /*vote for 64Mhz dfab clk rate*/
	},
};

static struct msm_bus_paths hsic_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(hsic_init_vectors),
		hsic_init_vectors,
	},
	{
		ARRAY_SIZE(hsic_max_vectors),
		hsic_max_vectors,
	},
};

static struct msm_bus_scale_pdata hsic_bus_scale_pdata = {
	hsic_bus_scale_usecases,
	ARRAY_SIZE(hsic_bus_scale_usecases),
	.name = "hsic",
};

static struct msm_hsic_host_platform_data msm_hsic_pdata = {
	.strobe			= 88,
	.data			= 89,
	.bus_scale_table	= &hsic_bus_scale_pdata,
};
#else
static struct msm_hsic_host_platform_data msm_hsic_pdata;
#endif

#define PID_MAGIC_ID		0x71432909
#define SERIAL_NUM_MAGIC_ID	0x61945374
#define SERIAL_NUMBER_LENGTH	127
#define DLOAD_USB_BASE_ADD	0x2A03F0C8

struct magic_num_struct {
	uint32_t pid;
	uint32_t serial_num;
};

struct dload_struct {
	uint32_t	reserved1;
	uint32_t	reserved2;
	uint32_t	reserved3;
	uint16_t	reserved4;
	uint16_t	pid;
	char		serial_number[SERIAL_NUMBER_LENGTH];
	uint16_t	reserved5;
	struct magic_num_struct magic_struct;
};

static int usb_diag_update_pid_and_serial_num(uint32_t pid, const char *snum)
{
	struct dload_struct __iomem *dload = 0;

	dload = ioremap(DLOAD_USB_BASE_ADD, sizeof(*dload));
	if (!dload) {
		pr_err("%s: cannot remap I/O memory region: %08x\n",
					__func__, DLOAD_USB_BASE_ADD);
		return -ENXIO;
	}

	pr_debug("%s: dload:%p pid:%x serial_num:%s\n",
				__func__, dload, pid, snum);
	/* update pid */
	dload->magic_struct.pid = PID_MAGIC_ID;
	dload->pid = pid;

	/* update serial number */
	dload->magic_struct.serial_num = 0;
	if (!snum) {
		memset(dload->serial_number, 0, SERIAL_NUMBER_LENGTH);
		goto out;
	}

	dload->magic_struct.serial_num = SERIAL_NUM_MAGIC_ID;
	strlcpy(dload->serial_number, snum, SERIAL_NUMBER_LENGTH);
out:
	iounmap(dload);
	return 0;
}

static struct android_usb_platform_data android_usb_pdata = {
	.update_pid_and_serial_num = usb_diag_update_pid_and_serial_num,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id	= -1,
	.dev	= {
		.platform_data = &android_usb_pdata,
	},
};

/* Bandwidth requests (zero) if no vote placed */
static struct msm_bus_vectors usb_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
};

/* Bus bandwidth requests in Bytes/sec */
static struct msm_bus_vectors usb_max_vectors[] = {
	{
		.src = MSM_BUS_MASTER_SPS,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 60000000,		/* At least 480Mbps on bus. */
		.ib = 960000000,	/* MAX bursts rate */
	},
};

static struct msm_bus_paths usb_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(usb_init_vectors),
		usb_init_vectors,
	},
	{
		ARRAY_SIZE(usb_max_vectors),
		usb_max_vectors,
	},
};

static struct msm_bus_scale_pdata usb_bus_scale_pdata = {
	usb_bus_scale_usecases,
	ARRAY_SIZE(usb_bus_scale_usecases),
	.name = "usb",
};

static int phy_init_seq[] = {
	0x68, 0x81, /* update DC voltage level */
	0x24, 0x82, /* set pre-emphasis and rise/fall time */
	-1
};

#define PMIC_GPIO_DP		27    /* PMIC GPIO for D+ change */
#define PMIC_GPIO_DP_IRQ	PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PMIC_GPIO_DP)
#define MSM_MPM_PIN_USB1_OTGSESSVLD	40

#ifdef CONFIG_PANTECH_ANDROID_OTG
static int pantech_control_usb_switch(int pm_gpio, int value)
{
	static int gpio;
	int rc;

	

	gpio = PM8921_GPIO_PM_TO_SYS(pm_gpio);

	printk(KERN_ERR "@@@@ %s: pm_gpio[%d], value[%d], gpio[%d]\n", __func__, pm_gpio, value, gpio);
	if(pm_gpio == 36){
		rc = gpio_request(gpio, "msm_usb_id_sw");
		if(rc){
				pr_err("request gpio 36 failed, rc=%d\n", rc);
				return -ENODEV;
		}
	}else if(pm_gpio == 44){
		rc = gpio_request(gpio, "pmic_usb_id_sw");
		if(rc){
				pr_err("request gpio  failed, rc=%d\n", rc);
				return -ENODEV;
		}
	}

	if(value){
		gpio_set_value_cansleep(gpio, 1);
	}else{
		gpio_set_value_cansleep(gpio, 0);
	}

	gpio_free(gpio);
	return 0;
}
#endif

static struct msm_otg_platform_data msm_otg_pdata = {
#ifdef CONFIG_PANTECH_ANDROID_OTG
	.mode			= USB_OTG,
	.otg_control		= OTG_PHY_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.pmic_id_irq		= PM8921_GPIO_IRQ(PM8921_IRQ_BASE, 25),
	.control_usb_switch = pantech_control_usb_switch,
	.power_budget		= 750,
        .bus_scale_table        = &usb_bus_scale_pdata,
	.phy_init_seq		= phy_init_seq,
	.mpm_otgsessvld_int	= MSM_MPM_PIN_USB1_OTGSESSVLD,
#else
	.mode			= USB_OTG,
	.otg_control		= OTG_PMIC_CONTROL,
	.phy_type		= SNPS_28NM_INTEGRATED_PHY,
	.pmic_id_irq		= PM8921_USB_ID_IN_IRQ(PM8921_IRQ_BASE),
	.power_budget		= 750,
	.bus_scale_table	= &usb_bus_scale_pdata,
	.phy_init_seq		= phy_init_seq,
	.mpm_otgsessvld_int	= MSM_MPM_PIN_USB1_OTGSESSVLD,
#endif

};

static struct msm_usb_host_platform_data msm_ehci_host_pdata3 = {
	.power_budget = 500,
};

#ifdef CONFIG_USB_EHCI_MSM_HOST4
static struct msm_usb_host_platform_data msm_ehci_host_pdata4;
#endif

static void __init apq8064_ehci_host_init(void)
{
	if (machine_is_apq8064_liquid() || machine_is_mpq8064_cdp() ||
		machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
		if (machine_is_apq8064_liquid())
			msm_ehci_host_pdata3.dock_connect_irq =
					PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9);
		else
			msm_ehci_host_pdata3.pmic_gpio_dp_irq =
							PMIC_GPIO_DP_IRQ;

		apq8064_device_ehci_host3.dev.platform_data =
				&msm_ehci_host_pdata3;
		platform_device_register(&apq8064_device_ehci_host3);

#ifdef CONFIG_USB_EHCI_MSM_HOST4
		apq8064_device_ehci_host4.dev.platform_data =
				&msm_ehci_host_pdata4;
		platform_device_register(&apq8064_device_ehci_host4);
#endif
	}
}

#if !defined(CONFIG_MACH_APQ8064_EF48S) && !defined(CONFIG_MACH_APQ8064_EF49K) && !defined(CONFIG_MACH_APQ8064_EF50L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K) && !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L)
#error
static struct smb349_platform_data smb349_data __initdata = {
	.en_n_gpio		= PM8921_GPIO_PM_TO_SYS(37),
	.chg_susp_gpio		= PM8921_GPIO_PM_TO_SYS(30),
	.chg_current_ma		= 2200,
};

static struct i2c_board_info smb349_charger_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(SMB349_NAME, 0x1B),
		.platform_data	= &smb349_data,
	},
};
#endif

struct sx150x_platform_data apq8064_sx150x_data[] = {
	[SX150X_EPM] = {
		.gpio_base	= GPIO_EPM_EXPANDER_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
};

static struct epm_chan_properties ads_adc_channel_data[] = {
	{10, 100}, {1000, 1}, {10, 100}, {1000, 1},
	{10, 100}, {1000, 1}, {10, 100}, {1000, 1},
	{10, 100}, {20, 100}, {500, 100}, {5, 100},
	{1000, 1}, {200, 100}, {50, 100}, {10, 100},
	{510, 100}, {50, 100}, {20, 100}, {100, 100},
	{510, 100}, {20, 100}, {50, 100}, {200, 100},
	{10, 100}, {20, 100}, {1000, 1}, {10, 100},
	{200, 100}, {510, 100}, {1000, 100}, {200, 100},
};

static struct epm_adc_platform_data epm_adc_pdata = {
	.channel		= ads_adc_channel_data,
	.bus_id	= 0x0,
	.epm_i2c_board_info = {
		.type	= "sx1509q",
		.addr = 0x3e,
		.platform_data = &apq8064_sx150x_data[SX150X_EPM],
	},
	.gpio_expander_base_addr = GPIO_EPM_EXPANDER_BASE,
};

static struct platform_device epm_adc_device = {
	.name   = "epm_adc",
	.id = -1,
	.dev = {
		.platform_data = &epm_adc_pdata,
	},
};

static void __init apq8064_epm_adc_init(void)
{
	epm_adc_pdata.num_channels = 32;
	epm_adc_pdata.num_adc = 2;
	epm_adc_pdata.chan_per_adc = 16;
	epm_adc_pdata.chan_per_mux = 8;
};

/* Micbias setting is based on 8660 CDP/MTP/FLUID requirement
 * 4 micbiases are used to power various analog and digital
 * microphones operating at 1800 mV. Technically, all micbiases
 * can source from single cfilter since all microphones operate
 * at the same voltage level. The arrangement below is to make
 * sure all cfilters are exercised. LDO_H regulator ouput level
 * does not need to be as high as 2.85V. It is choosen for
 * microphone sensitivity purpose.
 */
static struct wcd9xxx_pdata apq8064_tabla_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x10, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla = {
	.name = "tabla-slim",
	.e_addr = {0, 1, 0x10, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla_platform_data,
	},
};

static struct wcd9xxx_pdata apq8064_tabla20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 2700,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device apq8064_slim_tabla20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &apq8064_tabla20_platform_data,
	},
};

static struct wcd9xxx_pdata apq8064_tabla_i2c_platform_data = {
	.irq = MSM_GPIO_TO_INT(77),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "VDDD_CDC_D",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_A_1P2V",
		.min_uV = 1225000,
		.max_uV = 1250000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct i2c_board_info apq8064_tabla_i2c_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("tabla top level",
				APQ_8064_TABLA_I2C_SLAVE_ADDR),
		.platform_data = &apq8064_tabla_i2c_platform_data,
	},
	{
		I2C_BOARD_INFO("tabla analog",
				APQ_8064_TABLA_ANALOG_I2C_SLAVE_ADDR),
		.platform_data = &apq8064_tabla_i2c_platform_data,
	},
	{
		I2C_BOARD_INFO("tabla digital1",
				APQ_8064_TABLA_DIGITAL1_I2C_SLAVE_ADDR),
		.platform_data = &apq8064_tabla_i2c_platform_data,
	},
	{
		I2C_BOARD_INFO("tabla digital2",
				APQ_8064_TABLA_DIGITAL2_I2C_SLAVE_ADDR),
		.platform_data = &apq8064_tabla_i2c_platform_data,
	},
};

static struct wcd9xxx_pdata mpq8064_ashiko20_platform_data = {
	.slimbus_slave_device = {
		.name = "tabla-slave",
		.e_addr = {0, 0, 0x60, 0, 0x17, 2},
	},
	.irq = MSM_GPIO_TO_INT(42),
	.irq_base = TABLA_INTERRUPT_BASE,
	.num_irqs = NR_WCD9XXX_IRQS,
	.reset_gpio = PM8921_GPIO_PM_TO_SYS(34),
	.micbias = {
		.ldoh_v = TABLA_LDOH_2P85_V,
		.cfilt1_mv = 1800,
		.cfilt2_mv = 1800,
		.cfilt3_mv = 1800,
		.bias1_cfilt_sel = TABLA_CFILT1_SEL,
		.bias2_cfilt_sel = TABLA_CFILT2_SEL,
		.bias3_cfilt_sel = TABLA_CFILT3_SEL,
		.bias4_cfilt_sel = TABLA_CFILT3_SEL,
	},
	.regulator = {
	{
		.name = "CDC_VDD_CP",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_CP_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_RX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_RX_CUR_MAX,
	},
	{
		.name = "CDC_VDDA_TX",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_CDC_VDDA_TX_CUR_MAX,
	},
	{
		.name = "VDDIO_CDC",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.optimum_uA = WCD9XXX_VDDIO_CDC_CUR_MAX,
	},
	{
		.name = "HRD_VDDD_CDC_D",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.optimum_uA = WCD9XXX_VDDD_CDC_D_CUR_MAX,
	},
	{
		.name = "HRD_CDC_VDDA_A_1P2V",
		.min_uV = 1200000,
		.max_uV = 1200000,
		.optimum_uA = WCD9XXX_VDDD_CDC_A_CUR_MAX,
	},
	},
};

static struct slim_device mpq8064_slim_ashiko20 = {
	.name = "tabla2x-slim",
	.e_addr = {0, 1, 0x60, 0, 0x17, 2},
	.dev = {
		.platform_data = &mpq8064_ashiko20_platform_data,
	},
};


/* enable the level shifter for cs8427 to make sure the I2C
 * clock is running at 100KHz and voltage levels are at 3.3
 * and 5 volts
 */
static int enable_100KHz_ls(int enable)
{
	int ret = 0;
	if (enable) {
		ret = gpio_request(SX150X_GPIO(1, 10),
					"cs8427_100KHZ_ENABLE");
		if (ret) {
			pr_err("%s: Failed to request gpio %d\n", __func__,
				SX150X_GPIO(1, 10));
			return ret;
		}
		gpio_direction_output(SX150X_GPIO(1, 10), 1);
	} else {
		gpio_direction_output(SX150X_GPIO(1, 10), 0);
		gpio_free(SX150X_GPIO(1, 10));
	}
	return ret;
}

static struct cs8427_platform_data cs8427_i2c_platform_data = {
	.irq = SX150X_GPIO(1, 4),
	.reset_gpio = SX150X_GPIO(1, 6),
	.enable = enable_100KHz_ls,
};

static struct i2c_board_info cs8427_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("cs8427", CS8427_ADDR4),
		.platform_data = &cs8427_i2c_platform_data,
	},
};


#ifndef CONFIG_PN544
//#define HAP_SHIFT_LVL_OE_GPIO		PM8921_MPP_PM_TO_SYS(8)
#define HAP_SHIFT_LVL_OE_GPIO 
#define ISA1200_HAP_EN_GPIO		PM8921_GPIO_PM_TO_SYS(33)
#define ISA1200_HAP_LEN_GPIO		PM8921_GPIO_PM_TO_SYS(20)
#define ISA1200_HAP_CLK_PM8921		PM8921_GPIO_PM_TO_SYS(44)
#define ISA1200_HAP_CLK_PM8917		PM8921_GPIO_PM_TO_SYS(38)

static int isa1200_clk_enable(bool on)
{
	unsigned int gpio = ISA1200_HAP_CLK_PM8921;
	int rc = 0;

	if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
		gpio = ISA1200_HAP_CLK_PM8917;

	gpio_set_value_cansleep(gpio, on);

	if (on) {
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_1, true);
		if (rc) {
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
			goto err_gpio_dis;
		}
	} else {
		rc = pm8xxx_aux_clk_control(CLK_MP3_2, XO_DIV_NONE, true);
		if (rc)
			pr_err("%s: unable to write aux clock register(%d)\n",
				__func__, rc);
	}

	return rc;

err_gpio_dis:
	gpio_set_value_cansleep(gpio, !on);
	return rc;
}

static int isa1200_dev_setup(bool enable)
{
	unsigned int gpio = ISA1200_HAP_CLK_PM8921;
	int rc = 0;

	if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
		gpio = ISA1200_HAP_CLK_PM8917;

	if (!enable)
		goto free_gpio;

	rc = gpio_request(gpio, "haptics_clk");
	if (rc) {
		pr_err("%s: unable to request gpio %d config(%d)\n",
			__func__, gpio, rc);
		return rc;
	}

	rc = gpio_direction_output(gpio, 0);
	if (rc) {
		pr_err("%s: unable to set direction\n", __func__);
		goto free_gpio;
	}

	return 0;

free_gpio:
	gpio_free(gpio);
	return rc;
}

static struct isa1200_regulator isa1200_reg_data[] = {
	{
		.name = "vddp",
		.min_uV = ISA_I2C_VTG_MIN_UV,
		.max_uV = ISA_I2C_VTG_MAX_UV,
		.load_uA = ISA_I2C_CURR_UA,
	},
};

static struct isa1200_platform_data isa1200_1_pdata = {
	.name = "vibrator",
	.dev_setup = isa1200_dev_setup,
	.clk_enable = isa1200_clk_enable,
	.need_pwm_clk = true,
	.hap_en_gpio = ISA1200_HAP_EN_GPIO,
	.hap_len_gpio = ISA1200_HAP_LEN_GPIO,
	.max_timeout = 15000,
	.mode_ctrl = PWM_GEN_MODE,
	.pwm_fd = {
		.pwm_div = 256,
	},
	.is_erm = false,
	.smart_en = true,
	.ext_clk_en = true,
	.chip_en = 1,
	.regulator_info = isa1200_reg_data,
	.num_regulators = ARRAY_SIZE(isa1200_reg_data),
};

static struct i2c_board_info isa1200_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("isa1200_1", 0x90>>1),
		.platform_data = &isa1200_1_pdata,
	},
};
#endif

#if !defined(CONFIG_MACH_APQ8064_EF48S) && !defined(CONFIG_MACH_APQ8064_EF49K) && !defined(CONFIG_MACH_APQ8064_EF50L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K) && !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L)
/* configuration data for mxt1386e using V2.1 firmware */
static const u8 mxt1386e_config_data_v2_1[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	14, 3, 0, 5, 7, 12, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T7 Object */
	32, 10, 50,
	/* T8 Object */
	25, 0, 20, 20, 0, 0, 0, 0, 0, 0,
	/* T9 Object */
	139, 0, 0, 26, 42, 0, 32, 80, 2, 5,
	0, 5, 5, 79, 10, 30, 10, 10, 255, 2,
	85, 5, 0, 5, 9, 5, 12, 35, 70, 40,
	20, 5, 0, 0, 0,
	/* T18 Object */
	0, 0,
	/* T24 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* T25 Object */
	1, 0, 60, 115, 156, 99,
	/* T27 Object */
	0, 0, 0, 0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 255, 0, 255, 0, 0, 0, 0, 0,
	/* T43 Object */
	0, 0, 0, 0, 0, 0, 0, 64, 0, 8,
	16,
	/* T46 Object */
	68, 0, 16, 16, 0, 0, 0, 0, 0,
	/* T47 Object */
	0, 0, 0, 0, 0, 0, 3, 64, 66, 0,
	/* T48 Object */
	1, 64, 64, 0, 0, 0, 0, 0, 0, 0,
	32, 40, 0, 10, 10, 0, 0, 100, 10, 90,
	0, 0, 0, 0, 0, 0, 0, 10, 1, 10,
	52, 10, 12, 0, 33, 0, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T56 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
};

#define MXT_TS_GPIO_IRQ			6
#define MXT_TS_PWR_EN_GPIO		PM8921_GPIO_PM_TO_SYS(23)
#define MXT_TS_RESET_GPIO		33

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x21,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
		.fw_name	= "atmel_8064_liquid_v2_2_AA.hex",
	},
	{
		/* The config data for V2.2.AA is the same as for V2.1.AA */
		.config		= mxt1386e_config_data_v2_1,
		.config_length	= ARRAY_SIZE(mxt1386e_config_data_v2_1),
		.family_id	= 0xA0,
		.variant_id	= 0x7,
		.version	= 0x22,
		.build		= 0xAA,
		.bootldr_id	= MXT_BOOTLOADER_ID_1386E,
	},
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 1365,
	.panel_miny		= 0,
	.panel_maxy		= 767,
	.disp_minx		= 0,
	.disp_maxx		= 1365,
	.disp_miny		= 0,
	.disp_maxy		= 767,
	.irqflags		= IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
	.i2c_pull_up		= true,
	.reset_gpio		= MXT_TS_RESET_GPIO,
	.irq_gpio		= MXT_TS_GPIO_IRQ,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0x5b),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_GPIO_IRQ),
	},
};
#define CYTTSP_TS_GPIO_IRQ		6
#define CYTTSP_TS_GPIO_SLEEP		33
#define CYTTSP_TS_GPIO_SLEEP_ALT	12

static ssize_t tma340_vkeys_show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	return snprintf(buf, 200,
	__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":73:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":230:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":389:1120:97:97"
	":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":544:1120:97:97"
	"\n");
}

static struct kobj_attribute tma340_vkeys_attr = {
	.attr = {
		.mode = S_IRUGO,
	},
	.show = &tma340_vkeys_show,
};

static struct attribute *tma340_properties_attrs[] = {
	&tma340_vkeys_attr.attr,
	NULL
};

static struct attribute_group tma340_properties_attr_group = {
	.attrs = tma340_properties_attrs,
};

static int cyttsp_platform_init(struct i2c_client *client)
{
	int rc = 0;
	static struct kobject *tma340_properties_kobj;

	tma340_vkeys_attr.attr.name = "virtualkeys.cyttsp-i2c";
	tma340_properties_kobj = kobject_create_and_add("board_properties",
								NULL);
	if (tma340_properties_kobj)
		rc = sysfs_create_group(tma340_properties_kobj,
					&tma340_properties_attr_group);
	if (!tma340_properties_kobj || rc)
		pr_err("%s: failed to create board_properties\n",
				__func__);

	return 0;
}

static struct cyttsp_regulator cyttsp_regulator_data[] = {
	{
		.name = "vdd",
		.min_uV = CY_TMA300_VTG_MIN_UV,
		.max_uV = CY_TMA300_VTG_MAX_UV,
		.hpm_load_uA = CY_TMA300_CURR_24HZ_UA,
		.lpm_load_uA = CY_TMA300_CURR_24HZ_UA,
	},
	{
		.name = "vcc_i2c",
		.min_uV = CY_I2C_VTG_MIN_UV,
		.max_uV = CY_I2C_VTG_MAX_UV,
		.hpm_load_uA = CY_I2C_CURR_UA,
		.lpm_load_uA = CY_I2C_CURR_UA,
	},
};

static struct cyttsp_platform_data cyttsp_pdata = {
	.panel_maxx = 634,
	.panel_maxy = 1166,
	.disp_maxx = 599,
	.disp_maxy = 1023,
	.disp_minx = 0,
	.disp_miny = 0,
	.flags = 0x01,
	.gen = CY_GEN3,
	.use_st = CY_USE_ST,
	.use_mt = CY_USE_MT,
	.use_hndshk = CY_SEND_HNDSHK,
	.use_trk_id = CY_USE_TRACKING_ID,
	.use_sleep = CY_USE_DEEP_SLEEP_SEL,
	.use_gestures = CY_USE_GESTURES,
	.fw_fname = "cyttsp_8064_mtp.hex",
	/* change act_intrvl to customize the Active power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.act_intrvl = CY_ACT_INTRVL_DFLT,
	/* change tch_tmout to customize the touch timeout for the
	 * Active power state for Operating mode
	 */
	.tch_tmout = CY_TCH_TMOUT_DFLT,
	/* change lp_intrvl to customize the Low Power power state
	 * scanning/processing refresh interval for Operating mode
	 */
	.lp_intrvl = CY_LP_INTRVL_DFLT,
	.sleep_gpio = CYTTSP_TS_GPIO_SLEEP,
	.resout_gpio = -1,
	.irq_gpio = CYTTSP_TS_GPIO_IRQ,
	.regulator_info = cyttsp_regulator_data,
	.num_regulators = ARRAY_SIZE(cyttsp_regulator_data),
	.init = cyttsp_platform_init,
	.correct_fw_ver = 17,
};

static struct i2c_board_info cyttsp_info[] __initdata = {
	{
		I2C_BOARD_INFO(CY_I2C_NAME, 0x24),
		.platform_data = &cyttsp_pdata,
		.irq = MSM_GPIO_TO_INT(CYTTSP_TS_GPIO_IRQ),
	},
};
#endif

#if defined(CONFIG_PN544)

#define NFC_I2C_SDA             20
#define NFC_I2C_SCL             21
#define NFC_IRQ_GPIO            29
#define NFC_ENABLE_GPIO         PM8921_GPIO_PM_TO_SYS(7)
#define NFC_FW_DL_GPIO          PM8921_GPIO_PM_TO_SYS(22)
#define NFC_SLAVE_ADDR		    0x28	

//#define APQ_8064_GSBI1_QUP_I2C_BUS_ID 0

static struct pn544_i2c_platform_data pn544 = {
	.irq_gpio = NFC_IRQ_GPIO,
	.ven_gpio = NFC_ENABLE_GPIO,
	.firm_gpio = NFC_FW_DL_GPIO,
};

static struct i2c_board_info nfc_i2c_boardinfo[] __initdata = {
	{
		I2C_BOARD_INFO("pn544", NFC_SLAVE_ADDR),
        .irq = MSM_GPIO_TO_INT(NFC_IRQ_GPIO),
        .platform_data = &pn544,
	},
};
#endif	


#define MSM_WCNSS_PHYS	0x03000000
#define MSM_WCNSS_SIZE	0x280000

static struct resource resources_wcnss_wlan[] = {
	{
		.start	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.end	= RIVA_APPS_WLAN_RX_DATA_AVAIL_IRQ,
		.name	= "wcnss_wlanrx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.end	= RIVA_APPS_WLAN_DATA_XFER_DONE_IRQ,
		.name	= "wcnss_wlantx_irq",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= MSM_WCNSS_PHYS,
		.end	= MSM_WCNSS_PHYS + MSM_WCNSS_SIZE - 1,
		.name	= "wcnss_mmio",
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= 64,
		.end	= 68,
		.name	= "wcnss_gpios_5wire",
		.flags	= IORESOURCE_IO,
	},
};

static struct qcom_wcnss_opts qcom_wcnss_pdata = {
	.has_48mhz_xo	= 1,
};

static struct platform_device msm_device_wcnss_wlan = {
	.name		= "wcnss_wlan",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(resources_wcnss_wlan),
	.resource	= resources_wcnss_wlan,
	.dev		= {.platform_data = &qcom_wcnss_pdata},
};

static struct platform_device msm_device_iris_fm __devinitdata = {
	.name = "iris_fm",
	.id   = -1,
};

#ifdef CONFIG_QSEECOM
/* qseecom bus scaling */
static struct msm_bus_vectors qseecom_clks_init_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = 0,
		.ab = 0,
	},
};

static struct msm_bus_vectors qseecom_enable_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 0,
		.ib = 0,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_vectors qseecom_enable_dfab_sfpb_vectors[] = {
	{
		.src = MSM_BUS_MASTER_ADM_PORT0,
		.dst = MSM_BUS_SLAVE_EBI_CH0,
		.ab = 70000000UL,
		.ib = 70000000UL,
	},
	{
		.src = MSM_BUS_MASTER_ADM_PORT1,
		.dst = MSM_BUS_SLAVE_GSBI1_UART,
		.ab = 2480000000UL,
		.ib = 2480000000UL,
	},
	{
		.src = MSM_BUS_MASTER_SPDM,
		.dst = MSM_BUS_SLAVE_SPDM,
		.ib = (64 * 8) * 1000000UL,
		.ab = (64 * 8) *  100000UL,
	},
};

static struct msm_bus_paths qseecom_hw_bus_scale_usecases[] = {
	{
		ARRAY_SIZE(qseecom_clks_init_vectors),
		qseecom_clks_init_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_vectors),
		qseecom_enable_dfab_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_sfpb_vectors),
		qseecom_enable_sfpb_vectors,
	},
	{
		ARRAY_SIZE(qseecom_enable_dfab_sfpb_vectors),
		qseecom_enable_dfab_sfpb_vectors,
	},
};

static struct msm_bus_scale_pdata qseecom_bus_pdata = {
	qseecom_hw_bus_scale_usecases,
	ARRAY_SIZE(qseecom_hw_bus_scale_usecases),
	.name = "qsee",
};

static struct platform_device qseecom_device = {
	.name		= "qseecom",
	.id		= 0,
	.dev		= {
		.platform_data = &qseecom_bus_pdata,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

#define QCE_SIZE		0x10000
#define QCE_0_BASE		0x11000000

#define QCE_HW_KEY_SUPPORT	0
#define QCE_SHA_HMAC_SUPPORT	1
#define QCE_SHARE_CE_RESOURCE	3
#define QCE_CE_SHARED		0

static struct resource qcrypto_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource qcedev_resources[] = {
	[0] = {
		.start = QCE_0_BASE,
		.end = QCE_0_BASE + QCE_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name = "crypto_channels",
		.start = DMOV8064_CE_IN_CHAN,
		.end = DMOV8064_CE_OUT_CHAN,
		.flags = IORESOURCE_DMA,
	},
	[2] = {
		.name = "crypto_crci_in",
		.start = DMOV8064_CE_IN_CRCI,
		.end = DMOV8064_CE_IN_CRCI,
		.flags = IORESOURCE_DMA,
	},
	[3] = {
		.name = "crypto_crci_out",
		.start = DMOV8064_CE_OUT_CRCI,
		.end = DMOV8064_CE_OUT_CRCI,
		.flags = IORESOURCE_DMA,
	},
};

#endif

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)

static struct msm_ce_hw_support qcrypto_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcrypto_device = {
	.name		= "qcrypto",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcrypto_resources),
	.resource	= qcrypto_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcrypto_ce_hw_suppport,
	},
};
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)

static struct msm_ce_hw_support qcedev_ce_hw_suppport = {
	.ce_shared = QCE_CE_SHARED,
	.shared_ce_resource = QCE_SHARE_CE_RESOURCE,
	.hw_key_support = QCE_HW_KEY_SUPPORT,
	.sha_hmac = QCE_SHA_HMAC_SUPPORT,
	.bus_scale_table = NULL,
};

static struct platform_device qcedev_device = {
	.name		= "qce",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(qcedev_resources),
	.resource	= qcedev_resources,
	.dev		= {
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &qcedev_ce_hw_suppport,
	},
};
#endif

static struct mdm_vddmin_resource mdm_vddmin_rscs = {
	.rpm_id = MSM_RPM_ID_VDDMIN_GPIO,
	.ap2mdm_vddmin_gpio = 30,
	.modes  = 0x03,
	.drive_strength = 8,
	.mdm2ap_vddmin_gpio = 80,
};

static struct gpiomux_setting mdm2ap_status_gpio_run_cfg = {
	.func = GPIOMUX_FUNC_GPIO,
	.drv = GPIOMUX_DRV_8MA,
	.pull = GPIOMUX_PULL_NONE,
};

static struct mdm_platform_data mdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 2000,
	.early_power_on = 1,
	.sfr_query = 1,
	.send_shdn = 1,
	.vddmin_resource = &mdm_vddmin_rscs,
	.peripheral_platform_device = &apq8064_device_hsic_host,
	.ramdump_timeout_ms = 120000,
	.mdm2ap_status_gpio_run_cfg = &mdm2ap_status_gpio_run_cfg,
};

static struct tsens_platform_data apq_tsens_pdata  = {
		.tsens_factor		= 1000,
		.hw_type		= APQ_8064,
		.tsens_num_sensor	= 11,
		.slope = {1176, 1176, 1154, 1176, 1111,
			1132, 1132, 1199, 1132, 1199, 1132},
};

static struct platform_device msm_tsens_device = {
	.name   = "tsens8960-tm",
	.id = -1,
};

static struct msm_thermal_data msm_thermal_pdata = {
	.sensor_id = 7,
	.poll_ms = 250,
	.limit_temp_degC = 60,
	.temp_hysteresis_degC = 10,
	.freq_step = 2,
};

#define MSM_SHARED_RAM_PHYS 0x80000000
static void __init apq8064_map_io(void)
{
	msm_shared_ram_phys = MSM_SHARED_RAM_PHYS;
	msm_map_apq8064_io();
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
}

static void __init apq8064_init_irq(void)
{
	struct msm_mpm_device_data *data = NULL;

#ifdef CONFIG_MSM_MPM
	data = &apq8064_mpm_dev_data;
#endif

	msm_mpm_irq_extn_init(data);
	gic_init(0, GIC_PPI_START, MSM_QGIC_DIST_BASE,
						(void *)MSM_QGIC_CPU_BASE);
}

static struct msm_mhl_platform_data mhl_platform_data = {
	.irq = MSM_GPIO_TO_INT(MHL_GPIO_INT),
	.gpio_mhl_int = MHL_GPIO_INT,
	.gpio_mhl_reset = MHL_GPIO_RESET,
	.gpio_mhl_power = 0,
	.gpio_hdmi_mhl_mux = 0,
};

static struct i2c_board_info sii_device_info[] __initdata = {
	{
		/*
		 * keeps SI 8334 as the default
		 * MHL TX
		 */
		I2C_BOARD_INFO("sii8334", 0x39),
		.platform_data = &mhl_platform_data,
		.flags = I2C_CLIENT_WAKE,
	},
};

static struct platform_device msm8064_device_saw_regulator_core0 = {
	.name	= "saw-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s5,
	},
};

static struct platform_device msm8064_device_saw_regulator_core1 = {
	.name	= "saw-regulator",
	.id	= 1,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8921_s6,
	},
};

static struct platform_device msm8064_device_saw_regulator_core2 = {
	.name	= "saw-regulator",
	.id	= 2,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s0,
	},
};

static struct platform_device msm8064_device_saw_regulator_core3 = {
	.name	= "saw-regulator",
	.id	= 3,
	.dev	= {
		.platform_data = &msm8064_saw_regulator_pdata_8821_s1,

	},
};

static struct msm_rpmrs_level msm_rpmrs_levels[] = {
	{
		MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1, 784, 180000, 100,
	},

	{
		MSM_PM_SLEEP_MODE_RETENTION,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		415, 715, 340827, 475,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE,
		MSM_RPMRS_LIMITS(ON, ACTIVE, MAX, ACTIVE),
		true,
		1300, 228, 1200000, 2000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, GDHS, MAX, ACTIVE),
		false,
		2000, 138, 1208400, 3200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(ON, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		6000, 119, 1850300, 9000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, GDHS, MAX, ACTIVE),
		false,
		9200, 68, 2839200, 16400,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, MAX, ACTIVE),
		false,
		10300, 63, 3128000, 18200,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, ACTIVE, RET_HIGH),
		false,
		18000, 10, 4602600, 27000,
	},

	{
		MSM_PM_SLEEP_MODE_POWER_COLLAPSE,
		MSM_RPMRS_LIMITS(OFF, HSFS_OPEN, RET_HIGH, RET_LOW),
		false,
		20000, 2, 5752000, 32000,
	},
};

static struct msm_pm_boot_platform_data msm_pm_boot_pdata __initdata = {
	.mode = MSM_PM_BOOT_CONFIG_TZ,
};

static struct msm_rpmrs_platform_data msm_rpmrs_data __initdata = {
	.levels = &msm_rpmrs_levels[0],
	.num_levels = ARRAY_SIZE(msm_rpmrs_levels),
	.vdd_mem_levels  = {
		[MSM_RPMRS_VDD_MEM_RET_LOW]	= 750000,
		[MSM_RPMRS_VDD_MEM_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_MEM_ACTIVE]	= 1050000,
		[MSM_RPMRS_VDD_MEM_MAX]		= 1150000,
	},
	.vdd_dig_levels = {
		[MSM_RPMRS_VDD_DIG_RET_LOW]	= 500000,
		[MSM_RPMRS_VDD_DIG_RET_HIGH]	= 750000,
		[MSM_RPMRS_VDD_DIG_ACTIVE]	= 950000,
		[MSM_RPMRS_VDD_DIG_MAX]		= 1150000,
	},
	.vdd_mask = 0x7FFFFF,
	.rpmrs_target_id = {
		[MSM_RPMRS_ID_PXO_CLK]		= MSM_RPM_ID_PXO_CLK,
		[MSM_RPMRS_ID_L2_CACHE_CTL]	= MSM_RPM_ID_LAST,
		[MSM_RPMRS_ID_VDD_DIG_0]	= MSM_RPM_ID_PM8921_S3_0,
		[MSM_RPMRS_ID_VDD_DIG_1]	= MSM_RPM_ID_PM8921_S3_1,
		[MSM_RPMRS_ID_VDD_MEM_0]	= MSM_RPM_ID_PM8921_L24_0,
		[MSM_RPMRS_ID_VDD_MEM_1]	= MSM_RPM_ID_PM8921_L24_1,
		[MSM_RPMRS_ID_RPM_CTL]		= MSM_RPM_ID_RPM_CTL,
	},
};

static uint8_t spm_wfi_cmd_sequence[] __initdata = {
	0x03, 0x0f,
};

static uint8_t spm_power_collapse_without_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x03, 0x01,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static uint8_t spm_retention_cmd_sequence[] __initdata = {
	0x00, 0x05, 0x03, 0x0D,
	0x0B, 0x00, 0x0f,
};

static uint8_t spm_power_collapse_with_rpm[] __initdata = {
	0x00, 0x24, 0x54, 0x10,
	0x09, 0x07, 0x01, 0x0B,
	0x10, 0x54, 0x30, 0x0C,
	0x24, 0x30, 0x0f,
};

static struct msm_spm_seq_entry msm_spm_boot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_RETENTION,
		.notify_rpm = false,
		.cmd = spm_retention_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[3] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};
static struct msm_spm_seq_entry msm_spm_nonboot_cpu_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_MODE_CLOCK_GATING,
		.notify_rpm = false,
		.cmd = spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = false,
		.cmd = spm_power_collapse_without_rpm,
	},
	[2] = {
		.mode = MSM_SPM_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = spm_power_collapse_with_rpm,
	},
};

static uint8_t l2_spm_wfi_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x03, 0x20,
	0x00, 0x0f,
};

static uint8_t l2_spm_gdhs_cmd_sequence[] __initdata = {
	0x00, 0x20, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x20,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0f,
};
static uint8_t l2_spm_power_off_cmd_sequence[] __initdata = {
	0x00, 0x10, 0x34, 0x64,
	0x48, 0x07, 0x48, 0x10,
	0x50, 0x64, 0x04, 0x34,
	0x50, 0x0F,
};

static struct msm_spm_seq_entry msm_spm_l2_seq_list[] __initdata = {
	[0] = {
		.mode = MSM_SPM_L2_MODE_RETENTION,
		.notify_rpm = false,
		.cmd = l2_spm_wfi_cmd_sequence,
	},
	[1] = {
		.mode = MSM_SPM_L2_MODE_GDHS,
		.notify_rpm = true,
		.cmd = l2_spm_gdhs_cmd_sequence,
	},
	[2] = {
		.mode = MSM_SPM_L2_MODE_POWER_COLLAPSE,
		.notify_rpm = true,
		.cmd = l2_spm_power_off_cmd_sequence,
	},
};


static struct msm_spm_platform_data msm_spm_l2_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW_L2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x00A000AE,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A00020,
		.modes = msm_spm_l2_seq_list,
		.num_modes = ARRAY_SIZE(msm_spm_l2_seq_list),
	},
};

static struct msm_spm_platform_data msm_spm_data[] __initdata = {
	[0] = {
		.reg_base_addr = MSM_SAW0_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x03020004,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0084009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x00A4001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_boot_cpu_seq_list),
		.modes = msm_spm_boot_cpu_seq_list,
	},
	[1] = {
		.reg_base_addr = MSM_SAW1_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[2] = {
		.reg_base_addr = MSM_SAW2_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
	[3] = {
		.reg_base_addr = MSM_SAW3_BASE,
		.reg_init_values[MSM_SPM_REG_SAW2_CFG] = 0x1F,
#if defined(CONFIG_MSM_AVS_HW)
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_CTL] = 0x00,
		.reg_init_values[MSM_SPM_REG_SAW2_AVS_HYSTERESIS] = 0x00,
#endif
		.reg_init_values[MSM_SPM_REG_SAW2_SPM_CTL] = 0x01,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DLY] = 0x02020204,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_0] = 0x0060009C,
		.reg_init_values[MSM_SPM_REG_SAW2_PMIC_DATA_1] = 0x0000001C,
		.vctl_timeout_us = 50,
		.num_modes = ARRAY_SIZE(msm_spm_nonboot_cpu_seq_list),
		.modes = msm_spm_nonboot_cpu_seq_list,
	},
};

static void __init apq8064_init_buses(void)
{
	msm_bus_rpm_set_mt_mask();
	msm_bus_8064_apps_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_sys_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_mm_fabric_pdata.rpm_enabled = 1;
	msm_bus_8064_apps_fabric.dev.platform_data =
		&msm_bus_8064_apps_fabric_pdata;
	msm_bus_8064_sys_fabric.dev.platform_data =
		&msm_bus_8064_sys_fabric_pdata;
	msm_bus_8064_mm_fabric.dev.platform_data =
		&msm_bus_8064_mm_fabric_pdata;
	msm_bus_8064_sys_fpb.dev.platform_data = &msm_bus_8064_sys_fpb_pdata;
	msm_bus_8064_cpss_fpb.dev.platform_data = &msm_bus_8064_cpss_fpb_pdata;
}

/* PCIe gpios */
static struct msm_pcie_gpio_info_t msm_pcie_gpio_info[MSM_PCIE_MAX_GPIO] = {
	{"rst_n", PM8921_MPP_PM_TO_SYS(PCIE_RST_N_PMIC_MPP), 0},
	{"pwr_en", PM8921_GPIO_PM_TO_SYS(PCIE_PWR_EN_PMIC_GPIO), 1},
};

static struct msm_pcie_platform msm_pcie_platform_data = {
	.gpio = msm_pcie_gpio_info,
	.axi_addr = PCIE_AXI_BAR_PHYS,
	.axi_size = PCIE_AXI_BAR_SIZE,
	.wake_n = PM8921_GPIO_IRQ(PM8921_IRQ_BASE, PCIE_WAKE_N_PMIC_GPIO),
};

static int __init mpq8064_pcie_enabled(void)
{
	return !((readl_relaxed(QFPROM_RAW_FEAT_CONFIG_ROW0_MSB) & BIT(21)) ||
		(readl_relaxed(QFPROM_RAW_OEM_CONFIG_ROW0_LSB) & BIT(4)));
}

static void __init mpq8064_pcie_init(void)
{
	if (mpq8064_pcie_enabled()) {
		msm_device_pcie.dev.platform_data = &msm_pcie_platform_data;
		platform_device_register(&msm_device_pcie);
	}
}

static struct platform_device apq8064_device_ext_5v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(7),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_5V],
	},
};

static struct platform_device apq8064_device_ext_mpp8_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_MPP_PM_TO_SYS(8),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_MPP8],
	},
};

#if 0
static struct platform_device apq8064_device_ext_3p3v_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
#if (CONFIG_BOARD_VER <= CONFIG_PT10)
	.id	= APQ8064_EXT_3P3V_REG_EN_GPIO,
#endif	
	.dev	= {
		.platform_data =
			&apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_3P3V],
	},
};

static struct platform_device apq8064_device_ext_ts_sw_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= PM8921_GPIO_PM_TO_SYS(23),
	.dev	= {
		.platform_data
			= &apq8064_gpio_regulator_pdata[GPIO_VREG_ID_EXT_TS_SW],
	},
};
#endif
static struct platform_device apq8064_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= 0,
	.dev	= {
		.platform_data = &apq8064_rpm_regulator_pdata,
	},
};

static struct platform_device
apq8064_pm8921_device_rpm_regulator __devinitdata = {
	.name	= "rpm-regulator",
	.id	= 1,
	.dev	= {
		.platform_data = &apq8064_rpm_regulator_pm8921_pdata,
	},
};

static struct gpio_ir_recv_platform_data gpio_ir_recv_pdata = {
	.gpio_nr = 88,
	.active_low = 1,
};

static struct platform_device gpio_ir_recv_pdev = {
	.name = "gpio-rc-recv",
	.dev = {
		.platform_data = &gpio_ir_recv_pdata,
	},
};

static struct platform_device *common_not_mpq_devices[] __initdata = {
	&apq8064_device_qup_i2c_gsbi1,
	&apq8064_device_qup_i2c_gsbi3,
	&apq8064_device_qup_i2c_gsbi4,
#ifdef CONFIG_PIEZO_ECO
	&apq8064_device_qup_i2c_gsbi5,
#endif
};

static struct platform_device *common_mpq_devices[] __initdata = {
	&mpq_cpudai_sec_i2s_rx,
	&mpq_cpudai_mi2s_tx,
};

static struct platform_device *common_i2s_devices[] __initdata = {
	&apq_cpudai_mi2s,
	&apq_cpudai_i2s_rx,
	&apq_cpudai_i2s_tx,
};

static struct platform_device *early_common_devices[] __initdata = {
	&apq8064_device_acpuclk,
	&apq8064_device_dmov,
	&apq8064_device_qup_spi_gsbi5,
#ifdef CONFIG_SKY_DMB_SPI_HW
	&apq8064_device_qup_spi_gsbi7,
#endif
};

static struct platform_device *pm8921_common_devices[] __initdata = {
	&apq8064_device_ext_5v_vreg,
	&apq8064_device_ext_mpp8_vreg,
//	&apq8064_device_ext_3p3v_vreg,
	&apq8064_device_ssbi_pmic1,
	&apq8064_device_ssbi_pmic2,
	//	&apq8064_device_ext_ts_sw_vreg,
};

static struct platform_device *pm8917_common_devices[] __initdata = {
	&apq8064_device_ext_mpp8_vreg,
	//	&apq8064_device_ext_3p3v_vreg,
	&apq8064_device_ssbi_pmic1,
	&apq8064_device_ssbi_pmic2,
	//	&apq8064_device_ext_ts_sw_vreg,
};

static struct platform_device *common_devices[] __initdata = {
#if defined(CONFIG_SKY_SND_EXTERNAL_AMP) //YDA165
#if ( ( defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD) ) && (CONFIG_BOARD_VER > CONFIG_WS10) ) || \
	defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)
	&yda165_i2c_gpio_device,
#else
	&fab2210_i2c_gpio_device,
#endif	
#endif /* CONFIG_SKY_SND_EXTERNAL_AMP */
	&msm_device_smd_apq8064,
	&apq8064_device_otg,
	&apq8064_device_gadget_peripheral,
	&apq8064_device_hsusb_host,
	&android_usb_device,
	&msm_device_wcnss_wlan,
	&msm_device_iris_fm,
	&apq8064_fmem_device,
#ifdef CONFIG_ANDROID_PMEM
#ifndef CONFIG_MSM_MULTIMEDIA_USE_ION
	&apq8064_android_pmem_device,
	&apq8064_android_pmem_adsp_device,
	&apq8064_android_pmem_audio_device,
#endif /*CONFIG_MSM_MULTIMEDIA_USE_ION*/
#endif /*CONFIG_ANDROID_PMEM*/
#ifdef CONFIG_ION_MSM
	&apq8064_ion_dev,
#endif
	&msm8064_device_watchdog,
	&msm8064_device_saw_regulator_core0,
	&msm8064_device_saw_regulator_core1,
	&msm8064_device_saw_regulator_core2,
	&msm8064_device_saw_regulator_core3,
#if defined(CONFIG_QSEECOM)
	&qseecom_device,
#endif

#if 1//def CONFIG_MSM_USE_TSIF1
      &msm_8064_device_tsif[0], // EF48/49/50/51 use TSIF PORT1
#else
    //  &msm_8064_device_tsif[1],
#endif /* CONFIG_MSM_USE_TSIF1 */

#if defined(CONFIG_CRYPTO_DEV_QCRYPTO) || \
		defined(CONFIG_CRYPTO_DEV_QCRYPTO_MODULE)
	&qcrypto_device,
#endif

#if defined(CONFIG_CRYPTO_DEV_QCEDEV) || \
		defined(CONFIG_CRYPTO_DEV_QCEDEV_MODULE)
	&qcedev_device,
#endif

#ifdef CONFIG_HW_RANDOM_MSM
	&apq8064_device_rng,
#endif
	&apq_pcm,
	&apq_pcm_routing,
	&apq_cpudai0,
	&apq_cpudai1,
	&apq_cpudai_hdmi_rx,
	&apq_cpudai_bt_rx,
	&apq_cpudai_bt_tx,
	&apq_cpudai_fm_rx,
	&apq_cpudai_fm_tx,
	&apq_cpu_fe,
	&apq_stub_codec,
	&apq_voice,
	&apq_voip,
	&apq_lpa_pcm,
	&apq_compr_dsp,
	&apq_multi_ch_pcm,
	&apq_lowlatency_pcm,
	&apq_pcm_hostless,
	&apq_cpudai_afe_01_rx,
	&apq_cpudai_afe_01_tx,
	&apq_cpudai_afe_02_rx,
	&apq_cpudai_afe_02_tx,
	&apq_pcm_afe,
	&apq_cpudai_auxpcm_rx,
	&apq_cpudai_auxpcm_tx,
	&apq_cpudai_stub,
	&apq_cpudai_slimbus_1_rx,
	&apq_cpudai_slimbus_1_tx,
	&apq_cpudai_slimbus_2_rx,
	&apq_cpudai_slimbus_2_tx,
	&apq_cpudai_slimbus_3_rx,
	&apq_cpudai_slimbus_3_tx,
	&apq8064_rpm_device,
	&apq8064_rpm_log_device,
	&apq8064_rpm_stat_device,
	&apq8064_rpm_master_stat_device,
	&apq_device_tz_log,
	&msm_bus_8064_apps_fabric,
	&msm_bus_8064_sys_fabric,
	&msm_bus_8064_mm_fabric,
	&msm_bus_8064_sys_fpb,
	&msm_bus_8064_cpss_fpb,
	&apq8064_msm_device_vidc,
	&msm_pil_dsps,
	&msm_8960_q6_lpass,
	&msm_pil_vidc,
	&msm_gss,
	&apq8064_rtb_device,
	&apq8064_cpu_idle_device,
	&apq8064_device_cache_erp,
	&msm8960_device_ebi1_ch0_erp,
	&msm8960_device_ebi1_ch1_erp,
	&epm_adc_device,
	&coresight_tpiu_device,
	&coresight_etb_device,
	&apq8064_coresight_funnel_device,
	&coresight_etm0_device,
	&coresight_etm1_device,
	&coresight_etm2_device,
	&coresight_etm3_device,
	&apq_cpudai_slim_4_rx,
	&apq_cpudai_slim_4_tx,
#ifdef CONFIG_MSM_GEMINI
	&msm8960_gemini_device,
#endif
	&apq8064_iommu_domain_device,
	&msm_tsens_device,
	&apq8064_cache_dump_device,
	//&msm_8064_device_tspp,
#ifdef CONFIG_BATTERY_BCL
	&battery_bcl_device,
#endif
#ifdef CONFIG_SKY_DMB_SPI_GPIO
	&gpio_spi_tdmb_device,
#endif
#ifdef CONFIG_SKY_DMB_I2C_GPIO
   //GPIO not QUP
    &msm_device_i2c_gpio_dmb,
#endif /* CONFIG_SKY_DMB_I2C_GPIO */
#if defined(CONFIG_PANTECH_MAX17058_FG)
	&max17058_i2c_device,
#endif
#if defined(CONFIG_PANTECH_SMB347_CHARGER)
	&msm_device_i2c_gpio_smb,
#endif		

#if defined(CONFIG_TOUCHSCREEN_PP8360)
	&msm_device_i2c_gpio_backtouch,
#endif /* CONFIG_TOUCHSCREEN_PP8360 */

#ifdef CONFIG_PIEZO
#ifndef CONFIG_PIEZO_ECO
    &ts5000_i2c_gpio_device,
#endif //CONFIG_PIEZO_ECO
#endif /* CONFIG_PIEZO */
};

static struct platform_device *cdp_devices[] __initdata = {
#ifndef CONFIG_PN544
	&apq8064_device_uart_gsbi1,
#endif
	#ifdef CONFIG_PANTECH_GSBI5_UART_CONSOLE
	&apq8064_device_uart_gsbi5,
	#else
	&apq8064_device_uart_gsbi7,
	#endif
	&msm_device_sps_apq8064,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
};

static struct platform_device
mpq8064_device_ext_1p2_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 2),
	.dev	= {
		.platform_data =
		 &mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_1P2V],
	},
};

static struct platform_device
mpq8064_device_ext_1p8_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 4),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_1P8V],
	},
};

static struct platform_device
mpq8064_device_ext_2p2_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 14),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_2P2V],
	},
};

static struct platform_device
mpq8064_device_ext_5v_buck_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 3),
	.dev	= {
		.platform_data =
		 &mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_5V],
	},
};

static struct platform_device
mpq8064_device_ext_3p3v_ldo_vreg __devinitdata = {
	.name	= GPIO_REGULATOR_DEV_NAME,
	.id	= SX150X_GPIO(4, 15),
	.dev	= {
		.platform_data =
		&mpq8064_gpio_regulator_pdata[GPIO_VREG_ID_AVC_3P3V],
	},
};

static struct platform_device rc_input_loopback_pdev = {
	.name	= "rc-user-input",
	.id	= -1,
};

static int rf4ce_gpio_init(void)
{
	if (!machine_is_mpq8064_cdp() &&
		!machine_is_mpq8064_hrd() &&
			!machine_is_mpq8064_dtv())
		return -EINVAL;

	/* CC2533 SRDY Input */
	if (!gpio_request(SX150X_GPIO(4, 6), "rf4ce_srdy")) {
		gpio_direction_input(SX150X_GPIO(4, 6));
		gpio_export(SX150X_GPIO(4, 6), true);
	}

	/* CC2533 MRDY Output */
	if (!gpio_request(SX150X_GPIO(4, 5), "rf4ce_mrdy")) {
		gpio_direction_output(SX150X_GPIO(4, 5), 1);
		gpio_export(SX150X_GPIO(4, 5), true);
	}

	/* CC2533 Reset Output */
	if (!gpio_request(SX150X_GPIO(4, 7), "rf4ce_reset")) {
		gpio_direction_output(SX150X_GPIO(4, 7), 0);
		gpio_export(SX150X_GPIO(4, 7), true);
	}

	return 0;
}
late_initcall(rf4ce_gpio_init);

#ifdef CONFIG_SERIAL_MSM_HS
static int configure_uart_gpios(int on)
{
	int ret = 0, i;
	int uart_gpios[] = {14, 15, 16, 17};

	for (i = 0; i < ARRAY_SIZE(uart_gpios); i++) {
		if (on) {
			ret = gpio_request(uart_gpios[i], NULL);
			if (ret) {
				pr_err("%s:unable to request uart gpio[%d]\n",
						__func__, uart_gpios[i]);
				break;
			}
		} else {
			gpio_free(uart_gpios[i]);
		}
	}

	if (ret && on && i)
		for (; i >= 0; i--)
			gpio_free(uart_gpios[i]);
	return ret;
}

static struct msm_serial_hs_platform_data mpq8064_gsbi6_uartdm_pdata = {
	.inject_rx_on_wakeup	= 1,
	.rx_to_inject		= 0xFD,
	.gpio_config		= configure_uart_gpios,
};
#else
static struct msm_serial_hs_platform_data msm_uart_dm9_pdata;
#endif

static struct platform_device *mpq_devices[] __initdata = {
	&msm_device_sps_apq8064,
	&mpq8064_device_qup_i2c_gsbi5,
#ifdef CONFIG_MSM_ROTATOR
	&msm_rotator_device,
#endif
	&gpio_ir_recv_pdev,
	&mpq8064_device_ext_1p2_buck_vreg,
	&mpq8064_device_ext_1p8_buck_vreg,
	&mpq8064_device_ext_2p2_buck_vreg,
	&mpq8064_device_ext_5v_buck_vreg,
	&mpq8064_device_ext_3p3v_ldo_vreg,
#ifdef CONFIG_MSM_VCAP
	&msm8064_device_vcap,
#endif
	&rc_input_loopback_pdev,
};

static struct msm_spi_platform_data apq8064_qup_spi_gsbi5_pdata = {
	.max_clock_speed = 1100000,
};

#define KS8851_IRQ_GPIO		43

static struct spi_board_info spi_board_info[] __initdata = {
	{
		.modalias               = "ks8851",
		.irq                    = MSM_GPIO_TO_INT(KS8851_IRQ_GPIO),
		.max_speed_hz           = 19200000,
		.bus_num                = 0,
		.chip_select            = 2,
		.mode                   = SPI_MODE_0,
	},
	{
		.modalias		= "epm_adc",
		.max_speed_hz		= 1100000,
		.bus_num		= 0,
		.chip_select		= 3,
		.mode			= SPI_MODE_0,
	},
};

static struct slim_boardinfo apq8064_slim_devices[] = {
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla,
	},
	{
		.bus_num = 1,
		.slim_slave = &apq8064_slim_tabla20,
	},
	/* add more slimbus slaves as needed */
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi1_pdata = {
#if defined(CONFIG_PN544)
	.clk_freq = 400000,
#else
	.clk_freq = 100000,
#endif
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi3_pdata = {
	.clk_freq = 384000,
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi4_pdata = {
#ifdef CONFIG_PANTECH_CAMERA
		.clk_freq = 384000,
#else	
	.clk_freq = 100000,
#endif
	.src_clk_rate = 24000000,
};

static struct msm_i2c_platform_data mpq8064_i2c_qup_gsbi5_pdata = {
	.clk_freq = 100000,
	.src_clk_rate = 24000000,
};
#ifdef CONFIG_PIEZO_ECO
static struct msm_i2c_platform_data apq8064_i2c_qup_gsbi5_pdata = {
	.clk_freq = 384000,
	.src_clk_rate = 24000000,
    .use_gsbi_shared_mode = 1,
};
#endif

#define GSBI_DUAL_MODE_CODE 0x60
#define MSM_GSBI1_PHYS		0x12440000
static void __init apq8064_i2c_init(void)
{
#ifndef CONFIG_PN544
	void __iomem *gsbi_mem;

	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
	gsbi_mem = ioremap_nocache(MSM_GSBI1_PHYS, 4);
	writel_relaxed(GSBI_DUAL_MODE_CODE, gsbi_mem);
	/* Ensure protocol code is written before proceeding */
	wmb();
	iounmap(gsbi_mem);
	apq8064_i2c_qup_gsbi1_pdata.use_gsbi_shared_mode = 1;
#else
	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
#endif
	apq8064_device_qup_i2c_gsbi3.dev.platform_data =
					&apq8064_i2c_qup_gsbi3_pdata;
	apq8064_device_qup_i2c_gsbi1.dev.platform_data =
					&apq8064_i2c_qup_gsbi1_pdata;
	apq8064_device_qup_i2c_gsbi4.dev.platform_data =
					&apq8064_i2c_qup_gsbi4_pdata;
	mpq8064_device_qup_i2c_gsbi5.dev.platform_data =
					&mpq8064_i2c_qup_gsbi5_pdata;
#ifdef CONFIG_PIEZO_ECO
	apq8064_device_qup_i2c_gsbi5.dev.platform_data =
					&apq8064_i2c_qup_gsbi5_pdata;
#endif

#if defined(CONFIG_SKY_SND_EXTERNAL_AMP) //YDA165
#if ( ( defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD) ) && (CONFIG_BOARD_VER > CONFIG_WS10) ) || \
	defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)
	yda165_i2c_gpio_device.dev.platform_data =
					&yda165_i2c_gpio_data;
#else
	fab2210_i2c_gpio_device.dev.platform_data =
					&fab2210_i2c_gpio_data;
#endif
#endif /* CONFIG_SKY_SND_EXTERNAL_AMP */


}

#if defined(CONFIG_KS8851) || defined(CONFIG_KS8851_MODULE)
static int ethernet_init(void)
{
	int ret;
	ret = gpio_request(KS8851_IRQ_GPIO, "ks8851_irq");
	if (ret) {
		pr_err("ks8851 gpio_request failed: %d\n", ret);
		goto fail;
	}

	return 0;
fail:
	return ret;
}
#else
static int ethernet_init(void)
{
	return 0;
}
#endif

#define GPIO_KEY_HOME			PM8921_GPIO_PM_TO_SYS(27)

#if !defined(CONFIG_MACH_APQ8064_EF48S) && !defined(CONFIG_MACH_APQ8064_EF49K) && !defined(CONFIG_MACH_APQ8064_EF50L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K) && !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L)
#define GPIO_KEY_VOLUME_UP		PM8921_GPIO_PM_TO_SYS(35)
#define GPIO_KEY_VOLUME_DOWN_PM8921	PM8921_GPIO_PM_TO_SYS(38)
#define GPIO_KEY_CAM_FOCUS		PM8921_GPIO_PM_TO_SYS(3)
#define GPIO_KEY_CAM_SNAP		PM8921_GPIO_PM_TO_SYS(4)

#else
#define GPIO_KEY_VOLUME_UP		PM8921_GPIO_PM_TO_SYS(9)
#define GPIO_KEY_VOLUME_DOWN_PM8921	PM8921_GPIO_PM_TO_SYS(10)
#endif
#define GPIO_KEY_VOLUME_DOWN_PM8917	PM8921_GPIO_PM_TO_SYS(30)
#define GPIO_KEY_ROTATION_PM8921	PM8921_GPIO_PM_TO_SYS(42)
#define GPIO_KEY_ROTATION_PM8917	PM8921_GPIO_PM_TO_SYS(8)

static struct gpio_keys_button cdp_keys_pm8921[] = {
	{
		.code           = KEY_HOME,
		.gpio           = GPIO_KEY_HOME,
		.desc           = "home_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN_PM8921,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = SW_ROTATE_LOCK,
		.gpio           = GPIO_KEY_ROTATION_PM8921,
		.desc           = "rotate_key",
		.active_low     = 1,
		.type		= EV_SW,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_button cdp_keys_pm8917[] = {
	{
		.code           = KEY_HOME,
		.gpio           = GPIO_KEY_HOME,
		.desc           = "home_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN_PM8917,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = SW_ROTATE_LOCK,
		.gpio           = GPIO_KEY_ROTATION_PM8917,
		.desc           = "rotate_key",
		.active_low     = 1,
		.type		= EV_SW,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_platform_data cdp_keys_data = {
	.buttons        = cdp_keys_pm8921,
	.nbuttons       = ARRAY_SIZE(cdp_keys_pm8921),
};

static struct platform_device cdp_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &cdp_keys_data,
	},
};

#if !defined(CONFIG_MACH_APQ8064_EF48S) && !defined(CONFIG_MACH_APQ8064_EF49K) && !defined(CONFIG_MACH_APQ8064_EF50L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K) && !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L)
static struct gpio_keys_button mtp_keys[] = {
	{
		.code           = KEY_CAMERA_FOCUS,
		.gpio           = GPIO_KEY_CAM_FOCUS,
		.desc           = "cam_focus_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN_PM8921,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_CAMERA_SNAPSHOT,
		.gpio           = GPIO_KEY_CAM_SNAP,
		.desc           = "cam_snap_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.debounce_interval = 15,
	},
};
#else
static struct gpio_keys_button mtp_keys[] = {
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 0,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN_PM8921,
		.desc           = "volume_down_key",
		.active_low     = 0,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};
#endif

static struct gpio_keys_platform_data mtp_keys_data = {
	.buttons        = mtp_keys,
	.nbuttons       = ARRAY_SIZE(mtp_keys),
};

static struct platform_device mtp_kp_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &mtp_keys_data,
	},
};

static struct gpio_keys_button mpq_keys[] = {
	{
		.code           = KEY_VOLUMEDOWN,
		.gpio           = GPIO_KEY_VOLUME_DOWN_PM8921,
		.desc           = "volume_down_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
	{
		.code           = KEY_VOLUMEUP,
		.gpio           = GPIO_KEY_VOLUME_UP,
		.desc           = "volume_up_key",
		.active_low     = 1,
		.type		= EV_KEY,
		.wakeup		= 1,
		.debounce_interval = 15,
	},
};

static struct gpio_keys_platform_data mpq_keys_data = {
	.buttons        = mpq_keys,
	.nbuttons       = ARRAY_SIZE(mpq_keys),
};

static struct platform_device mpq_gpio_keys_pdev = {
	.name           = "gpio-keys",
	.id             = -1,
	.dev            = {
		.platform_data  = &mpq_keys_data,
	},
};

#define MPQ_KP_ROW_BASE		SX150X_EXP2_GPIO_BASE
#define MPQ_KP_COL_BASE		(SX150X_EXP2_GPIO_BASE + 4)

static unsigned int mpq_row_gpios[] = {MPQ_KP_ROW_BASE, MPQ_KP_ROW_BASE + 1,
				MPQ_KP_ROW_BASE + 2, MPQ_KP_ROW_BASE + 3};
static unsigned int mpq_col_gpios[] = {MPQ_KP_COL_BASE, MPQ_KP_COL_BASE + 1,
				MPQ_KP_COL_BASE + 2};

static const unsigned int mpq_keymap[] = {
	KEY(0, 0, KEY_UP),
	KEY(0, 1, KEY_ENTER),
	KEY(0, 2, KEY_3),

	KEY(1, 0, KEY_DOWN),
	KEY(1, 1, KEY_EXIT),
	KEY(1, 2, KEY_4),

	KEY(2, 0, KEY_LEFT),
	KEY(2, 1, KEY_1),
	KEY(2, 2, KEY_5),

	KEY(3, 0, KEY_RIGHT),
	KEY(3, 1, KEY_2),
	KEY(3, 2, KEY_6),
};

static struct matrix_keymap_data mpq_keymap_data = {
	.keymap_size	= ARRAY_SIZE(mpq_keymap),
	.keymap		= mpq_keymap,
};

static struct matrix_keypad_platform_data mpq_keypad_data = {
	.keymap_data		= &mpq_keymap_data,
	.row_gpios		= mpq_row_gpios,
	.col_gpios		= mpq_col_gpios,
	.num_row_gpios		= ARRAY_SIZE(mpq_row_gpios),
	.num_col_gpios		= ARRAY_SIZE(mpq_col_gpios),
	.col_scan_delay_us	= 32000,
	.debounce_ms		= 20,
	.wakeup			= 1,
	.active_low		= 1,
	.no_autorepeat		= 1,
};

static struct platform_device mpq_keypad_device = {
	.name           = "matrix-keypad",
	.id             = -1,
	.dev            = {
		.platform_data  = &mpq_keypad_data,
	},
};

/* Sensors DSPS platform data */
#define DSPS_PIL_GENERIC_NAME		"dsps"
static void __init apq8064_init_dsps(void)
{
	struct msm_dsps_platform_data *pdata =
		msm_dsps_device_8064.dev.platform_data;
	pdata->pil_name = DSPS_PIL_GENERIC_NAME;
	pdata->gpios = NULL;
	pdata->gpios_num = 0;

	platform_device_register(&msm_dsps_device_8064);
}

#define I2C_SURF 1
#define I2C_FFA  (1 << 1)
#define I2C_RUMI (1 << 2)
#define I2C_SIM  (1 << 3)
#define I2C_LIQUID (1 << 4)
#define I2C_MPQ_CDP	BIT(5)
#define I2C_MPQ_HRD	BIT(6)
#define I2C_MPQ_DTV	BIT(7)

struct i2c_registry {
	u8                     machs;
	int                    bus;
	struct i2c_board_info *info;
	int                    len;
};

static struct i2c_registry apq8064_i2c_devices[] __initdata = {
#if !defined(CONFIG_MACH_APQ8064_EF48S) && !defined(CONFIG_MACH_APQ8064_EF49K) && !defined(CONFIG_MACH_APQ8064_EF50L) && !defined(CONFIG_MACH_APQ8064_EF51S) && !defined(CONFIG_MACH_APQ8064_EF51K) && !defined(CONFIG_MACH_APQ8064_EF51L) && !defined(CONFIG_MACH_APQ8064_EF52S) && !defined(CONFIG_MACH_APQ8064_EF52K) && !defined(CONFIG_MACH_APQ8064_EF52L)
	{
		I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		smb349_charger_i2c_info,
		ARRAY_SIZE(smb349_charger_i2c_info)
	},
	{
		I2C_SURF | I2C_LIQUID,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		mxt_device_info,
		ARRAY_SIZE(mxt_device_info),
	},
	{
		I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		cyttsp_info,
		ARRAY_SIZE(cyttsp_info),
	},
#endif
#if defined(CONFIG_PN544)
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID | I2C_RUMI,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		nfc_i2c_boardinfo,
		ARRAY_SIZE(nfc_i2c_boardinfo),
	},
#else
	{
		I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		isa1200_board_info,
		ARRAY_SIZE(isa1200_board_info),
	},
#endif
#if 0
#ifdef CONFIG_PIEZO_ECO
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI5_QUP_I2C_BUS_ID,
		ts5000_i2c_info,
		ARRAY_SIZE(ts5000_i2c_info),
	},
#endif // CONFIG_PIEZO_ECO
#endif
#if defined(CONFIG_TOUCHSCREEN_CYTTSP_GEN4)
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		cyttsp4_i2c_info,
		ARRAY_SIZE(cyttsp4_i2c_info),
	},
#elif defined(CONFIG_TOUCHSCREEN_QT602240)
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		qt602240_i2c_boardinfo,
		ARRAY_SIZE(qt602240_i2c_boardinfo),
},
#endif 

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_PANTECH)
	{
		I2C_SURF | I2C_FFA,
		APQ_8064_GSBI3_QUP_I2C_BUS_ID,
		synaptics_i2c_info,
		ARRAY_SIZE(synaptics_i2c_info),		
	},
#endif
	{
		I2C_MPQ_CDP,
		APQ_8064_GSBI5_QUP_I2C_BUS_ID,
		cs8427_device_info,
		ARRAY_SIZE(cs8427_device_info),
	},
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		sii_device_info,
		ARRAY_SIZE(sii_device_info),
	},

#if defined(CONFIG_SKY_SND_EXTERNAL_AMP) //YDA165
#if ( ( defined(CONFIG_SKY_EF51S_BOARD) || defined(CONFIG_SKY_EF51K_BOARD) || defined(CONFIG_SKY_EF51L_BOARD) ) && (CONFIG_BOARD_VER > CONFIG_WS10) ) || \
	defined(CONFIG_SKY_EF52S_BOARD) || defined(CONFIG_SKY_EF52K_BOARD) || defined(CONFIG_SKY_EF52L_BOARD)
	{
		I2C_SURF | I2C_FFA,
		APQ_8960_GSBI7_QUP_I2C_BUS_ID,
		yda165_i2c_boardinfo,
		ARRAY_SIZE(yda165_i2c_boardinfo),
	},
#else
	{
		I2C_SURF | I2C_FFA,
		APQ_8960_GSBI7_QUP_I2C_BUS_ID,
		fab2210_i2c_boardinfo,
		ARRAY_SIZE(fab2210_i2c_boardinfo),
	},
#endif	
#endif /* CONFIG_SKY_SND_EXTERNAL_AMP */ 
};

static struct i2c_registry apq8064_tabla_i2c_devices[] __initdata = {
	{
		.bus = APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		.info = apq8064_tabla_i2c_device_info,
		.len = ARRAY_SIZE(apq8064_tabla_i2c_device_info),
	},
};

#define SX150X_EXP1_INT_N	PM8921_MPP_IRQ(PM8921_IRQ_BASE, 9)
#define SX150X_EXP2_INT_N	MSM_GPIO_TO_INT(81)

struct sx150x_platform_data mpq8064_sx150x_pdata[] = {
	[SX150X_EXP1] = {
		.gpio_base	= SX150X_EXP1_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= SX150X_EXP1_INT_N,
		.irq_base	= SX150X_EXP1_IRQ_BASE,
	},
	[SX150X_EXP2] = {
		.gpio_base	= SX150X_EXP2_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0f,
		.io_pulldn_ena	= 0x70,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= SX150X_EXP2_INT_N,
		.irq_base	= SX150X_EXP2_IRQ_BASE,
	},
	[SX150X_EXP3] = {
		.gpio_base	= SX150X_EXP3_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
	[SX150X_EXP4] = {
		.gpio_base	= SX150X_EXP4_GPIO_BASE,
		.oscio_is_gpo	= false,
		.io_pullup_ena	= 0x0,
		.io_pulldn_ena	= 0x0,
		.io_open_drain_ena = 0x0,
		.io_polarity	= 0,
		.irq_summary	= -1,
	},
};

static struct i2c_board_info sx150x_gpio_exp_info[] = {
	{
		I2C_BOARD_INFO("sx1509q", 0x70),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP1],
	},
	{
		I2C_BOARD_INFO("sx1508q", 0x23),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP2],
	},
	{
		I2C_BOARD_INFO("sx1508q", 0x22),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP3],
	},
	{
		I2C_BOARD_INFO("sx1509q", 0x3E),
		.platform_data = &mpq8064_sx150x_pdata[SX150X_EXP4],
	},
};

#define MPQ8064_I2C_GSBI5_BUS_ID	5

static struct i2c_registry mpq8064_i2c_devices[] __initdata = {
	{
		I2C_MPQ_CDP,
		MPQ8064_I2C_GSBI5_BUS_ID,
		sx150x_gpio_exp_info,
		ARRAY_SIZE(sx150x_gpio_exp_info),
	},
};

static void __init register_i2c_devices(void)
{
	u8 mach_mask = 0;
	int i;
	u32 version;

#ifdef CONFIG_MSM_CAMERA
	struct i2c_registry apq8064_camera_i2c_devices = {
		I2C_SURF | I2C_FFA | I2C_LIQUID | I2C_RUMI,
		APQ_8064_GSBI4_QUP_I2C_BUS_ID,
		apq8064_camera_board_info.board_info,
		apq8064_camera_board_info.num_i2c_board_info,
	};
#endif
	/* Build the matching 'supported_machs' bitmask */
	if (machine_is_apq8064_cdp())
		mach_mask = I2C_SURF;
	else if (machine_is_apq8064_mtp()
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
		 || (machine_is_apq8064_ef48s() || machine_is_apq8064_ef49k() || machine_is_apq8064_ef50l() || machine_is_apq8064_ef51s() || machine_is_apq8064_ef51k() || machine_is_apq8064_ef51l() || machine_is_apq8064_ef52s() ||  machine_is_apq8064_ef52k() ||  machine_is_apq8064_ef52l())
#endif
	  )
		mach_mask = I2C_FFA;
	else if (machine_is_apq8064_liquid())
		mach_mask = I2C_LIQUID;
	else if (PLATFORM_IS_MPQ8064())
		mach_mask = I2C_MPQ_CDP;
	else
		pr_err("unmatched machine ID in register_i2c_devices\n");

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_devices); ++i) {
		if (apq8064_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(apq8064_i2c_devices[i].bus,
						apq8064_i2c_devices[i].info,
						apq8064_i2c_devices[i].len);
	}
#ifdef CONFIG_MSM_CAMERA
	if (apq8064_camera_i2c_devices.machs & mach_mask)
		i2c_register_board_info(apq8064_camera_i2c_devices.bus,
			apq8064_camera_i2c_devices.info,
			apq8064_camera_i2c_devices.len);
#endif

	for (i = 0; i < ARRAY_SIZE(mpq8064_i2c_devices); ++i) {
		if (mpq8064_i2c_devices[i].machs & mach_mask)
			i2c_register_board_info(
					mpq8064_i2c_devices[i].bus,
					mpq8064_i2c_devices[i].info,
					mpq8064_i2c_devices[i].len);
	}

	if (machine_is_apq8064_mtp()) {
		version = socinfo_get_platform_version();
		if (SOCINFO_VERSION_MINOR(version) == 1)
			for (i = 0; i < ARRAY_SIZE(apq8064_tabla_i2c_devices);
				 ++i)
				i2c_register_board_info(
				apq8064_tabla_i2c_devices[i].bus,
				apq8064_tabla_i2c_devices[i].info,
				apq8064_tabla_i2c_devices[i].len);
	}


#if defined(CONFIG_PANTECH_MAX17058_FG)
	i2c_register_board_info(APQ_8064_MAX17058_I2C_BUS_ID,
				max17058_i2c_boardinfo,
				ARRAY_SIZE(max17058_i2c_boardinfo));
#endif

#if defined(CONFIG_PANTECH_SMB347_CHARGER)
	i2c_register_board_info(APQ_8064_SMB347_I2C_BUS_ID,
				smb347_i2c_boardinfo,
				ARRAY_SIZE(smb347_i2c_boardinfo));
#endif

}

static void enable_ddr3_regulator(void)
{
	static struct regulator *ext_ddr3;

	/* Use MPP7 output state as a flag for PCDDR3 presence. */
	if (gpio_get_value_cansleep(PM8921_MPP_PM_TO_SYS(7)) > 0) {
		ext_ddr3 = regulator_get(NULL, "ext_ddr3");
		if (IS_ERR(ext_ddr3) || ext_ddr3 == NULL)
			pr_err("Could not get MPP7 regulator\n");
		else
			regulator_enable(ext_ddr3);
	}
}

static void enable_avc_i2c_bus(void)
{
	int avc_i2c_en_mpp = PM8921_MPP_PM_TO_SYS(8);
	int rc;

	rc = gpio_request(avc_i2c_en_mpp, "avc_i2c_en");
	if (rc)
		pr_err("request for avc_i2c_en mpp failed,"
						 "rc=%d\n", rc);
	else
		gpio_set_value_cansleep(avc_i2c_en_mpp, 1);
}

/* Modify platform data values to match requirements for PM8917. */
static void __init apq8064_pm8917_pdata_fixup(void)
{
	cdp_keys_data.buttons = cdp_keys_pm8917;
	cdp_keys_data.nbuttons = ARRAY_SIZE(cdp_keys_pm8917);
}

static void __init apq8064_common_init(void)
{
	u32 platform_version = socinfo_get_platform_version();

	if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
		apq8064_pm8917_pdata_fixup();
	platform_device_register(&msm_gpio_device);
	msm_tsens_early_init(&apq_tsens_pdata);
	msm_thermal_init(&msm_thermal_pdata);
	if (socinfo_init() < 0)
		pr_err("socinfo_init() failed!\n");
	BUG_ON(msm_rpm_init(&apq8064_rpm_data));
	BUG_ON(msm_rpmrs_levels_init(&msm_rpmrs_data));
	regulator_suppress_info_printing();
	if (socinfo_get_pmic_model() == PMIC_MODEL_PM8917)
		configure_apq8064_pm8917_power_grid();
	platform_device_register(&apq8064_device_rpm_regulator);
	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		platform_device_register(&apq8064_pm8921_device_rpm_regulator);
	if (msm_xo_init())
		pr_err("Failed to initialize XO votes\n");
	msm_clock_init(&apq8064_clock_init_data);
	apq8064_init_gpiomux();
	apq8064_i2c_init();
	register_i2c_devices();

	apq8064_device_qup_spi_gsbi5.dev.platform_data =
						&apq8064_qup_spi_gsbi5_pdata;
#ifdef CONFIG_SKY_DMB_SPI_HW
  apq8064_device_qup_spi_gsbi7.dev.platform_data = &apq8064_qup_spi_gsbi7_pdata;
#endif
  #ifdef CONFIG_SKY_DMB_SPI_GPIO
    gpio_spi_tdmb_device.dev.platform_data = &gpio_spi_tdmb_data;
  #endif

	apq8064_init_pmic();
	if (machine_is_apq8064_liquid())
		msm_otg_pdata.mhl_enable = true;

	if (apq8064_mhl_display_enabled())
		mhl_platform_data.mhl_enabled = true;

	android_usb_pdata.swfi_latency =
		msm_rpmrs_levels[0].latency_us;

	apq8064_device_otg.dev.platform_data = &msm_otg_pdata;
	apq8064_ehci_host_init();
	apq8064_init_buses();

	platform_add_devices(early_common_devices,
				ARRAY_SIZE(early_common_devices));
	if (socinfo_get_pmic_model() != PMIC_MODEL_PM8917)
		platform_add_devices(pm8921_common_devices,
					ARRAY_SIZE(pm8921_common_devices));
	else
		platform_add_devices(pm8917_common_devices,
					ARRAY_SIZE(pm8917_common_devices));
	platform_add_devices(common_devices, ARRAY_SIZE(common_devices));
	if (!(machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
			machine_is_mpq8064_dtv()))
		platform_add_devices(common_not_mpq_devices,
			ARRAY_SIZE(common_not_mpq_devices));

	if ((machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
			machine_is_mpq8064_dtv()))
		platform_add_devices(common_mpq_devices,
			ARRAY_SIZE(common_mpq_devices));

	if (machine_is_apq8064_mtp()) {
		if (SOCINFO_VERSION_MINOR(platform_version) == 1)
			platform_add_devices(common_i2s_devices,
			ARRAY_SIZE(common_i2s_devices));
	}

        enable_ddr3_regulator();
	msm_hsic_pdata.swfi_latency =
		msm_rpmrs_levels[0].latency_us;
	if (machine_is_apq8064_mtp()
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	  || (machine_is_apq8064_ef48s() || machine_is_apq8064_ef49k() || machine_is_apq8064_ef50l() || machine_is_apq8064_ef51s() || machine_is_apq8064_ef51k() || machine_is_apq8064_ef51l() || machine_is_apq8064_ef52s() ||  machine_is_apq8064_ef52k() ||  machine_is_apq8064_ef52l())
#endif
	) {
		msm_hsic_pdata.log2_irq_thresh = 5,
		apq8064_device_hsic_host.dev.platform_data = &msm_hsic_pdata;
		device_initialize(&apq8064_device_hsic_host.dev);
	}
	apq8064_pm8xxx_gpio_mpp_init();
	apq8064_init_mmc();

	if (machine_is_apq8064_mtp()
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	  || (machine_is_apq8064_ef48s() || machine_is_apq8064_ef49k() || machine_is_apq8064_ef50l() || machine_is_apq8064_ef51s() || machine_is_apq8064_ef51k() || machine_is_apq8064_ef51l() || machine_is_apq8064_ef52s() ||  machine_is_apq8064_ef52k() ||  machine_is_apq8064_ef52l())
#endif
	) {
		mdm_8064_device.dev.platform_data = &mdm_platform_data;
		if (SOCINFO_VERSION_MINOR(platform_version) == 1) {
			i2s_mdm_8064_device.dev.platform_data =
				&mdm_platform_data;
			platform_device_register(&i2s_mdm_8064_device);
		} else {
			mdm_8064_device.dev.platform_data = &mdm_platform_data;
		platform_device_register(&mdm_8064_device);
	}
	}
	platform_device_register(&apq8064_slim_ctrl);
	if (machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
		apq8064_slim_devices[ARRAY_SIZE(apq8064_slim_devices) - 1].\
			slim_slave = &mpq8064_slim_ashiko20;
	}
	slim_register_board_info(apq8064_slim_devices,
		ARRAY_SIZE(apq8064_slim_devices));
	if (!PLATFORM_IS_MPQ8064()) {
	apq8064_init_dsps();
		platform_device_register(&msm_8960_riva);
	}
	msm_spm_init(msm_spm_data, ARRAY_SIZE(msm_spm_data));
	msm_spm_l2_init(msm_spm_l2_data);
	BUG_ON(msm_pm_boot_init(&msm_pm_boot_pdata));
	apq8064_epm_adc_init();
	msm_pm_set_tz_retention_flag(1);

#if 0//def CONFIG_SKY_DMB_PMIC_19200
	configure_dmb_xo(); 
#endif
}

static void __init apq8064_allocate_memory_regions(void)
{
	apq8064_allocate_fb_region();
}

#if defined(CONFIG_QC_ABNORMAL_DEBUG_CODE) && !defined(CONFIG_PANTECH_USER_BUILD)
extern void enable_uncache_log_buf(char *, int);//ALRAN
extern void enable_uncache_sched_log(char *, int);//ALRAN
extern void enable_uncache_irq_log(char *, int);//ALRAN
#endif

static void __init apq8064_cdp_init(void)
{
	if (meminfo_init(SYS_MEMORY, SZ_256M) < 0)
		pr_err("meminfo_init() failed!\n");

#if defined(CONFIG_QC_ABNORMAL_DEBUG_CODE) && !defined(CONFIG_PANTECH_USER_BUILD)
	{
	unsigned long lphy;
	char * temp;
	
	//lphy = allocate_contiguous_ebi_nomap(ALRANLOGSIZE, PAGE_SIZE);
    //temp = (char *)__arm_ioremap(lphy, ALRANLOGSIZE, 11); //same as MT_MEMORY_NONCACHED

#if defined(CONFIG_PANTECH_DEBUG)
	lphy=0x88b00000;
#else
    lphy=0x88c00000;
#endif
    
	temp = (char *)ioremap_nocache(lphy, ALRANLOGSIZE);

	memset((void*)temp, 0, ALRANLOGSIZE);
	enable_uncache_log_buf(temp, ALRANLOGSIZE/2);
	pr_err("ALRAN:   log_buf mapped at 0x%08x/size %dKB\n", (unsigned int)temp, ALRANLOGSIZE/2/1024);
	temp = temp + (ALRANLOGSIZE/2);
	enable_uncache_sched_log(temp, ALRANLOGSIZE/4);
	pr_err("ALRAN: sched_buf mapped at 0x%08x/size %dKB\n", (unsigned int)temp, ALRANLOGSIZE/4/1024);
	temp = temp + (ALRANLOGSIZE/4);
	enable_uncache_irq_log(temp, ALRANLOGSIZE/4);
	pr_err("ALRAN:   irq_buf mapped at 0x%08x/size %dKB\n", (unsigned int)temp, ALRANLOGSIZE/4/1024);
	}
#endif

	apq8064_common_init();
	if (machine_is_mpq8064_cdp() || machine_is_mpq8064_hrd() ||
		machine_is_mpq8064_dtv()) {
		enable_avc_i2c_bus();
		msm_rotator_set_split_iommu_domain();
		platform_add_devices(mpq_devices, ARRAY_SIZE(mpq_devices));
		mpq8064_pcie_init();
	} else {
		ethernet_init();
		msm_rotator_set_split_iommu_domain();
		platform_add_devices(cdp_devices, ARRAY_SIZE(cdp_devices));
		spi_register_board_info(spi_board_info,
						ARRAY_SIZE(spi_board_info));
	}
	apq8064_init_fb();
	apq8064_init_gpu();
	platform_add_devices(apq8064_footswitch, apq8064_num_footswitch);
#ifdef CONFIG_MSM_CAMERA
	apq8064_init_cam();
#endif

	if (machine_is_mpq8064_hrd() || machine_is_mpq8064_dtv()) {
		platform_device_register(&mpq8064_device_uartdm_gsbi6);
#ifdef CONFIG_SERIAL_MSM_HS
		/* GSBI6(2) - UARTDM_RX */
		mpq8064_gsbi6_uartdm_pdata.wakeup_irq = gpio_to_irq(15);
		mpq8064_device_uartdm_gsbi6.dev.platform_data =
					&mpq8064_gsbi6_uartdm_pdata;
#endif
	}

	if (machine_is_apq8064_cdp() || machine_is_apq8064_liquid())
		platform_device_register(&cdp_kp_pdev);

	if (machine_is_apq8064_mtp()
#if defined(CONFIG_MACH_APQ8064_EF48S) ||defined(CONFIG_MACH_APQ8064_EF49K) || defined(CONFIG_MACH_APQ8064_EF50L) || defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) || defined(CONFIG_MACH_APQ8064_EF52S) || defined(CONFIG_MACH_APQ8064_EF52K) || defined(CONFIG_MACH_APQ8064_EF52L)
	  || (machine_is_apq8064_ef48s() || machine_is_apq8064_ef49k() || machine_is_apq8064_ef50l() || machine_is_apq8064_ef51s() || machine_is_apq8064_ef51k() || machine_is_apq8064_ef51l() || machine_is_apq8064_ef52s() ||  machine_is_apq8064_ef52k() ||  machine_is_apq8064_ef52l())
#endif
	)
		platform_device_register(&mtp_kp_pdev);

	change_memory_power = &apq8064_change_memory_power;

	if (machine_is_mpq8064_cdp()) {
		platform_device_register(&mpq_gpio_keys_pdev);
		platform_device_register(&mpq_keypad_device);
	}

#ifdef CONFIG_SKY_DMB_SPI_HW //check
  if (machine_is_apq8064_cdp())
    platform_device_register(&apq8064_device_qup_spi_gsbi7);
#endif

  #ifdef CONFIG_SKY_DMB_SPI_GPIO
    if (machine_is_apq8064_cdp())
	   platform_device_register(&gpio_spi_tdmb_device);
  #endif

#ifdef CONFIG_SKY_DMB_SPI_IF
	spi_register_board_info(dmb_spi_board_info, ARRAY_SIZE(dmb_spi_board_info));
#endif

#ifdef CONFIG_SKY_DMB_I2C_GPIO
	sky_dmb_i2c_gpio_init();
#endif /* CONFIG_SKY_DMB_I2C_GPIO */

#if defined(CONFIG_TOUCHSCREEN_PP8360)
	sky_back_touch_i2c_gpio_init();
#endif/* CONFIG_TOUCHSCREEN_PP8360 */
#ifdef CONFIG_PIEZO
#ifndef CONFIG_PIEZO_ECO
  //  ts5000_i2c_gpio_init();
#endif //CONFIG_PIEZO_ECO
#endif /* CONFIG_PIEZO */
}

MACHINE_START(APQ8064_CDP, "QCT APQ8064 CDP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_MTP, "QCT APQ8064 MTP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_LIQUID, "QCT APQ8064 LIQUID")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_CDP, "QCT MPQ8064 CDP")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_HRD, "QCT MPQ8064 HRD")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(MPQ8064_DTV, "QCT MPQ8064 DTV")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF48S, "PANTECH APQ8064 EF48S")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF49K, "PANTECH APQ8064 EF49K")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF50L, "PANTECH APQ8064 EF50L")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF51S, "PANTECH APQ8064 EF51S")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF51K, "PANTECH APQ8064 EF51K")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF51L, "PANTECH APQ8064 EF51L")
	.map_io = apq8064_map_io,
	.reserve = apq8064_reserve,
	.init_irq = apq8064_init_irq,
	.handle_irq = gic_handle_irq,
	.timer = &msm_timer,
	.init_machine = apq8064_cdp_init,
	.init_early = apq8064_allocate_memory_regions,
	.init_very_early = apq8064_early_reserve,
	.restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF52S, "PANTECH APQ8064 EF52S")
        .map_io = apq8064_map_io,
        .reserve = apq8064_reserve,
        .init_irq = apq8064_init_irq,
        .handle_irq = gic_handle_irq,
        .timer = &msm_timer,
        .init_machine = apq8064_cdp_init,
        .init_early = apq8064_allocate_memory_regions,
        .init_very_early = apq8064_early_reserve,
        .restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF52K, "PANTECH APQ8064 EF52K")
        .map_io = apq8064_map_io,
        .reserve = apq8064_reserve,
        .init_irq = apq8064_init_irq,
        .handle_irq = gic_handle_irq,
        .timer = &msm_timer,
        .init_machine = apq8064_cdp_init,
        .init_early = apq8064_allocate_memory_regions,
        .init_very_early = apq8064_early_reserve,
        .restart = msm_restart,
MACHINE_END

MACHINE_START(APQ8064_EF52L, "PANTECH APQ8064 EF52L")
        .map_io = apq8064_map_io,
        .reserve = apq8064_reserve,
        .init_irq = apq8064_init_irq,
        .handle_irq = gic_handle_irq,
        .timer = &msm_timer,
        .init_machine = apq8064_cdp_init,
        .init_early = apq8064_allocate_memory_regions,
        .init_very_early = apq8064_early_reserve,
        .restart = msm_restart,
MACHINE_END


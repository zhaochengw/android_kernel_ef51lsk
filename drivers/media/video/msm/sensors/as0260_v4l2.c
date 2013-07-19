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
#include <linux/gpio.h>
#include "msm_sensor.h"
#include <linux/regulator/machine.h> //F_AS0260_POWER
//#include "sensor_i2c.h"
#include "sensor_ctrl.h"

#define AS0260_MIPI_2_LANE

#ifdef AS0260_MIPI_2_LANE
#include "as0260_v4l2_cfg.h"
#else
#include "as0260_v4l2_cfg_1080.h"
#endif
#include "msm_camera_i2c.h"

#include "msm.h"
#include "msm_ispif.h"

#define SENSOR_NAME "as0260"
#define PLATFORM_DRIVER_NAME "msm_camera_as0260"
#define as0260_obj as0260_##obj


//wsyang_temp
#define F_AS0260_POWER

#ifdef F_AS0260_POWER
#define CAMIO_RST_N	0
#define CAMIO_STB_N	1

#define CAMIO_MAX	2

#define CAMIO_PM_MAX 1
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L))
#define CAM2_STANDBY	37
#elif (defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
#define CAM2_STANDBY	13
#else
#define CAM2_STANDBY	12
#endif

#define CAM2_RST_N		29
#define CAM1_IOVDD		77

static int as0260_set_aec_lock(struct msm_sensor_ctrl_t *s_ctrl ,int8_t is_lock);

static sgpio_ctrl_t sgpios[CAMIO_MAX] = {
	{CAMIO_RST_N, "CAM_RST_N_PM", CAM2_RST_N},
	{CAMIO_STB_N, "CAM_STB_N", CAM2_STANDBY},	
};

#define CAMV_IO_1P8V	0
#define CAMV_CORE_1P8V	1
#define CAMV_A_2P8V	2
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L)||defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
#define CAMV_I2C_1P8V	3
#define CAMV_MAX	4
#else
#define CAMV_MAX	3
#endif

static svreg_ctrl_t svregs[CAMV_MAX] = {
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L))
	{CAMV_IO_1P8V,   "8921_l29", NULL, 1800},
#elif (defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
	{CAMV_IO_1P8V,   "8921_l17", NULL, 1800},
#else
	{CAMV_IO_1P8V,   "8921_lvs5", NULL, 0},
#endif
	{CAMV_CORE_1P8V, "8921_lvs2",   NULL, 0},
	{CAMV_A_2P8V,    "8921_l22",  NULL, 2800},
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L)||defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
	{CAMV_I2C_1P8V,   "8921_lvs5", NULL, 0},
#endif	
};

int32_t as0260_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
#endif


DEFINE_MUTEX(as0260_mut);
static struct msm_sensor_ctrl_t as0260_s_ctrl;



static struct v4l2_subdev_info as0260_subdev_info[] = {
	{
	.code   = V4L2_MBUS_FMT_YUYV8_2X8,
	.colorspace = V4L2_COLORSPACE_JPEG,
	.fmt    = 1,
	.order    = 0,
	},
	/* more can be supported, to be added later */
};


static struct msm_camera_i2c_conf_array as0260_init_conf[] = {
	{as0260_recommend_settings,
	ARRAY_SIZE(as0260_recommend_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};

#if 0
static struct msm_camera_i2c_conf_array as0260_confs[] = {
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};
#else
static struct msm_camera_i2c_conf_array as0260_confs[] = {
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_hd_settings,
	ARRAY_SIZE(as0260_hd_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
	{as0260_1088_settings,
	ARRAY_SIZE(as0260_1088_settings), 0, MSM_CAMERA_I2C_WORD_DATA},
};
#endif

#define AS0260_FULL_SIZE_DUMMY_PIXELS     0
#define AS0260_FULL_SIZE_DUMMY_LINES    0
#define AS0260_FULL_SIZE_WIDTH    1920
#if 1
#define AS0260_FULL_SIZE_HEIGHT   1088
#else
#define AS0260_FULL_SIZE_HEIGHT   1080
#endif

#define AS0260_HD_SIZE_WIDTH     1280
#define AS0260_HD_SIZE_HEIGHT    720

#define AS0260_QTR_SIZE_DUMMY_PIXELS  0
#define AS0260_QTR_SIZE_DUMMY_LINES   0
#define AS0260_QTR_SIZE_WIDTH     800
#define AS0260_QTR_SIZE_HEIGHT    480

#define AS0260_HRZ_FULL_BLK_PIXELS   0//16 //190//304//696; //890;//
#define AS0260_VER_FULL_BLK_LINES     0//12 //32//24//44; //44;
#define AS0260_HRZ_QTR_BLK_PIXELS    0//16 //1830//1884//1648;//890;//888; //696;
#define AS0260_VER_QTR_BLK_LINES      0//12 //16//40//924;//44;

static struct msm_sensor_output_info_t as0260_dimensions[] = {
	{
		.x_output = AS0260_FULL_SIZE_WIDTH,//0x1070,
		.y_output = AS0260_FULL_SIZE_HEIGHT,
		.line_length_pclk = AS0260_FULL_SIZE_WIDTH + AS0260_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = AS0260_FULL_SIZE_HEIGHT+ AS0260_VER_FULL_BLK_LINES ,
#ifdef AS0260_MIPI_2_LANE
		.vt_pixel_clk = 62668800,//48000000,//50000000,
		.op_pixel_clk = 192000000,//128000000,//150000000,
#else
		.vt_pixel_clk = 41472000,//48000000,//50000000,
		.op_pixel_clk = 96000000,//128000000,//150000000,
#endif
		.binning_factor = 1,
	},
	{
#if 0	
		.x_output = AS0260_FULL_SIZE_WIDTH,
		.y_output = AS0260_FULL_SIZE_HEIGHT,
		.line_length_pclk = AS0260_FULL_SIZE_WIDTH + AS0260_HRZ_FULL_BLK_PIXELS,
		.frame_length_lines = AS0260_FULL_SIZE_HEIGHT+ AS0260_VER_FULL_BLK_LINES,
#else
		.x_output = AS0260_HD_SIZE_WIDTH,//0x1070,
		.y_output = AS0260_HD_SIZE_HEIGHT,
		.line_length_pclk = AS0260_HD_SIZE_WIDTH + AS0260_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = AS0260_HD_SIZE_HEIGHT+ AS0260_VER_FULL_BLK_LINES ,
#endif
#ifdef AS0260_MIPI_2_LANE
		.vt_pixel_clk = 62668800,//48000000,//50000000,
		.op_pixel_clk = 192000000,//128000000,//150000000,
#else
		.vt_pixel_clk = 41472000,//48000000,//50000000,
		.op_pixel_clk = 96000000,//128000000,//150000000,
#endif
		.binning_factor = 1,
	},
	{
		.x_output = AS0260_FULL_SIZE_WIDTH,//0x1070,
		.y_output = AS0260_FULL_SIZE_HEIGHT,
		.line_length_pclk = AS0260_FULL_SIZE_WIDTH + AS0260_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = AS0260_FULL_SIZE_HEIGHT+ AS0260_VER_FULL_BLK_LINES ,
#ifdef AS0260_MIPI_2_LANE
		.vt_pixel_clk = 62668800,//48000000,//50000000,
		.op_pixel_clk = 192000000,//128000000,//150000000,
#else
		.vt_pixel_clk = 41472000,//48000000,//50000000,
		.op_pixel_clk = 96000000,//128000000,//150000000,
#endif		
		.binning_factor = 1,
	},
	{
		.x_output = AS0260_FULL_SIZE_WIDTH,//0x1070,
		.y_output = AS0260_FULL_SIZE_HEIGHT,
		.line_length_pclk = AS0260_FULL_SIZE_WIDTH + AS0260_HRZ_FULL_BLK_PIXELS ,
		.frame_length_lines = AS0260_FULL_SIZE_HEIGHT+ AS0260_VER_FULL_BLK_LINES ,
#ifdef AS0260_MIPI_2_LANE
		.vt_pixel_clk = 62668800,//48000000,//50000000,
		.op_pixel_clk = 192000000,//128000000,//150000000,
#else
		.vt_pixel_clk = 41472000,//48000000,//50000000,
		.op_pixel_clk = 96000000,//128000000,//150000000,
#endif
		.binning_factor = 1,
	},
};

#if 0
static struct msm_camera_csid_vc_cfg as0260_cid_cfg[] = {
	{0, CSI_YUV422_8, CSI_DECODE_8BIT},
	{1, CSI_EMBED_DATA, CSI_DECODE_8BIT},
	{2, CSI_RESERVED_DATA, CSI_DECODE_8BIT},
	{3, CSI_RESERVED_DATA, CSI_DECODE_8BIT},
};

static struct msm_camera_csi2_params as0260_csi_params = {
	.csid_params = {
		.lane_assign = 0xe4,
#ifdef AS0260_MIPI_2_LANE
		.lane_cnt = 2,
#else
		.lane_cnt = 1,
#endif
		.lut_params = {
			.num_cid = 2,
			.vc_cfg = as0260_cid_cfg,
		},
	},
	.csiphy_params = {
#ifdef AS0260_MIPI_2_LANE
		.lane_cnt = 2,
#else
		.lane_cnt = 1,
#endif
		.settle_cnt = 0x17,//0x1B,//0x22,//0x14,
#ifdef AS0260_MIPI_2_LANE
		.lane_mask = 3,
#else
		.lane_mask = 1,
#endif
	},
};

static struct msm_camera_csi2_params *as0260_csi_params_array[] = {
	&as0260_csi_params,
	&as0260_csi_params,
	&as0260_csi_params,
	&as0260_csi_params,
};
#endif

#if 0
static struct msm_sensor_output_reg_addr_t as0260_reg_addr = {
	.x_output = 0xC86C,
	.y_output = 0xC86E,
	.line_length_pclk = 0xC86C,
	.frame_length_lines = 0xC86E,
};
#endif

static struct msm_sensor_id_info_t as0260_id_info = {
	.sensor_id_reg_addr = 0x3000,
	.sensor_id = 0x4580,
};

static const struct i2c_device_id as0260_i2c_id[] = {
	{SENSOR_NAME, (kernel_ulong_t)&as0260_s_ctrl},
	{ }
};

static struct i2c_driver as0260_i2c_driver = {
	.id_table = as0260_i2c_id,
	//.probe  = msm_sensor_i2c_probe,
	.probe = as0260_sensor_i2c_probe,
	.driver = {
		.name = SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client as0260_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

/* msm_sensor_write_res_settings */
int32_t as0260_sensor_write_res_settings(struct msm_sensor_ctrl_t *s_ctrl,
	uint16_t res)
{
	int32_t rc = 0;
SKYCDBG("%s:[F_PANTECH_CAMERA] %d, res=%d\n", __func__, __LINE__, res);
	//////////////////////////
	//1088 only
	//res = 0;
	//////////////////////////
	
	rc = msm_sensor_write_conf_array(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->mode_settings, res);
	if (rc < 0)
		return rc;
#ifndef CONFIG_PANTECH_CAMERA_AS0260
	rc = msm_sensor_write_output_settings(s_ctrl, res);
#endif
	return rc;
}


int32_t as0260_sensor_setting(struct msm_sensor_ctrl_t *s_ctrl,
			int update_type, int res)
{
	int32_t rc = 0;

SKYCDBG("%s:[F_PANTECH_CAMERA] %d, %d res=%d\n", __func__, __LINE__,update_type,res);

	if (update_type == MSM_SENSOR_REG_INIT) {
		msm_sensor_write_init_settings(s_ctrl);
		if (s_ctrl->func_tbl->sensor_stop_stream)
			s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
	} else if (update_type == MSM_SENSOR_UPDATE_PERIODIC) {
#if 0
		 if (s_ctrl->func_tbl->sensor_stop_stream)
			s_ctrl->func_tbl->sensor_stop_stream(s_ctrl);
#endif
 		v4l2_subdev_notify(&s_ctrl->sensor_v4l2_subdev,
			NOTIFY_PCLK_CHANGE, &s_ctrl->msm_sensor_reg->
			output_settings[res].op_pixel_clk);
#if 0
	        if (s_ctrl->func_tbl->sensor_start_stream) {						
			s_ctrl->func_tbl->sensor_start_stream(s_ctrl);
		 }
#endif		
		 as0260_sensor_write_res_settings(s_ctrl, res);
		 msleep(30);

	}
	SKYCDBG("%s: %d x\n", __func__, __LINE__);
	return rc;
}

#ifdef CONFIG_PANTECH_CAMERA
static int as0260_sensor_set_brightness(struct msm_sensor_ctrl_t * s_ctrl,int8_t brightness)
{
	int rc = 0;

	SKYCDBG("%s brightness=%d start \n",__func__,brightness); //SKYCDBG

	if(brightness < 0 || brightness > 8){
		SKYCERR("%s error. brightness=%d\n", __func__, brightness); //SKYCERR
		return -EINVAL;
	}

	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->bright_cfg_settings[brightness],
		s_ctrl->msm_sensor_reg->bright_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}

static int as0260_sensor_set_effect(struct msm_sensor_ctrl_t *s_ctrl ,int8_t effect)
{
	int rc = 0;

	SKYCDBG("%s effect=%d start \n",__func__,effect); //SKYCDBG

	if(effect < CAMERA_EFFECT_OFF || effect >= CAMERA_EFFECT_MAX){
		SKYCERR("%s error. effect=%d\n", __func__, effect); //SKYCERR
		return -EINVAL;
	}

	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->effect_cfg_settings[effect],
		s_ctrl->msm_sensor_reg->effect_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}

static int as0260_sensor_set_exposure_mode(struct msm_sensor_ctrl_t *s_ctrl ,int8_t exposure)
{
	int rc = 0;

	SKYCDBG("%s exposure=%d start \n",__func__,exposure); //SKYCDBG

	if(exposure < 0 || exposure > 3){
		SKYCERR("%s error. exposure=%d\n", __func__, exposure); //SKYCERR
		return -EINVAL;
	}

	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->exposure_mode_cfg_settings[exposure],
		s_ctrl->msm_sensor_reg->exposure_mode_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}

static int as0260_sensor_set_reflect(struct msm_sensor_ctrl_t *s_ctrl ,int8_t reflect)
{
	int rc = 0;

	SKYCDBG("%s reflect=%d start \n",__func__,reflect); //SKYCDBG

	if(reflect < 0 || reflect > 3){
		SKYCERR("%s error. reflect=%d\n", __func__, reflect); //SKYCERR
		return -EINVAL;
	}

	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->reflect_cfg_settings[reflect],
		s_ctrl->msm_sensor_reg->reflect_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}

static int as0260_sensor_set_wb(struct msm_sensor_ctrl_t *s_ctrl ,int8_t wb)
{
	int rc = 0;

	SKYCDBG("%s wb=%d start \n",__func__,wb); //SKYCDBG

	if(wb < 0 || wb > 6){
		SKYCERR("%s error. wb=%d\n", __func__, wb); //SKYCERR
		return -EINVAL;
	}

	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->wb_cfg_settings[wb],
		s_ctrl->msm_sensor_reg->wb_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}

static int as0260_set_aec_lock(struct msm_sensor_ctrl_t *s_ctrl ,int8_t is_lock)
{
	int rc =0;

	SKYCDBG("%s is_lock=%d start \n",__func__,is_lock); //SKYCDBG
	
    rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->set_aec_lock[is_lock],
		s_ctrl->msm_sensor_reg->set_aec_lock_size,
		s_ctrl->msm_sensor_reg->default_data_type);
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
    return rc;
}



static int as0260_sensor_set_preview_fps(struct msm_sensor_ctrl_t *s_ctrl ,int8_t preview_fps)
{
	int rc = 0;

	SKYCDBG("%s preview_fps=%d start \n",__func__,preview_fps); //SKYCDBG

	if(preview_fps < 5 || preview_fps > 31){
		SKYCERR("%s error. preview_fps=%d\n", __func__, preview_fps); //SKYCERR
		return -EINVAL;
	}

	//g_preview_fps = preview_fps;
	
	//if (preview_fps > 30) preview_fps = 30;
	
#if 0//CONFIG_PANTECH_CAMERA_YACD5C1SBDBC for VTS
    if(preview_fps == 24) {
        preview_24fps_for_motion_detect_check = 1;
    }else{
    	rc = msm_camera_i2c_write_tbl(
    		s_ctrl->sensor_i2c_client,
    		s_ctrl->msm_sensor_reg->preview_fps_cfg_settings[preview_fps-5], //original
		//s_ctrl->msm_sensor_reg->preview_fps_cfg_settings[10], //temporary
    		s_ctrl->msm_sensor_reg->preview_fps_cfg_settings_size,
    		s_ctrl->msm_sensor_reg->default_data_type);
    }
#else
	rc = msm_camera_i2c_write_tbl(
		s_ctrl->sensor_i2c_client,
		s_ctrl->msm_sensor_reg->preview_fps_cfg_settings[preview_fps-5],
		s_ctrl->msm_sensor_reg->preview_fps_cfg_settings_size,
		s_ctrl->msm_sensor_reg->default_data_type);
#endif

	if (rc < 0)
	{
		SKYCERR("ERR:%s FAIL!!! rc=%d \n", __func__, rc); //SKYCERR
		return rc;
	}	
	SKYCDBG("%s rc=%d end \n",__func__,rc); //SKYCDBG
	
	return rc;
}



#endif

#ifdef F_AS0260_POWER
static int as0260_vreg_init(void)
{
	int rc = 0;
	SKYCDBG("%s:%d\n", __func__, __LINE__);
	
	rc = sgpio_init(sgpios, CAMIO_MAX);
	if (rc < 0) {
		SKYCERR("%s: sgpio_init failed \n", __func__);
		goto sensor_init_fail;
	}

	rc = svreg_init(svregs, CAMV_MAX);
	if (rc < 0) {
		SKYCERR("%s: svreg_init failed \n", __func__);
		goto sensor_init_fail;
	}

	return rc;

sensor_init_fail:
    return -ENODEV;
}


int32_t as0260_sensor_power_up(struct msm_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	SKYCDBG("%s: %d\n", __func__, __LINE__);

#if 0
	msm_sensor_probe_on(&s_ctrl->sensor_i2c_client->client->dev);
	SKYCDBG("%s msm_sensor_probe_on ok\n", __func__); 
	msm_camio_clk_rate_set(MSM_SENSOR_MCLK_24HZ);
	SKYCDBG("%s msm_camio_clk_rate_set ok\n", __func__);
#else
    rc = msm_sensor_power_up(s_ctrl);
    SKYCDBG(" %s : msm_sensor_power_up : rc = %d E\n",__func__, rc);  
#endif

    	as0260_vreg_init();
	
	if (svreg_ctrl(svregs, CAMV_CORE_1P8V, 1) < 0)	rc = -EIO;
	mdelay(50);
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L)||defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
	if (svreg_ctrl(svregs, CAMV_I2C_1P8V, 1) < 0)	rc = -EIO;
#endif
	if (svreg_ctrl(svregs, CAMV_IO_1P8V, 1) < 0)	rc = -EIO;
	mdelay(1); /* > 20us */
	if (svreg_ctrl(svregs, CAMV_A_2P8V, 1) < 0)	rc = -EIO;
	mdelay(1); /* > 15us */
	if (sgpio_ctrl(sgpios, CAMIO_RST_N, 0) < 0)	rc = -EIO;
	mdelay(1);
//	if (svreg_ctrl(svregs, CAMV_CORE_1P8V, 1) < 0)	rc = -EIO;
//	mdelay(1);
	
	if (sgpio_ctrl(sgpios, CAMIO_STB_N, 0) < 0)	rc = -EIO;
	//msm_camio_clk_rate_set(24000000);
	msleep(10); /* > 50us */
	if (sgpio_ctrl(sgpios, CAMIO_RST_N, 1) < 0)	rc = -EIO;
	mdelay(30);
	//mdelay(1); /* > 50us */

	SKYCDBG("%s X (%d)\n", __func__, rc);
	return rc;

}

int32_t as0260_sensor_power_down(struct msm_sensor_ctrl_t *s_ctrl)
{
    int32_t rc = 0;
	SKYCDBG("%s\n", __func__);

#if 0
	msm_sensor_probe_off(&s_ctrl->sensor_i2c_client->client->dev);
#else
    msm_sensor_power_down(s_ctrl);
    SKYCDBG(" %s : msm_sensor_power_down : rc = %d E\n",__func__, rc);  
#endif

	if (sgpio_ctrl(sgpios, CAMIO_RST_N, 0) < 0)	rc = -EIO;
		mdelay(1); /* > 20 cycles (approx. 0.64us) */
	if (sgpio_ctrl(sgpios, CAMIO_STB_N, 0) < 0)	rc = -EIO;

	if (svreg_ctrl(svregs, CAMV_A_2P8V, 0) < 0)	rc = -EIO;
	if (svreg_ctrl(svregs, CAMV_IO_1P8V, 0) < 0)	rc = -EIO;
#if (defined(CONFIG_MACH_APQ8064_EF51S)||defined(CONFIG_MACH_APQ8064_EF51K)||defined(CONFIG_MACH_APQ8064_EF51L)||defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)||defined(CONFIG_MACH_APQ8064_EF52W))
	if (svreg_ctrl(svregs, CAMV_I2C_1P8V, 0) < 0)	rc = -EIO;
#endif
	if (svreg_ctrl(svregs, CAMV_CORE_1P8V, 0) < 0)	rc = -EIO;

	svreg_release(svregs, CAMV_MAX);
	sgpio_release(sgpios, CAMIO_MAX);

	SKYCDBG("%s X (%d)\n", __func__, rc);
	return rc;

}
#endif


static int32_t as0260_trans_gpio_pm_to_sys(sgpio_ctrl_t *gpios, struct msm_sensor_ctrl_t *s_ctrl, uint32_t gpio_max_num)
{
	int i;
	int rc = 0;
	struct msm_camera_gpio_conf *gpio_conf =
		s_ctrl->sensordata->sensor_platform_info->gpio_conf;
	
	for (i = 0; i < gpio_max_num; i++) {		
		//if(strstr(gpios[i].label, gpio_conf->cam_gpio_common_tbl[i].label))
		if(strstr(gpios[i].label, "PM"))
		{	
			SKYCDBG("%s MATCH gpio string_111~~~[board=%s, %d], [driver=%s, %d]\n", __func__,
				gpio_conf->cam_gpio_req_tbl[i].label, gpio_conf->cam_gpio_req_tbl[i].gpio, gpios[i].label, gpios[i].nr);
			
			gpios[i].nr = gpio_conf->cam_gpio_req_tbl[i].gpio;
			
			SKYCDBG("%s MATCH gpio string_222~~~[board=%s, %d], [driver=%s, %d]\n", __func__,
				gpio_conf->cam_gpio_req_tbl[i].label, gpio_conf->cam_gpio_req_tbl[i].gpio, gpios[i].label, gpios[i].nr);
		}
		else
			SKYCDBG("%s DON'T MATCH gpio string~~~(board=%s, driver=%s)\n", __func__, gpio_conf->cam_gpio_req_tbl[i].label, gpios[i].label);
	}

	return rc;
}

int32_t as0260_sensor_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int rc = 0;
	struct msm_sensor_ctrl_t *s_ctrl;
	SKYCDBG("%s_i2c_probe called\n", client->name);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		SKYCERR("i2c_check_functionality failed\n");
		//rc = -EFAULT;
		return rc;
		//goto probe_fail;
	}

	s_ctrl = (struct msm_sensor_ctrl_t *)(id->driver_data);
	if (s_ctrl->sensor_i2c_client != NULL) {
		s_ctrl->sensor_i2c_client->client = client;
		if (s_ctrl->sensor_i2c_addr != 0)
			s_ctrl->sensor_i2c_client->client->addr =
				s_ctrl->sensor_i2c_addr;
	} else {
		rc = -EFAULT;
		//return rc;
		goto probe_fail;
	}

	s_ctrl->sensordata = client->dev.platform_data;
	if (s_ctrl->sensordata == NULL) {
		SKYCERR("%s %s NULL sensor data\n", __func__, client->name);
		rc = -EFAULT;
		goto probe_fail;
	}

	//We need to change gpio number for using pmic gpio
	as0260_trans_gpio_pm_to_sys(&sgpios[0], s_ctrl, CAMIO_PM_MAX);

	rc = s_ctrl->func_tbl->sensor_power_up(s_ctrl);
	if (rc < 0) {
		SKYCERR("%s %s power up failed\n", __func__, client->name);
		//return rc;
		rc = -EFAULT;
		goto probe_fail;
	}

	if (s_ctrl->func_tbl->sensor_match_id)
		rc = s_ctrl->func_tbl->sensor_match_id(s_ctrl);
	else
		rc = msm_sensor_match_id(s_ctrl);
	if (rc < 0)
		goto probe_fail;	

	snprintf(s_ctrl->sensor_v4l2_subdev.name,
		sizeof(s_ctrl->sensor_v4l2_subdev.name), "%s", id->name);
	v4l2_i2c_subdev_init(&s_ctrl->sensor_v4l2_subdev, client,
		s_ctrl->sensor_v4l2_subdev_ops);

	msm_sensor_register(&s_ctrl->sensor_v4l2_subdev);
	goto i2c_probe_end;
probe_fail:
	SKYCERR("%s_i2c_probe failed\n", client->name);
i2c_probe_end:
	if (rc > 0)
		rc = 0;
	s_ctrl->func_tbl->sensor_power_down(s_ctrl);
	SKYCDBG("%s_probe Success!\n", client->name);
	return rc;
}

static int __init msm_sensor_init_module(void)
{
	return i2c_add_driver(&as0260_i2c_driver);
}

static struct v4l2_subdev_core_ops as0260_subdev_core_ops = {
	//.s_ctrl = msm_sensor_v4l2_s_ctrl,
	//.queryctrl = msm_sensor_v4l2_query_ctrl,
	.ioctl = msm_sensor_subdev_ioctl,
	.s_power = msm_sensor_power,
};

static struct v4l2_subdev_video_ops as0260_subdev_video_ops = {
	.enum_mbus_fmt = msm_sensor_v4l2_enum_fmt,
};

static struct v4l2_subdev_ops as0260_subdev_ops = {
	.core = &as0260_subdev_core_ops,
	.video  = &as0260_subdev_video_ops,
};

static struct msm_sensor_fn_t as0260_func_tbl = {
	.sensor_start_stream = msm_sensor_start_stream,
	.sensor_stop_stream = msm_sensor_stop_stream,
	.sensor_setting = as0260_sensor_setting,//msm_sensor_setting,
	.sensor_set_sensor_mode = msm_sensor_set_sensor_mode,
	.sensor_mode_init = msm_sensor_mode_init,
	.sensor_get_output_info = msm_sensor_get_output_info,
	.sensor_config = msm_sensor_config,
#ifdef F_AS0260_POWER
	.sensor_power_up = as0260_sensor_power_up,
	.sensor_power_down = as0260_sensor_power_down,
#endif
	.sensor_set_effect = as0260_sensor_set_effect,
	.sensor_set_brightness = as0260_sensor_set_brightness,
	.sensor_set_exposure_mode = as0260_sensor_set_exposure_mode,
	.sensor_set_reflect = as0260_sensor_set_reflect,
	.sensor_set_wb = as0260_sensor_set_wb,
	.sensor_set_preview_fps = as0260_sensor_set_preview_fps,
	.sensor_set_aec_lock = as0260_set_aec_lock,
	.sensor_get_csi_params = msm_sensor_get_csi_params,
};

static struct msm_sensor_reg_t as0260_regs = {
	.default_data_type = MSM_CAMERA_I2C_WORD_DATA,
#ifdef F_STREAM_ON_OFF	
	.start_stream_conf = as0260_start_settings,
	.start_stream_conf_size = ARRAY_SIZE(as0260_start_settings),
	.stop_stream_conf = as0260_stop_settings,
	.stop_stream_conf_size = ARRAY_SIZE(as0260_stop_settings),
#endif
	.init_settings = &as0260_init_conf[0],
	.init_size = ARRAY_SIZE(as0260_init_conf),
	.mode_settings = &as0260_confs[0],
	.output_settings = &as0260_dimensions[0],
	.num_conf = ARRAY_SIZE(as0260_confs),
#ifdef CONFIG_PANTECH_CAMERA
	.effect_cfg_settings = &as0260_cfg_effect[0],
	.effect_cfg_settings_size = ARRAY_SIZE(as0260_cfg_effect[0]),
	.bright_cfg_settings = &as0260_cfg_brightness[0],
    .bright_cfg_settings_size = ARRAY_SIZE(as0260_cfg_brightness[0]),
    .exposure_mode_cfg_settings = &as0260_cfg_exposure_mode[0],
    .exposure_mode_cfg_settings_size = ARRAY_SIZE(as0260_cfg_exposure_mode[0]),
    .reflect_cfg_settings = &as0260_cfg_reflect[0],
    .reflect_cfg_settings_size = ARRAY_SIZE(as0260_cfg_reflect[0]),
    .wb_cfg_settings = &as0260_cfg_wb[0],
    .wb_cfg_settings_size = ARRAY_SIZE(as0260_cfg_wb[0]),
    .preview_fps_cfg_settings = &as0260_cfg_preview_fps[0],
    .preview_fps_cfg_settings_size = ARRAY_SIZE(as0260_cfg_preview_fps[0]),
    .set_aec_lock = &as0260_cfg_aec_lock[0],
    .set_aec_lock_size = ARRAY_SIZE(as0260_cfg_aec_lock[0]),
#endif
};

static struct msm_sensor_ctrl_t as0260_s_ctrl = {
	.msm_sensor_reg = &as0260_regs,
//	.msm_sensor_v4l2_ctrl_info = as0260_v4l2_ctrl_info,
//	.num_v4l2_ctrl = ARRAY_SIZE(as0260_v4l2_ctrl_info),
	.sensor_i2c_client = &as0260_sensor_i2c_client,
	.sensor_i2c_addr = 0x90,//0x48,//0x5D,//,
//	.sensor_output_reg_addr = &as0260_reg_addr,
	.sensor_id_info = &as0260_id_info,
	.cam_mode = MSM_SENSOR_MODE_INVALID,
#if 0
	.csi_params = &as0260_csi_params_array[0],
#endif
	.msm_sensor_mutex = &as0260_mut,
	.sensor_i2c_driver = &as0260_i2c_driver,
	.sensor_v4l2_subdev_info = as0260_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(as0260_subdev_info),
	.sensor_v4l2_subdev_ops = &as0260_subdev_ops,
	.func_tbl = &as0260_func_tbl,
	.clk_rate = MSM_SENSOR_MCLK_24HZ,
};

//module_init(msm_sensor_init_module);
late_initcall(msm_sensor_init_module);
MODULE_DESCRIPTION("Aptina FULL HD YUV sensor driver");
MODULE_LICENSE("GPL v2");

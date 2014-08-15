/* Copyright (c) 2012, PANTECH. All rights reserved.
 */

#ifndef _CE1502_V4L2_CFG_H_
#define _CE1502_V4L2_CFG_H_

#define CE1502_EFFECT_MAX	11
#define CE1502_WHITEBALANCE_MAX 6
#define CE1502_BRIGHTNESS_MAX 9
#if defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)
#define CE1502_SCENE_MAX 21
#elif defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L)
#define CE1502_SCENE_MAX 11
#endif
#define CE1502_EXPOSURE_MAX 4
#define CE1502_FLICKER_MAX 4
#define CE1502_ISO_MAX 5


/* register configration */

static const uint8_t ce1502_effect_data[CE1502_EFFECT_MAX] = {
#if 1
	0x00, 	// none
	0x01,	// mono	
	0x02, 	// negative
	0x00, 	// solarize 미지원
	0x03, 	// sepia
	0x04, 	// posterize : have to set 3DH, 07H command
	0x05, 	//whiteboard
	0x07, 	// blackboard
	0x09,	// aqua
	0x06, 	// whiteboard color
	0x08, 	// blackboard color
#else
	0x00, 	// none
	0x01,	// mono	
	0x02, 	// negative
	0x00, 	// solarize 미지원
	0x03, 	// sepia
	0x00, 	// posterize : have to set 3DH, 07H command
	0x04, 	//whiteboard
	0x06, 	// blackboard
	0x08,	// aqua
	0x05, 	//whiteboard
	0x07 	// blackboard
#endif
};

static const uint8_t ce1502_wb_data[CE1502_WHITEBALANCE_MAX] = {
	0x00, 	// AWB
	0x00, 	// CUSTOM
	0x04,	// INCANDESCENT	
	0x03, 	// FLUORESCENT
	0x01, 	// DAYLIGHT
	0x02, 	// CLOUDY_DAYLIGHT
};

static const uint8_t ce1502_bright_data[CE1502_BRIGHTNESS_MAX] = {
	0x00, 	// -4[EV]
	0x01,	// -3[EV]
	0x02, 	// -4[EV]
	0x03, 	// -1[EV]
	0x04, 	//  0[EV]
	0x05, 	// 1[EV]
	0x06,	// 2[EV]
	0x07, 	// 3[EV]
	0x08, 	// 4[EV]
};
#if defined(CONFIG_MACH_APQ8064_EF52S)||defined(CONFIG_MACH_APQ8064_EF52K)||defined(CONFIG_MACH_APQ8064_EF52L)
static const uint8_t ce1502_scene_data[CE1502_SCENE_MAX] = {
    0x00,   //CAMERA_BESTSHOT_OFF = 0,
    0x01,   //CAMERA_BESTSHOT_AUTO = 1,
    0x02,   //CAMERA_BESTSHOT_LANDSCAPE = 2,
    0x03,   //CAMERA_BESTSHOT_SNOW,
    0x04,   //CAMERA_BESTSHOT_BEACH,
    0x05,   //CAMERA_BESTSHOT_SUNSET,
    0x06,   //CAMERA_BESTSHOT_NIGHT,
    0x07,   //CAMERA_BESTSHOT_PORTRAIT,
    0x08,   //CAMERA_BESTSHOT_BACKLIGHT,
    0x09,   //CAMERA_BESTSHOT_SPORTS,
    0x0a,   //CAMERA_BESTSHOT_ANTISHAKE,
    0x0b,   //CAMERA_BESTSHOT_FLOWERS,
    0x0c,   //CAMERA_BESTSHOT_CANDLELIGHT,
    0x0d,   //CAMERA_BESTSHOT_FIREWORKS,
    0x0e,   //CAMERA_BESTSHOT_PARTY,
    0x0f,   //CAMERA_BESTSHOT_NIGHT_PORTRAIT,
    0x10,   //CAMERA_BESTSHOT_THEATRE,
    0x11,   //CAMERA_BESTSHOT_ACTION,
    0x12,   //CAMERA_BESTSHOT_AR,
    0x13,   //CAMERA_BESTSHOT_INDOOR,
    0x14,   //CAMERA_BESTSHOT_TEXT,
};
#elif defined(CONFIG_MACH_APQ8064_EF51S) || defined(CONFIG_MACH_APQ8064_EF51K) || defined(CONFIG_MACH_APQ8064_EF51L) //for EF51
static const uint8_t ce1502_scene_data[CE1502_SCENE_MAX] = {
	0x00, 	// none
	0x00,	// Potrait	
	0x01, 	// LandScape
	0x02, 	// Indoor
	0x03, 	//  Sports
	0x04, 	// Night
	0x05,	// Beach
	0x06, 	// Snow
	0x07, 	// Sunset
	0x08, 	// TEXT
	0x09, 	// Party
};
#endif
static const uint8_t ce1502_exposure_data[CE1502_EXPOSURE_MAX] = {
	0x01, 	// CENTER WEIGHT (normal)
	0x00, 	// AVERAGE
	0x01,	// CENTER WEIGHT	
	0x02, 	// SPOT
};

static const uint8_t ce1502_flicker_data[CE1502_FLICKER_MAX] = {
	0x00, 	// OFF
	0x03,	// 60Hz
	0x02, 	// 50HZ
	0x01, 	// AUTO
};

static const uint8_t ce1502_iso_data[CE1502_ISO_MAX] = {
	0x00, 	// AUTO
	0x02,	// 100
	0x03, 	// 200
	0x04, 	// 400
	0x05, 	// 800
};

#endif /* _CE1502_V4L2_CFG_H_ */
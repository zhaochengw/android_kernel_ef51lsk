/*
 * Core Header for:
 * Cypress TrueTouch(TM) Standard Product (TTSP) touchscreen drivers.
 * For use with Cypress Gen4 and Solo parts.
 * Supported parts include:
 * CY8CTMA768
 * CY8CTMA4XX
 *
 * Copyright (C) 2012 Pantech, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, and only version 2, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contact Cypress Semiconductor at www.cypress.com <kev@cypress.com>
 *
 */

#define ACT_INTRVL0                             (0x0013)
#define LP_INTRVL0                              (0x0016)
#define ACT_DIST0                               (0x0011)
#define ACT_DIST2                               (0x0054)
#define X_RESOLUTION                            (0x019C)
#define Y_RESOLUTION                            (0x01A0)

#define T_COMP_ENABLE                           (0x0094)
#define T_COMP_INTERVAL                         (0x0098)
#define T_COMP_RECAL_MUTUAL_SENSOR_LIMIT        (0x009C)
#define T_COMP_RECAL_MUTUAL_HIGH                (0x0000)
#define T_COMP_RECAL_MUTUAL_LOW                 (0x0000)
#define T_COMP_RECAL_SELF_SENSOR_LIMIT          (0x0000)
#define T_COMP_RECAK_SELF_HIGH                  (0x0000)
#define T_COMP_RECAK_SELF_LOW                   (0x0000)
#define FINGER_ID_MAX_FINGER_VELOCITY2          (0x0164)
#define MAX_FAT_FINGER_SIZE                     (0x0195)
#define MIN_FAT_FINGER_SIZE                     (0x0196)
#define FINGER_THRESH_MUTUAL                    (0x0197)
#define FINGER_THRESH_SELF                      (0x0198)
#define INNER_EDGE_GAIN                         (0x019A)
#define OUTER_EDGE_GAIN                         (0x019B)
#define BYPASS_THRESHOLD_GAIN                   (0x01AD)
#define BYPASS_THRESHOLD_EDGE_GAIN              (0x01AE)
#define MULTI_TOUCH_DEBOUNCE                    (0x01B2)

#define CHARGER_ARMOR_ENABLE                    (0x00B4)
#define CA_NUM_SUB_CONV_BASE_MUTUAL             (0x0000)
#define CA_ALT_NUM_SUB_CONV_MUTUAL              (0x00F0)
#define CA_BLOCK_NOISE_THRESHOLD                (0x00C4)
#define CA_NUM_SUB_CONV_BASE_SELF               (0x0000)
#define CA_ALT_NUM_SUB_CONV_SELF                (0x00E0)
#define XY_FILTER_MASK_CA                       (0x013C)
#define XY_FILT_IIR_COEFF_CA                    (0x0140)
#define XY_FILT_Z_IIR_COEFF_CA                  (0x0144)
#define XY_FILT_XY_FAST_THR_CA                  (0x0148)
#define XY_FILT_XY_SLOW_THR_CA                  (0x0149)
#define XY_FILT_ADAPTIVE_IIR_FILTER             (0x014C)
#define XY_FILT_ADAPTIVE_IIR_FILTER_DISTANCE    (0x014D)
#define RAW_FILTER_MASK_CA                      (0x01FC)
#define RAW_FILT_IIR_COEFF_MUTUAL_CA            (0x01FE)
#define RAW_FILT_IIR_THRESHOLD_MUTUAL_CA        (0x01FF)
#define DETECT_CHARGER_THRESHOLD                (0x03D0)

#define XY_FILTER_MASK                          (0x012C)
#define XY_FILT_IIR_COEFF                       (0x0130)
#define XY_FILT_Z_IIR_COEFF                     (0x0134)
#define XY_FILT_XY_FAST_THR                     (0x0138)
#define XY_FILT_XY_SLOW_THR                     (0x0139)
#define XY_FILT_IIR_FAST_COEFF                  (0x013A)
#define XY_FILT_TOUCH_SIZE_IIR_COEFF            (0x014E)
#define RAW_FILTER_MASK                         (0x01F0)
#define RAW_FILT_IIR_COEFF_MUTUAL               (0x01F2)
#define RAW_FILT_IIR_THRESHOLD_MUTUAL           (0x01F3)
#define BL_DELAY_MUT                            (0x0214)
#define BL_DELAY_SELF                           (0x0215)
#define BL_THR_MUT                              (0x0218)
#define BL_THR_SELF                             (0x0219)
#define BL_H20_RJCT                             (0x021C)
#define CMF_THR_MUT                             (0x0224)
#define CMF_THR_SELF                            (0x0225)

#define SCALE_MUT                               (0x03AC)
#define SCALE_SELF                              (0x03B0)

/*
 * Core Source for:
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

#include "cyttsp4_pm8921.h"

int hw_on_off(struct device *dev, int on)
{
    struct regulator *vreg_touch_3_3, *vreg_touch_1_8;
    int rc =0;
    static int vreg_touch_1_8_state = false;
    static int vreg_touch_3_3_state = false;
#if defined(CONFIG_MACH_APQ8064_EF50L)
#if CONFIG_BOARD_VER < CONFIG_WS20
    struct regulator *vreg_touch_1_8_50L;
    vreg_touch_1_8_50L = regulator_get(NULL, "8921_lvs5");
    rc = regulator_enable(vreg_touch_1_8_50L);
    regulator_put(vreg_touch_1_8_50L);
#endif /* --CONFIG_BOARD_VER < CONFIG_WS20 */
#endif /* --CONFIG_MACH_APQ8064_EF50L */

    if(on != vreg_touch_1_8_state) {
        vreg_touch_1_8 = regulator_get(NULL, TOUCH_POWER_VCC);
        if(on) {
            rc = regulator_set_voltage(vreg_touch_1_8, 1900000, 1900000);
            if (rc) { 
                dev_err(dev, "%s: set_voltage 8921_l21_1p8 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
            rc = regulator_enable(vreg_touch_1_8);
            if (rc) { 
                dev_err(dev, "%s: regulator_enable vreg_touch_1_8 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        else {
            rc = regulator_disable(vreg_touch_1_8);
            if (rc) { 
                dev_err(dev, "%s: regulator_disable vreg_touch_1_8 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        regulator_put(vreg_touch_1_8);
        vreg_touch_1_8_state = on;
    }

    if(on != vreg_touch_3_3_state) {
        vreg_touch_3_3 = regulator_get(NULL, TOUCH_POWER_VDD);
        if(on) {
            rc = regulator_set_voltage(vreg_touch_3_3, 2900000, 2900000);
            if (rc) { 
                dev_err(dev, "%s: set_voltage 8921_l17_2p8 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
            rc = regulator_enable(vreg_touch_3_3);
            if (rc) { 
                dev_err(dev, "%s: regulator_enable vreg_touch_3_3 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        else {
            rc = regulator_disable(vreg_touch_3_3);
            if (rc) { 
                dev_err(dev, "%s: regulator_disable vreg_touch_3_3 failed, rc=%d\n", __func__, rc);
                return -EINVAL;
            }
        }
        regulator_put(vreg_touch_3_3);
        vreg_touch_3_3_state = on;
    }
    msleep(5);
    return 0;
}
EXPORT_SYMBOL_GPL(hw_on_off);

int init_hw_setting(struct device *dev)
{
    int rc =0;
    
    rc = hw_on_off(dev, true);
    if(rc<0) {
        dev_err(dev, "%s: fail to turn on power. rc=%d\n", __func__, rc);
        return rc;
    }

#if CONFIG_BOARD_VER == CONFIG_PT10 
    rc = gpio_request(PM8921_GPIO_PM_TO_SYS(GPIO_TOUCH_LDO1), "touch_ldo1");
    if (rc) {
        dev_err(dev, "%s: gpio_request GPIO_TOUCH_LDO1 failed, rc=%d\n",__func__, rc);
        return -EINVAL;
    }
    rc = gpio_direction_output(PM8921_GPIO_PM_TO_SYS(GPIO_TOUCH_LDO1), 1);
    if (rc) {
        dev_err(dev, "%s: gpio_direction_output GPIO_TOUCH_LDO1 failed, rc=%d\n", __func__, rc);
        return -EINVAL;
    }
    rc = gpio_request(PM8921_GPIO_PM_TO_SYS(GPIO_TOUCH_LDO2), "touch_ldo2");
    if (rc) {
        dev_err(dev, "%s: gpio_request GPIO_TOUCH_LDO2 failed, rc=%d\n", __func__, rc);
        return -EINVAL;
    }
    rc = gpio_direction_output(PM8921_GPIO_PM_TO_SYS(GPIO_TOUCH_LDO2), 1);
    if (rc) {
        dev_err(dev, "%s: gpio_direction_output GPIO_TOUCH_LDO2 failed, rc=%d\n",__func__, rc);
        return -EINVAL;
    }
#endif

    rc = gpio_request(GPIO_TOUCH_RST, "touch_rst");
    if (rc) {
        gpio_free(GPIO_TOUCH_RST);
        rc = gpio_request(GPIO_TOUCH_RST, "touch_rst");
        if (rc) {
            dev_err(dev, "%s: gpio_request GPIO_TOUCH_RST : %d failed, rc=%d\n",__func__, GPIO_TOUCH_RST, rc);
        }
    }

    rc = gpio_direction_output(GPIO_TOUCH_RST, 1);
    if (rc) {
        dev_err(dev, "%s: gpio_direction_output GPIO_TOUCH_RST : %d failed, rc=%d\n",__func__, GPIO_TOUCH_RST, rc);
    }

    rc = gpio_request(GPIO_TOUCH_CHG, "touch_chg");
    if (rc) {
        gpio_free(GPIO_TOUCH_CHG);
        rc = gpio_request(GPIO_TOUCH_CHG, "touch_chg");
        if (rc) {
            dev_err(dev, "%s: gpio_request GPIO_TOUCH_CHG : %d failed, rc=%d\n",__func__, GPIO_TOUCH_CHG, rc);
        }
    }  

    rc = gpio_direction_input(GPIO_TOUCH_CHG);
    if (rc) {
        dev_err(dev, "%s: gpio_direction_input gpio_chg : %d failed, rc=%d\n",__func__, GPIO_TOUCH_CHG, rc);
    }

    msleep(5);
    return 0;
}
EXPORT_SYMBOL_GPL(init_hw_setting);

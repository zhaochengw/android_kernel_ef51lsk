/* Copyright (c) 2011, PANTECH. All rights reserved.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef _PTUNE_PARSER_H_
#define _PTUNE_PARSER_H_

#include "sensor_i2c.h"
#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
#include "msm_camera_i2c.h"
#endif

#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
msm_camera_i2c_reg_tune_t *ptune_parse(const char *pname, const char *fbuf);
#else
si2c_cmd_t *ptune_parse(const char *pname, const char *fbuf);
#endif

#endif /* _PTUNE_PARSER_H_ */

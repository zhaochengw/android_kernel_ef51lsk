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

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include "ptune_parser.h"
#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
#include "msm_sensor.h"
#endif

/* PANTECH_CAMERA_TODO */
#undef CDBG
#undef CERR
#define CDBG(fmt, args...) do{}while(0) //printk(KERN_DEBUG fmt, ##args)
#define CINFO(fmt, args...) printk(KERN_INFO fmt, ##args)
#define CERR(fmt, args...) printk(KERN_ERR fmt, ##args)


#define PTUNE_TOKEN_LEN		16	// len of command token
#define PTUNE_PNAME_LEN		64	// len of 'start of parameter' mark


/**
 * Simple config file needs complex parser, so we apply STRICT format to
 * config file as below.
 * - Each token should be separated by comma, and each line should have exact
 *   number of tokens.
 * - If the line starts with "//" or blank characters (' ', '\t'), we regard
 *   current line as comment and discard current line.
 * - Even if we find just one line of malformed command, we return error.
 */


static inline int ptune_isblank(char c)
{
	return (c == ' ' || c == '\t');
}

static inline int ptune_isdigit(char c)
{
	return ((c >= '0') && (c <= '9'));
}

static inline int ptune_ishex(char c)
{
	return ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

static inline int ptune_isxdigit(char c)
{
	return ptune_isdigit(c) || ptune_ishex(c);
}

static inline uint32_t ptune_hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
		
	return 0;
}

static char *ptune_get_nl(char *str);
static char *ptune_get_bop(const char *str, const char *pname);
static char *ptune_get_eop(const char *str, const char *pname);

#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
static int ptune_parse_cmd(char *str, msm_camera_i2c_reg_tune_t *cmd);
extern int tuner_line;
#else
static int ptune_parse_cmd(char *str, si2c_cmd_t *cmd);
static int ptune_parse_id(char *str, si2c_cid_t *id);
static int ptune_parse_width(char *str, si2c_cw_t *w);
#endif
static int ptune_parse_num(char *str, uint32_t *num);

static uint32_t ptune_count_nol(void);
static char *ptune_copy_token(char *str, char *token);
static int ptune_atoi(char *str, uint32_t *num);


static char *ptune_bop = NULL;	// begin of parameter
static char *ptune_eop = NULL;	// end of parameter


/**
 * PANTECH_CAMERA_TODO
 * Reads configuration file and translates these to command array then return
 * to caller. (Caller should release this array after use.)
 */
#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
msm_camera_i2c_reg_tune_t *ptune_parse(const char *pname, const char *fbuf)
{
	msm_camera_i2c_reg_tune_t *cmds = NULL;

	uint32_t nol = 0;	// # of valid lines
	uint32_t noc = 0;	// # of commands (nol == noc)
	char *p = NULL;

	CINFO("%s E pname=%s\n", __func__, pname ? : "null");

	if (!pname || !fbuf) {
		CERR("%s pname/fbuf=null\n", __func__);
		return NULL;
	}

	ptune_bop = NULL;
	ptune_eop = NULL;

	ptune_bop = ptune_get_bop(fbuf, pname);
	if (!ptune_bop)
		goto load_fail;

	ptune_eop = ptune_get_eop(fbuf, pname);
	if (!ptune_eop)
		goto load_fail;

	if (ptune_bop >= ptune_eop) {
		CERR("%s invalid BOP/EOP\n", __func__);
		goto load_fail;
	}

	/* Counts # of lines (=commands). */
	nol = ptune_count_nol();
	if (nol == 0)
		goto load_fail;

	/* Allocates command array and return to caller. So caller SHOULD
	 * release this after use. */
	cmds = (msm_camera_i2c_reg_tune_t *)kmalloc(sizeof(msm_camera_i2c_reg_tune_t) * nol, GFP_KERNEL);

	if (!cmds) {
		CERR("%s no mem\n", __func__);
		goto load_fail;
	}

	/* Parses line by line (only valid lines), and completes command 
	 * array. */
	p = ptune_bop;
	for (;;) {
		p = ptune_get_nl(p);
		if (!p)
			goto load_fail;
		if (p == ptune_eop) {
			CDBG("%s EOP\n", __func__);
			break;
		}
		if (ptune_parse_cmd(p, &cmds[noc]) < 0)
			goto load_fail;

		noc += 1;
	}

	/* # of lines should be same with # of commands. We think specified 
	 * parameter is eventually not valid even if there're just one invalid
	 * line (=invalid command). */
	if (nol != noc) {
		CERR("%s nol != noc\n", __func__);
		goto load_fail;
	}

#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
    tuner_line = noc;
#else
	/* Last entry should be SI2C_EOC (end of commands). */
	if (cmds[noc-1].id != SI2C_EOC) {
		CERR("%s no EOC\n", __func__);
		goto load_fail;
	}
#endif

	CINFO("%s X found %d commands\n", __func__, noc);
	return cmds;

load_fail:

	if (cmds)
		kfree(cmds);

	CERR("%s failed\n", __func__);
	return NULL;
}

#else
si2c_cmd_t *ptune_parse(const char *pname, const char *fbuf)
{
	si2c_cmd_t *cmds = NULL;

	uint32_t nol = 0;	// # of valid lines
	uint32_t noc = 0;	// # of commands (nol == noc)
	char *p = NULL;

	CINFO("%s E pname=%s\n", __func__, pname ? : "null");

	if (!pname || !fbuf) {
		CERR("%s pname/fbuf=null\n", __func__);
		return NULL;
	}

	ptune_bop = NULL;
	ptune_eop = NULL;

	ptune_bop = ptune_get_bop(fbuf, pname);
	if (!ptune_bop)
		goto load_fail;

	ptune_eop = ptune_get_eop(fbuf, pname);
	if (!ptune_eop)
		goto load_fail;

	if (ptune_bop >= ptune_eop) {
		CERR("%s invalid BOP/EOP\n", __func__);
		goto load_fail;
	}

	/* Counts # of lines (=commands). */
	nol = ptune_count_nol();
	if (nol == 0)
		goto load_fail;

	/* Allocates command array and return to caller. So caller SHOULD
	 * release this after use. */
	cmds = (si2c_cmd_t *)kmalloc(sizeof(si2c_cmd_t) * nol, GFP_KERNEL);

	if (!cmds) {
		CERR("%s no mem\n", __func__);
		goto load_fail;
	}

	/* Parses line by line (only valid lines), and completes command 
	 * array. */
	p = ptune_bop;
	for (;;) {
		p = ptune_get_nl(p);
		if (!p)
			goto load_fail;
		if (p == ptune_eop) {
			CDBG("%s EOP\n", __func__);
			break;
		}
		if (ptune_parse_cmd(p, &cmds[noc]) < 0)
			goto load_fail;

		noc += 1;
	}

	/* # of lines should be same with # of commands. We think specified 
	 * parameter is eventually not valid even if there're just one invalid
	 * line (=invalid command). */
	if (nol != noc) {
		CERR("%s nol != noc\n", __func__);
		goto load_fail;
	}

	/* Last entry should be SI2C_EOC (end of commands). */
	if (cmds[noc-1].id != SI2C_EOC) {
		CERR("%s no EOC\n", __func__);
		goto load_fail;
	}

	CINFO("%s X found %d commands\n", __func__, noc);
	return cmds;

load_fail:

	if (cmds)
		kfree(cmds);

	CERR("%s failed\n", __func__);
	return NULL;
}
#endif

/**
 * Returns address of 'start of next line'. If next line is comment, blank or
 * malformed command, just return null.
 */
static char *ptune_get_nl(char *str)
{
	CDBG("%s E\n", __func__);

	if (!str) {
		/* PANTECH_CAMERA_TODO, assert? */
		CERR("%s str=null\n", __func__);
		return NULL;
	}

	for (;;) {
		str = strchr(str, '\n');
		if (!str) {
			/* PANTECH_CAMERA_TODO, assert? */
			CERR("%s end of string\n", __func__);
			return NULL;
		}
		str++;

		if ((*str == '/') && (*(str + 1) == '/')) {
			CDBG("%s comment\n", __func__);
			continue;
		}
		else if (ptune_isblank(*str)) {
			CDBG("%s blank\n", __func__);
			continue;
		}
		else
			break;
	}

	CDBG("%s X\n", __func__);
	return str;
}


/**
 * Returns address of 'parameter mark'.
 */
static char *ptune_get_bop(const char *str, const char *pname)
{
	CDBG("%s E pname=%s\n", __func__, pname ? : "null");

	if (!str || !pname) {
		CERR("%s str/pname=null\n", __func__);
		return NULL;
	}

	str = strstr(str, pname);
	if (!str) {
		CERR("%s can't find %s\n", __func__, pname);
		return NULL;
	}

	CDBG("%s X\n", __func__);
	return (char *)str;
}


/**
 * Returns address of 'end of parameter mark' (pname##_end).
 */
static char *ptune_get_eop(const char *str, const char *pname)
{
	char eop_mark[PTUNE_PNAME_LEN + 16];

	CDBG("%s E pname=%s\n", __func__, pname ? : "null");

	if (!str || !pname) {
		CERR("%s str/pname=null\n", __func__);
		return NULL;
	}

	if ((strlen(pname) + 1) > PTUNE_PNAME_LEN) {
		CERR("%s pname is too long\n", __func__);
		return NULL;
	}

	(void)strcpy(eop_mark, pname);
	(void)strcat(eop_mark, "_end");

	str = strstr(str, eop_mark);
	if (!str) {
		CERR("%s can't find %s\n", __func__, eop_mark);
		return NULL;
	}

	CDBG("%s X\n", __func__);
	return (char *)str;
}


/**
 * Parses current line and completes command structue.
 */
#ifdef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
static int ptune_parse_cmd(char *str, msm_camera_i2c_reg_tune_t *cmd)
{
	char token[PTUNE_TOKEN_LEN];
	uint32_t num = 0;

	CDBG("%s E\n", __func__);

	if (!str || !cmd) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}
    
	/* parse addr */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no addr\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_num(token, &num) < 0) {
		CERR("%s parse err (addr)\n", __func__);
		return -EFAULT;
	}
	cmd->reg_addr = (uint16_t)num;    

	/* parse data */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no data\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_num(token, &num) < 0) {
		CERR("%s parse err (data)\n", __func__);
		return -EFAULT;
	}
	cmd->reg_data = (uint16_t)num;    

	CDBG("%s X\n", __func__);
	return 0;
}

#else
static int ptune_parse_cmd(char *str, si2c_cmd_t *cmd)
{
	char token[PTUNE_TOKEN_LEN];
	uint32_t num = 0;

	CDBG("%s E\n", __func__);

	if (!str || !cmd) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}
	/* parse command id */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no command id\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_id(token, &(cmd->id)) < 0) {
		CERR("%s parse err (command id)\n", __func__);
		return -EFAULT;
	}

	/* parse width */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no width\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_width(token, &(cmd->w)) < 0) {
		CERR("%s parse err (width)\n", __func__);
		return -EFAULT;
	}
	/* parse addr */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no addr\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_num(token, &num) < 0) {
		CERR("%s parse err (addr)\n", __func__);
		return -EFAULT;
	}
	cmd->a = (uint16_t)num;    

	/* parse data */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no data\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_num(token, &num) < 0) {
		CERR("%s parse err (data)\n", __func__);
		return -EFAULT;
	}
	cmd->d = (uint16_t)num;    
	/* parse arg */
	str = ptune_copy_token(str, token);
	if (!str) {
		CERR("%s no arg\n", __func__);
		return -EFAULT;
	}
	if (ptune_parse_num(token, &num) < 0) {
		CERR("%s parse err (arg)\n", __func__);
		return -EFAULT;
	}
	cmd->arg = num;
	CDBG("%s X\n", __func__);
	return 0;
}
#endif

#ifndef CONFIG_PANTECH_CAMERA_YACD5C1SBDBC
/**
 * Translates string to command id.
 */
static int ptune_parse_id(char *str, si2c_cid_t *id)
{
	const char *ids[SI2C_CID_MAX] = {
		"SI2C_NOP",
		"SI2C_EOC",
		"SI2C_WR",
		"SI2C_BWR",
		"SI2C_RD",
		"SI2C_DELAY",
		"SI2C_POLL",
		"SI2C_POLLN",
		"SI2C_POLL_B1AND",
		"SI2C_POLL_B1OR",
		"SI2C_POLL_B0AND",
		"SI2C_POLL_B0OR",
		"SI2C_POLLV",
	};
	uint32_t i = 0;

	CDBG("%s E %s\n", __func__, str ? : "null");

	if (!str || !id) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < SI2C_CID_MAX; i++) {
		if (!strcmp(str, ids[i])) {
			CDBG("%s found %s\n", __func__, ids[i]);
			break;
		}
	}

	if (i == SI2C_CID_MAX) {
		CERR("%s (-EFAULT)\n", __func__);
		return -EFAULT;
	}

	*id = (si2c_cid_t)i;

	CDBG("%s X\n", __func__);
	return 0;
}


/**
 * Translates string to command width.
 */
static int ptune_parse_width(char *str, si2c_cw_t *width)
{
	const char *widths[SI2C_WIDTH_MAX] = {
		"SI2C_A1D1",
		"SI2C_A1D2",
		"SI2C_A2D1",
		"SI2C_A2D2",
	};
	uint32_t i = 0;

	CDBG("%s E %s\n", __func__, str ? : "null");

	if (!str || !width) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < SI2C_WIDTH_MAX; i++) {
		if (!strcmp(str, widths[i])) {
			CDBG("%s found %s\n", __func__, widths[i]);
			break;
		}
	}

	if (i == SI2C_WIDTH_MAX) {
		CERR("%s (-EFAULT)\n", __func__);
		return -EFAULT;
	}

	*width = (si2c_cw_t)i;

	CDBG("%s X\n", __func__);
	return 0;
}
#endif

/**
 * Translates string to numbers (address/data/argument).
 */
static int ptune_parse_num(char *str, uint32_t *num)
{
	CDBG("%s E str=%s\n", __func__, str ? : "null");

	if (!str || !num) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}

	for (;;) {
		if (*str == '\n') {
			CERR("%s (-EFAULT)\n", __func__);
			return -EFAULT;
		}
		if (ptune_isxdigit(*str))
			break;

		str++;
	}

	if (ptune_atoi(str, num) < 0) {
		CERR("%s failed atoi\n", __func__);
		return -EFAULT;
	}

	CDBG("%s X hex=0x%08x, dec=%d\n", __func__, *num, *num);
	return 0;
}


/**
 * Returns number of lines except comments, blanks and malformed commands. 
 */
static uint32_t ptune_count_nol(void)
{
	char *p = NULL;
	uint32_t nol = 0;

	CDBG("%s E\n", __func__);

	p = ptune_bop;
	for (;;) {
		p = ptune_get_nl(p);
		if (!p) {
			/* PANTECH_CAMERA_TODO, assert? */
			CERR("%s no lines\n", __func__);
			return 0;
		}
		if (p == ptune_eop) {
			CDBG("%s found EOP\n", __func__);
			break;
		}
		nol += 1;
	}

	CDBG("%s X found %d lines\n", __func__, nol);
	return nol;
}


/**
 * Parses tokens at current position. Tokens end with character ','. Returns
 * address of 'start of next token'.
 */
static char *ptune_copy_token(char *str, char *token)
{
	uint32_t token_len = 0;

	CDBG("%s E\n", __func__);

	if (!str) {
		CERR("%s str=null\n", __func__);
		return NULL;
	}

	if (!token) {
		CERR("%s token=null\n", __func__);
		return NULL;
	}

	for (;;) {
		if (*str == '\n') {
			CERR("%s malformed command\n", __func__);
			return NULL;
		}
		if (ptune_isblank(*str) || *str == '{') {
			str++;
			continue;
		}
		*token = *str;
		if ((*str == ',') || (*str == '}')) {
			*token = 0;
			break;
		}

		str++;
		token++;
		token_len++;
	}

	if (token_len > PTUNE_TOKEN_LEN) {
		CERR("%s invalid token\n", __func__);
		return NULL;
	}

	str++;

	CDBG("%s X\n", __func__);
	return str;
}


/**
 * Translates ascii string to 'unsigned' integer.
 */
static int ptune_atoi(char *str, uint32_t *num)
{
	uint32_t val = 0;

	CDBG("%s E str=%s\n", __func__, str ? : "null");

	if (!str || !num) {
		CERR("%s (-EINVAL)\n", __func__);
		return -EINVAL;
	}

	if (str[0] == '0' && str[1] == 'x') {
		/* hex */
		str += 2;
		while (*str && ptune_isxdigit(*str))
			val = val * 16 + ptune_hexval(*str++);
	} else {
		/* decimal */
		while (*str && ptune_isdigit(*str))
			val = val * 10 + *str++ - '0';
	}

	*num = val;

	CDBG("%s X hex=0x%08x, dec=%d", __func__, val, val);
	return 0;
}

/* drivers/misc/pantech_apanic.c
 *
 * Copyright (C) 2011 PANTECH, Co. Ltd.
 * based on drivers/misc/apanic.c
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.      See the
 * GNU General Public License for more details.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/notifier.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/preempt.h>
#include <asm/cacheflush.h>
#include <asm/system.h>
#include <linux/fb.h>
#include <linux/time.h>

#include "smd_private.h"
#include "modem_notifier.h"

#include <linux/io.h>
#include <linux/delay.h>
#include <linux/nmi.h>
#include <mach/msm_iomap.h>

#include <linux/kobject.h>
#include <linux/sysfs.h>




#define BITMAP_FONT_SUPPORT
#define FONT_24X31_SUPPORT

#ifdef BITMAP_FONT_SUPPORT
#ifdef FONT_24X31_SUPPORT
#define FONT_WIDTH		24
#define FONT_HEIGHT		31
#include "font24x31.h"
#else
#define FONT_WIDTH		31
#define FONT_HEIGHT     45
#include "font32x45.h"
#endif
#endif
extern void force_mdp_on(void);
extern void force_mdp4_overlay_unset(void);
extern struct fb_info *registered_fb[FB_MAX];
static atomic_t is_displayed = ATOMIC_INIT(0);
static int cx, cy, cmaxx, cmaxy;
#ifdef CONFIG_FB_MSM_DEFAULT_DEPTH_RGBA8888
static const unsigned int BGCOLOR = 0x00FF0000;
static const unsigned int FGCOLOR = 0x00FFFFFF;
static const unsigned int REDCOLOR = 0x000000FF;
#else
static const unsigned short BGCOLOR = 0x001F;
static const unsigned short FGCOLOR = 0xFFFF;
#endif

static const unsigned font5x12[] = {
  0x00000000, 0x00000000,
  0x08421080, 0x00020084,
  0x00052940, 0x00000000,
  0x15f52800, 0x0000295f,
  0x1c52f880, 0x00023e94,
  0x08855640, 0x0004d542,
  0x04528800, 0x000b2725,
  0x00021080, 0x00000000,
  0x04211088, 0x00821042,
  0x10841082, 0x00221108,
  0x09575480, 0x00000000,
  0x3e420000, 0x00000084,
  0x00000000, 0x00223000,
  0x3e000000, 0x00000000,
  0x00000000, 0x00471000,
  0x08844200, 0x00008442,
  0x2318a880, 0x00022a31,
  0x08429880, 0x000f9084,
  0x1108c5c0, 0x000f8444,
  0x1c4443e0, 0x00074610,
  0x14a62100, 0x000423e9,
  0x26d087e0, 0x00074610,
  0x1e10c5c0, 0x00074631,
  0x088443e0, 0x00010844,
  0x1d18c5c0, 0x00074631,
  0x3d18c5c0, 0x00074610,
  0x08e20000, 0x00471000,
  0x08e20000, 0x00223000,
  0x02222200, 0x00082082,
  0x01f00000, 0x000003e0,
  0x20820820, 0x00008888,
  0x1108c5c0, 0x00020084,
  0x2b98c5c0, 0x000f05b5,
  0x2318a880, 0x0008c63f,
  0x1d2949e0, 0x0007ca52,
  0x0210c5c0, 0x00074421,
  0x252949e0, 0x0007ca52,
  0x1e1087e0, 0x000f8421,
  0x1e1087e0, 0x00008421,
  0x0210c5c0, 0x00074639,
  0x3f18c620, 0x0008c631,
  0x084211c0, 0x00071084,
  0x10842380, 0x00032508,
  0x0654c620, 0x0008c525,
  0x02108420, 0x000f8421,
  0x2b5dc620, 0x0008c631,
  0x2b59ce20, 0x0008c739,
  0x2318c5c0, 0x00074631,
  0x1f18c5e0, 0x00008421,
  0x2318c5c0, 0x01075631,
  0x1f18c5e0, 0x0008c525,
  0x1c10c5c0, 0x00074610,
  0x084213e0, 0x00021084,
  0x2318c620, 0x00074631,
  0x1518c620, 0x0002114a,
  0x2b18c620, 0x000556b5,
  0x08a54620, 0x0008c54a,
  0x08a54620, 0x00021084,
  0x088443e0, 0x000f8442,
  0x0421084e, 0x00e10842,
  0x08210420, 0x00084108,
  0x1084210e, 0x00e42108,
  0x0008a880, 0x00000000,
  0x00000000, 0x01f00000,
  0x00000104, 0x00000000,
  0x20e00000, 0x000b663e,
  0x22f08420, 0x0007c631,
  0x22e00000, 0x00074421,
  0x23e84200, 0x000f4631,
  0x22e00000, 0x0007443f,
  0x1e214980, 0x00010842,
  0x22e00000, 0x1d187a31,
  0x26d08420, 0x0008c631,
  0x08601000, 0x00071084,
  0x10c02000, 0x0c94a108,
  0x0a908420, 0x0008a4a3,
  0x084210c0, 0x00071084,
  0x2ab00000, 0x0008d6b5,
  0x26d00000, 0x0008c631,
  0x22e00000, 0x00074631,
  0x22f00000, 0x0210be31,
  0x23e00000, 0x21087a31,
  0x26d00000, 0x00008421,
  0x22e00000, 0x00074506,
  0x04f10800, 0x00064842,
  0x23100000, 0x000b6631,
  0x23100000, 0x00022951,
  0x23100000, 0x000556b5,
  0x15100000, 0x0008a884, 
  0x23100000, 0x1d185b31,
  0x11f00000, 0x000f8444,
  0x06421098, 0x01821084,
  0x08421080, 0x00021084,
  0x30421083, 0x00321084,
  0x0004d640, 0x00000000,
  0x00000000, 0x00000000, 
};

#ifdef CONFIG_FB_MSM_DEFAULT_DEPTH_RGBA8888
#ifdef BITMAP_FONT_SUPPORT
static void drawglyph(unsigned int *base, int pixels, unsigned int paint,
                      unsigned stride, const unsigned *glyph, int bpp, unsigned char width)
{
    unsigned x, y, data;
    //unsigned bytes_per_bpp = ((bpp) >> 3);
    stride -= width;

    for (y = 0; y < FONT_HEIGHT; y++) {
        data = glyph[y];
        for (x = 0; x < width; x++) {
            if (data & 1) {
                base[pixels] = paint;
            }
            data >>= 1;
            pixels++;
        }
        pixels += stride;
    }
}
#else
static void drawglyph(unsigned int *base, int pixels, unsigned int paint,
                      unsigned stride, const unsigned *glyph, int bpp)
{
    unsigned x, y, data;
    stride -= 5;

    data = glyph[0];
    for (y = 0; y < 6; y++) {
        for (x = 0; x < 5; x++) {
            if (data & 1) {
                base[pixels] = paint;
            }
            data >>= 1;
            pixels++;
        }
        pixels += stride;
    }
    data = glyph[1];
    for (y = 0; y < 6; y++) {
        for (x = 0; x < 5; x++) {
            if (data & 1) {
                base[pixels] = paint;
            }
            data >>= 1;
            pixels++;
        }
        pixels += stride;
    }
}
#endif                    
#else
static void drawglyph(unsigned char *base, int pixels, unsigned short paint,
                      unsigned stride, const unsigned *glyph, int bpp)
{
    unsigned x, y, data;
    stride -= 5;

    data = glyph[0];
    for(y = 0; y < 6; y++) {
        for(x = 0; x < 5; x++) {
            if(data & 1) {
                if(bpp == 2) {
                    ((unsigned short *)base)[pixels] = paint;
                } else if(bpp == 3){
                    base[pixels*3] = (paint & 0xF800) >> 11;
                    base[pixels*3 + 1] = (paint & 0x07E0) >> 5;
                    base[pixels*3 + 2] = (paint & 0x001F);
                }
            }
            data >>= 1;
            pixels++;
        }
        pixels += stride;
    }
    data = glyph[1];
    for(y = 0; y < 6; y++) {
        for(x = 0; x < 5; x++) {
            if(data & 1) {
                if(bpp == 2) {
                    ((unsigned short *)base)[pixels] = paint;
                } else if(bpp == 3){
                    base[pixels*3] = (paint & 0xF800) >> 11;
                    base[pixels*3 + 1] = (paint & 0x07E0) >> 5;
                    base[pixels*3 + 2] = (paint & 0x001F);
                }
            }
            data >>= 1;
            pixels++;
        } 
        pixels += stride;
    }
}
#endif

static void display_log(const char *str, int size)
{
    struct fb_info *info;
    unsigned short c;
    int i, j,k, count;
    int fb_width, fb_height;
    int fb_num;
    int bpp;
    int t_cx, t_cy;
    if (size <= 0) return;

    info = registered_fb[0];
    fb_width = info->var.xres + 16;
    //fb_width = info->var.xres;
    fb_height = info->var.yres;
#ifdef BITMAP_FONT_SUPPORT
    if (cx == 0 && cy == 0 && cmaxx == 0 && cmaxy == 0) { 
        cmaxx = fb_width / (18);
        cmaxy = (fb_height - 1) / FONT_HEIGHT;
        cx = cy = 0;
    }
#else
    if (cx == 0 && cy == 0 && cmaxx == 0 && cmaxy == 0) { 
        cmaxx = fb_width / 6;
        cmaxy = (fb_height - 1) / 12;
        cx = cy = 0;
    }
#endif
    bpp = info->var.bits_per_pixel >> 3; // / 8;
    fb_num = info->fix.smem_len / (fb_width * fb_height * bpp);

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
    if (fb_num > 3 ) fb_num = 3;
#else
    if (fb_num > 2 ) fb_num = 2;
#endif	

    t_cx = t_cy = 0;
#ifdef CONFIG_FB_MSM_DEFAULT_DEPTH_RGBA8888
#ifdef BITMAP_FONT_SUPPORT
    for (k = 0; k < fb_num; k++ ) {
        unsigned int *base;
        unsigned before_offset = 0;
        unsigned int text_c = FGCOLOR;
        base  = ((unsigned int *)info->screen_base) + fb_width * fb_height * k;
        t_cx = cx; t_cy = cy;

        count = fb_width * FONT_HEIGHT;

        j = t_cy * fb_width * FONT_HEIGHT;

        while (count--) {                
            base[j++] = BGCOLOR;
        }

        for (i = 0; i < size; i++) {
            c = str[i];
            if (c > 127) continue;
            if (c < 32) {
                if (c == '\n') {
                    t_cy++;
                    t_cx = 0;
                    before_offset = 0;
                    count = fb_width * FONT_HEIGHT;
                    j = t_cy * fb_width * FONT_HEIGHT;
                    while (count--) {                             
                        base[j++] = BGCOLOR;
                    }
                }else if(c == '\a'){
					text_c = REDCOLOR;
				}
				else if(c == '\b'){
					text_c = FGCOLOR;
				}
                continue;
            }
            /*static void drawglyph(unsigned int *base, int pixels, unsigned int paint,
                      unsigned stride, const unsigned *glyph, int bpp, unsigned char width)*/
#ifdef FONT_24X31_SUPPORT
			drawglyph(base, t_cy * FONT_HEIGHT * fb_width + before_offset, text_c,
                    fb_width, font24x31 + (offset_table[c-32]/sizeof(unsigned)), bpp, width_table[c-32]);
#else
            drawglyph(base, t_cy * FONT_HEIGHT * fb_width + before_offset, text_c,
                    fb_width, font32x45 + (offset_table[c-32]/sizeof(unsigned)), bpp, width_table[c-32]);
#endif                    
            before_offset += width_table[c-32];
            
            t_cx++;
            if( before_offset >= fb_width - 32 ) {
                t_cy++;
                t_cx = 0;
                before_offset = 0;
                count = fb_width * FONT_HEIGHT;
                j = t_cy * fb_width * FONT_HEIGHT;
                while (count--) {                        
                    base[j++] = BGCOLOR;
                }
            }
        }
    }
    cx = t_cx; cy = t_cy;
    cy++; cx= 0;
#else
    for (k = 0; k < fb_num; k++ ) {
        unsigned int *base;

        base  = ((unsigned int *)info->screen_base) + fb_width * fb_height * k;
        t_cx = cx; t_cy = cy;

        count = fb_width * 12;

        j = t_cy * fb_width * 12;

        while (count--) {                
            base[j++] = BGCOLOR;
        }

        for (i = 0; i < size; i++) {
            c = str[i];
            if (c > 127) continue;
            if (c < 32) {
                if (c == '\n') {
                    t_cy++;
                    t_cx = 0;
                    count = fb_width * 12;
                    j = t_cy * fb_width * 12;
                    while (count--) {                             
                        base[j++] = BGCOLOR;
                    }
                }
                continue;
            }
            drawglyph(base, t_cy * 12 * fb_width + t_cx * 6, FGCOLOR,
                    fb_width, font5x12 + (c - 32) * 2, bpp);
            t_cx++;
            if (t_cx >= cmaxx ) {
                t_cy++;
                t_cx = 0;
                count = fb_width * 12;
                j = t_cy * fb_width * 12;
                while (count--) {                        
                    base[j++] = BGCOLOR;
                }
            }
        }
    }
    cx = t_cx; cy = t_cy;
    cy++; cx= 0;
#endif
#else    
    if (bpp == 2) { // RGB565
        for(k = 0; k < fb_num; k++ ){
            unsigned short *base;

            base  = (unsigned short *)(((unsigned char *)info->screen_base) 
                    + fb_width * fb_height * bpp * k);
            t_cx = cx; t_cy = cy;

            count = fb_width * 12;  
            j = t_cy * fb_width * 12;
            while(count--) {
                base[j++] = BGCOLOR;
            }

            for(i = 0; i < size; i++) {
                c = str[i];
                if(c > 127) continue;
                if(c < 32){
                    if(c == '\n') {
                        t_cy++;
                        t_cx = 0;
                        count = fb_width * 12;
                        j = t_cy * fb_width * 12;
                        while(count--) {
                            base[j++] = BGCOLOR;
                        }
                    }
                    continue;
                }

                drawglyph((unsigned char *)base, t_cy * 12 * fb_width + t_cx * 6, FGCOLOR, 
                                 fb_width, font5x12 + (c - 32) * 2, bpp );
                t_cx++;
                if(t_cx >= cmaxx ){
                    t_cy++;
                    t_cx = 0;
                    count = fb_width * 12;
                    j = t_cy * fb_width * 12;
                    while(count--) {
                        base[j++] = BGCOLOR;
                    }
                }
            }

        }
        cx = t_cx; cy = t_cy;   

    } else { //RGB888
        for (k = 0; k < fb_num; k++ ) {
            unsigned char *base;

            base  = ((unsigned char *)info->screen_base) + fb_width * fb_height * bpp * k;
            t_cx = cx; t_cy = cy;

            count = fb_width * 12;  
            j = t_cy * fb_width * 12;
            while(count--) {
                base[j++] = (BGCOLOR & 0xF800) >> 11;
                base[j++] = (BGCOLOR & 0x07E0) >> 5;
                base[j++] = (BGCOLOR & 0x001F);
            }

            for(i = 0; i < size; i++) {
                c = str[i];
                if(c > 127) continue;
                if(c < 32){
                    if(c == '\n') {
                        t_cy++;
                        t_cx = 0;
                        count = fb_width * 12;
                        j = t_cy * fb_width * 12;
                        while(count--) {
                            base[j++] = (BGCOLOR & 0xF800) >> 11;
                            base[j++] = (BGCOLOR & 0x07E0) >> 5;
                            base[j++] = (BGCOLOR & 0x001F);
                        }
                    }
                    continue;
                }

                drawglyph((unsigned char*)base , t_cy * 12 * fb_width + t_cx * 6, FGCOLOR, 
                                 fb_width, font5x12 + (c - 32) * 2, bpp );
                t_cx++;
                if( t_cx >= cmaxx ){
                    t_cy++;
                    t_cx = 0;
                    count = fb_width * 12;
                    j = t_cy * fb_width * 12;
                    while(count--) {
                        base[j++] = (BGCOLOR & 0xF800) >> 11;
                        base[j++] = (BGCOLOR & 0x07E0) >> 5;
                        base[j++] = (BGCOLOR & 0x001F);
                    }
                }
            }
        }
        cx = t_cx; cy = t_cy;   
    }
    cy++; cx= 0;
#endif    
}

void pantech_errlog_display_put_log(const char *log, int size) {
    int i;
    int preempt_cnt = 0;
    printk(KERN_INFO "%s : 0\n", __func__);
    if (atomic_read(&is_displayed) == 0) {
        atomic_set(&is_displayed, 1);

        preempt_cnt = preempt_count();
        printk(KERN_INFO "preempt_count before : %d\n", preempt_cnt);

#if 1 // Make preempt_count to 1.
        if(preempt_cnt == 0)
            preempt_disable();
        else
            for(i=1; i<preempt_cnt; i++) preempt_enable();

        preempt_cnt = preempt_count();
        printk(KERN_INFO "preempt_count after : %d\n", preempt_cnt);
#endif
        
        bust_spinlocks(1);
        force_mdp4_overlay_unset();
        display_log(log, size);
        bust_spinlocks(0);
        preempt_enable(); 

        force_mdp_on();
        printk(KERN_INFO "%s : 1\n", __func__);
    }
}
EXPORT_SYMBOL(pantech_errlog_display_put_log);

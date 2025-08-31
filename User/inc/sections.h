#ifndef __SECTIONS_H__
#define __SECTIONS_H__

#include "stdint.h"
extern uint8_t _img_cache_start[];
extern uint8_t _img_cache_end[];

#define LCD_FRAMEBUF __attribute__((section(".lcd_framebuf")))
#define IMG_CACHE_ADDR ((uint32_t)_img_cache_start)

#define ALIGN(x) __attribute__((aligned(x)))
#endif

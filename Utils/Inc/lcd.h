#ifndef __LCD_H
#define __LCD_H

#include "setting.h"
#if USE_LTED_LCD

#include "ltdc.h"
#include "main.h"
#include "stm32f4xx_hal_dma2d.h"

#define WHITE (0xFFFF)
#define BLACK (0x0000)
#define BLUE (0x001F)
#define BRED (0XF81F)
#define GRED (0XFFE0)
#define GBLUE (0X07FF)
#define RED (0xF800)
#define MAGENTA (0xF81F)
#define GREEN (0x07E0)
#define CYAN (0x7FFF)
#define YELLOW (0xFFE0)
#define BROWN (0XBC40)      // 棕色
#define BRRED (0XFC07)      // 棕红色
#define GRAY (0X8430)       // 灰色
#define DARKBLUE (0X01CF)   // 深蓝色
#define LIGHTBLUE (0X7D7C)  // 浅蓝色
#define GRAYBLUE (0X5458)   // 灰蓝色
#define LIGHTGREEN (0X841F) // 浅绿色
#define LGRAY (0XC618)      // 浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE (0XA651)  // 浅灰蓝色(中间层颜色)
#define LBBLUE (0X2B12)     // 浅棕蓝色(选择条目的反色)
#define LCD_DIR 0           // 0为横屏1为竖屏，默认横屏

void lcd_clear(uint16_t color); // 清屏函数
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color);
void dma2d_fill_rect(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint32_t color);
void dma2d_draw_bitmap(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);

#endif
#endif

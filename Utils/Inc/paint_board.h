#ifndef __PAINT_BOARD_H__
#define __PAINT_BOARD_H__
#include "setting.h"
#if USE_LVGL
#include "lvgl.h"
#include "gui_guider.h"
#include "sections.h"
#include "color_picker.h"
#include "math.h"

void paint_clear(lv_color_t color);
void paint_set_pen_color(lv_color_t color);
void paint_set_pen_size(uint8_t size);
void custom_screen_paint_init(lv_ui *ui);

#endif 
#endif

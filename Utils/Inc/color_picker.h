#ifndef __COLOR_PICKER_H__
#define __COLOR_PICKER_H__
#include "setting.h"
#if USE_LVGL
#include "lvgl/lvgl.h"
#include "string.h"
#include "lv_types.h"
#include "gui_guider.h"

typedef struct {
    lv_obj_t *window;
    lv_obj_t *palette_area;
    lv_obj_t *preview;
    lv_obj_t *hue_bar;
    lv_obj_t *brightness_bar;
    lv_color_t selected_color;
    lv_obj_t *brush_slider;
    lv_obj_t *brush_label;
    uint8_t brush_width;
} color_picker_t;

void show_color_picker(lv_obj_t *parent, lv_color_t initial_color, lv_event_cb_t callback);
lv_color_t color_picker_get_selected();
void paint_set_pen_color(lv_color_t color);
#endif
#endif

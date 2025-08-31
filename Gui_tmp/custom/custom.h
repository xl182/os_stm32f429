/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by
 * downloading, installing, activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by, such license terms.  If you do
 * not agree to be bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 */

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include "string.h"
#define LV_IMG_PATH "0:/lvgl_app/images/"

typedef struct {
    lv_img_dsc_t desc;
    char name[32];
    uint32_t sdram_addr;
} ImageEntry;

typedef struct {
    uint32_t magic;
    uint32_t w;
    uint32_t h;
    uint32_t cf;
    uint32_t reserved;
} lv_img_header_t;

extern const char *image_paths[];
extern lv_font_t *montserratMedium_12;
extern lv_font_t *montserratMedium_16;
extern lv_font_t *montserratMedium_18;

void custom_init(lv_ui *ui);
void init_file_explorer(lv_ui *ui);
void custom_screen_init(lv_ui *ui);
void custom_screen_serial_init(lv_ui *ui);
void custom_screen_file_init(lv_ui *ui);
void custom_screen_paint_init(lv_ui *ui);
void image_table_init();
lv_img_dsc_t *get_image_desc(const char *name);
void load_font();
#endif /* EVENT_CB_H_ */

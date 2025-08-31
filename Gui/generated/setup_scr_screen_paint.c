/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_paint(lv_ui *ui)
{
    //Write codes screen_paint
    ui->screen_paint = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_paint, 1024, 600);
    lv_obj_set_scrollbar_mode(ui->screen_paint, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_paint, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_paint, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_paint_btn_back
    ui->screen_paint_btn_back = lv_button_create(ui->screen_paint);
    lv_obj_set_pos(ui->screen_paint_btn_back, 916, 9);
    lv_obj_set_size(ui->screen_paint_btn_back, 100, 50);
    ui->screen_paint_btn_back_label = lv_label_create(ui->screen_paint_btn_back);
    lv_label_set_text(ui->screen_paint_btn_back_label, "back");
    lv_label_set_long_mode(ui->screen_paint_btn_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_paint_btn_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_paint_btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_paint_btn_back_label, LV_PCT(100));

    //Write style for screen_paint_btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_paint_btn_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_paint_btn_back, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_paint_btn_back, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_paint_btn_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_paint_btn_back, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_paint_btn_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_paint_btn_back, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_paint_btn_back, montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_paint_btn_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_paint_btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_paint_btn_clear
    ui->screen_paint_btn_clear = lv_button_create(ui->screen_paint);
    lv_obj_set_pos(ui->screen_paint_btn_clear, 916, 543);
    lv_obj_set_size(ui->screen_paint_btn_clear, 100, 50);
    ui->screen_paint_btn_clear_label = lv_label_create(ui->screen_paint_btn_clear);
    lv_label_set_text(ui->screen_paint_btn_clear_label, "clear");
    lv_label_set_long_mode(ui->screen_paint_btn_clear_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_paint_btn_clear_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_paint_btn_clear, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_paint_btn_clear_label, LV_PCT(100));

    //Write style for screen_paint_btn_clear, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_paint_btn_clear, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_paint_btn_clear, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_paint_btn_clear, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_paint_btn_clear, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_paint_btn_clear, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_paint_btn_clear, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_paint_btn_clear, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_paint_btn_clear, montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_paint_btn_clear, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_paint_btn_clear, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_paint_paint_area
    ui->screen_paint_paint_area = lv_image_create(ui->screen_paint);
    lv_obj_set_pos(ui->screen_paint_paint_area, 0, 0);
    lv_obj_set_size(ui->screen_paint_paint_area, 900, 600);
    lv_obj_add_flag(ui->screen_paint_paint_area, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(ui->screen_paint_paint_area, 50,50);
    lv_image_set_rotation(ui->screen_paint_paint_area, 0);

    //Write style for screen_paint_paint_area, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_paint_paint_area, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_paint_paint_area, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_paint_btn_choose_color
    ui->screen_paint_btn_choose_color = lv_button_create(ui->screen_paint);
    lv_obj_set_pos(ui->screen_paint_btn_choose_color, 946, 114);
    lv_obj_set_size(ui->screen_paint_btn_choose_color, 50, 50);
    ui->screen_paint_btn_choose_color_label = lv_label_create(ui->screen_paint_btn_choose_color);
    lv_label_set_text(ui->screen_paint_btn_choose_color_label, "");
    lv_label_set_long_mode(ui->screen_paint_btn_choose_color_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_paint_btn_choose_color_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_paint_btn_choose_color, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_paint_btn_choose_color_label, LV_PCT(100));

    //Write style for screen_paint_btn_choose_color, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_paint_btn_choose_color, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_paint_btn_choose_color, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_paint_btn_choose_color, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_paint_btn_choose_color, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(ui->screen_paint_btn_choose_color, get_image_desc("color"), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_opa(ui->screen_paint_btn_choose_color, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_recolor_opa(ui->screen_paint_btn_choose_color, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_paint_btn_choose_color, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_paint_btn_choose_color, montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_paint_btn_choose_color, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_paint_btn_choose_color, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_paint_btn_paint_width
    ui->screen_paint_btn_paint_width = lv_button_create(ui->screen_paint);
    lv_obj_set_pos(ui->screen_paint_btn_paint_width, 946, 186);
    lv_obj_set_size(ui->screen_paint_btn_paint_width, 50, 50);
    ui->screen_paint_btn_paint_width_label = lv_label_create(ui->screen_paint_btn_paint_width);
    lv_label_set_text(ui->screen_paint_btn_paint_width_label, "");
    lv_label_set_long_mode(ui->screen_paint_btn_paint_width_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_paint_btn_paint_width_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_paint_btn_paint_width, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_paint_btn_paint_width_label, LV_PCT(100));

    //Write style for screen_paint_btn_paint_width, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_paint_btn_paint_width, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_paint_btn_paint_width, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_paint_btn_paint_width, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_paint_btn_paint_width, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(ui->screen_paint_btn_paint_width, get_image_desc("huabi"), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_opa(ui->screen_paint_btn_paint_width, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_recolor_opa(ui->screen_paint_btn_paint_width, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_paint_btn_paint_width, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_paint_btn_paint_width, montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_paint_btn_paint_width, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_paint_btn_paint_width, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_paint.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_paint);

    //Init events for screen.
    events_init_screen_paint(ui);
}
// modified
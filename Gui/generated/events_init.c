/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom.h"
#include "AHT10.h"
extern lv_ui *ui;
#include "usart.h"
#include "stdlib.h"
#include "stdio.h"
extern lv_ui *ui;
#include "paint_board.h"

static void screen_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        custom_screen_init(ui);
        break;
    }
    case LV_EVENT_SCREEN_LOAD_START:
    {
        ui->g_kb_top_layer = lv_keyboard_create(ui->screen);
        lv_obj_add_flag(ui->g_kb_top_layer, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

static void screen_digital_clock_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_VALUE_CHANGED:
    {
        char str[20];
        float humidity = 0, temperature = 0;
        aht10_read_data(&humidity, &temperature);
        lv_label_set_text_fmt(ui->screen_label_temp, "%.2f", temperature);
        lv_label_set_text_fmt(ui->screen_label_hum, "%.2f", humidity);
        break;
    }
    default:
        break;
    }
}

static void screen_btn_file_browser_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_file, guider_ui.screen_file_del, &guider_ui.screen_del, setup_scr_screen_file, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void screen_btn_serial_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_serial, guider_ui.screen_serial_del, &guider_ui.screen_del, setup_scr_screen_serial, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

static void screen_btn_paint_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_paint, guider_ui.screen_paint_del, &guider_ui.screen_del, setup_scr_screen_paint, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen, screen_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_digital_clock_1, screen_digital_clock_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_btn_file_browser, screen_btn_file_browser_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_btn_serial, screen_btn_serial_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_btn_paint, screen_btn_paint_event_handler, LV_EVENT_ALL, ui);
}

static void screen_file_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        custom_screen_file_init(ui);
        break;
    }
    default:
        break;
    }
}

static void screen_file_btn_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_file_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_file (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_file, screen_file_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_file_btn_back, screen_file_btn_back_event_handler, LV_EVENT_ALL, ui);
}

static void screen_serial_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        custom_screen_serial_init(ui);
        break;
    }
    case LV_EVENT_SCREEN_LOAD_START:
    {
        ui->g_kb_top_layer = lv_keyboard_create(ui->screen_serial);
        lv_obj_add_flag(ui->g_kb_top_layer, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    default:
        break;
    }
}

static void screen_serial_btn_clear_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        break;
    }
    default:
        break;
    }
}

static void screen_serial_ddlist_baudrate_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_VALUE_CHANGED:
    {
        uint32_t id = lv_dropdown_get_selected(guider_ui.screen_serial_ddlist_baudrate);
        char baudrate_str[20];
        lv_dropdown_get_selected_str(ui->screen_serial_ddlist_baudrate, baudrate_str, 20);
        uint32_t baudrate = atoi(baudrate_str);
        huart1.Instance = USART1;
        huart1.Init.BaudRate = baudrate;
        huart1.Init.WordLength = UART_WORDLENGTH_8B;
        huart1.Init.StopBits = UART_STOPBITS_1;
        huart1.Init.Parity = UART_PARITY_NONE;
        huart1.Init.Mode = UART_MODE_TX_RX;
        huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart1.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&huart1) != HAL_OK)
        {
            Error_Handler();
        }

        break;
    }
    default:
        break;
    }
}

static void screen_serial_btn_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_serial_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void screen_serial_btn_send_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        const char *text = lv_textarea_get_text(ui->screen_serial_ta_send);
        printf("%s\r\n", text);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_serial (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_serial, screen_serial_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_serial_btn_clear, screen_serial_btn_clear_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_serial_ddlist_baudrate, screen_serial_ddlist_baudrate_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_serial_btn_back, screen_serial_btn_back_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_serial_btn_send, screen_serial_btn_send_event_handler, LV_EVENT_ALL, ui);
}

static void screen_paint_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOADED:
    {
        custom_screen_paint_init(ui);
        break;
    }
    default:
        break;
    }
}

static void screen_paint_btn_back_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_paint_del, setup_scr_screen, LV_SCR_LOAD_ANIM_NONE, 200, 200, true, true);
        break;
    }
    default:
        break;
    }
}

static void screen_paint_btn_clear_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        paint_clear(lv_color_white());
        break;
    }
    default:
        break;
    }
}

void events_init_screen_paint (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_paint, screen_paint_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_paint_btn_back, screen_paint_btn_back_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_paint_btn_clear, screen_paint_btn_clear_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}

/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by
 * downloading, installing, activating and/or otherwise using the software, you are agreeing that
 * you have read, and that you agree to comply with and are bound by, such license terms.  If you do
 * not agree to be bound by the applicable license terms, then you may not retain, install, activate
 * or otherwise use the software.
 */

/*********************
 *      INCLUDES
 *********************/
#include "custom.h"
#include "AHT10.h"
#include "lv_file_explorer.h"
#include "lv_port_fs.h"
#include "lvgl.h"
#include "rtc.h"
#include "sections.h"
#include "usb_device.h"
#include <stdio.h>
extern lv_ui *ui;

/*********************
 *      DEFINES
 *********************/
#if 0
#define custom_log(...) printf(__VA_ARGS__)
#else
#define custom_log(...)
#endif
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */
#define SDRAM_SIZE (32 * 1024 * 1024)
#define MAX_IMAGES 20

extern int screen_digital_clock_1_hour_value;
extern int screen_digital_clock_1_min_value;
extern int screen_digital_clock_1_sec_value;
extern char screen_digital_clock_1_meridiem[];

static ImageEntry image_table[MAX_IMAGES];
uint32_t current_sdram_addr = IMG_CACHE_ADDR;

lv_font_t *montserratMedium_12;
lv_font_t *montserratMedium_16;
lv_font_t *montserratMedium_18;

lv_font_t *load_bin_font_to_sdram(char *path) {
    FIL fil;
    if (f_open(&fil, path, FA_READ) != FR_OK) {
        printf("File not found: %s\r\n", path);
    };

    uint8_t *data_addr = (uint8_t *)current_sdram_addr;
    uint32_t size      = f_size(&fil);
    current_sdram_addr += (size + 3) & ~3;
    UINT bytes_read = 0;
    FRESULT res     = f_read(&fil, data_addr, size, &bytes_read);
    if (res != FR_OK) {
        printf("read font file error code %d, size: %d\r\n", res, size);
    }
    f_close(&fil);
    return lv_binfont_create_from_buffer(data_addr, size);
}

void load_font() {
    montserratMedium_16 = load_bin_font_to_sdram("0:/lvgl_app/fonts/montserratMedium_16.bin");
    montserratMedium_12 = load_bin_font_to_sdram("0:/lvgl_app/fonts/montserratMedium_12.bin");
    montserratMedium_18 = load_bin_font_to_sdram("0:/lvgl_app/fonts/montserratMedium_18.bin");
}

void image_table_init() { memset(image_table, 0, sizeof(image_table)); }

lv_img_dsc_t *get_image_desc(const char *name) {
    for (int i = 0; i < MAX_IMAGES; i++) {
        if (strcmp(image_table[i].name, name) == 0) {
            custom_log("exist get_image_desc: %s\r\n", name);
            return &image_table[i].desc;
        }
    }
    char path[128];
    snprintf(path, sizeof(path), "0:/lvgl_app/images/%s.bin", name);

    FIL fil;
    UINT bytes_read;
    if (f_open(&fil, path, FA_READ) != FR_OK) {
        custom_log("File not found: %s\r\n", path);
        return NULL;
    }

    uint32_t size = f_size(&fil) - 5;
    uint8_t data[4];

    uint8_t type;
    f_read(&fil, &type, 1, &bytes_read);
    lv_color_format_t cf;
    if (type == 1) {
        cf = LV_COLOR_FORMAT_RGB565A8;
    } else if (type == 0) {
        cf = LV_COLOR_FORMAT_RGB565;
    } else if (type == 2) {
        cf = LV_COLOR_FORMAT_ARGB8565;
    }

    f_read(&fil, data, 4, &bytes_read);
    uint16_t width, height;
    width  = data[1] | (data[0] << 8);
    height = data[3] | (data[2] << 8);
    custom_log("type: %d, width: %d, height: %d, size: %d\r\n", type, width, height, size);
    f_read(&fil, (void *)current_sdram_addr, size, &bytes_read);
    f_close(&fil);

    for (int i = 0; i < MAX_IMAGES; i++) {
        if (image_table[i].name[0] == '\0') {
            strncpy(image_table[i].name, name, sizeof(image_table[i].name));
            image_table[i].sdram_addr = current_sdram_addr;
            image_table[i].desc = (lv_img_dsc_t){.header    = {.w = width, .h = height, .cf = cf},
                                                 .data_size = size,
                                                 .data      = (uint8_t *)current_sdram_addr};
            current_sdram_addr += (size + 3) & ~3;
            custom_log("get_image_desc: %s\r\n", name);
            return &image_table[i].desc;
        }
    }
    custom_log("not found \r\n");
    return NULL;
}

void init_file_explorer(lv_ui *ui) {
    ui->screen_file_file_explorer = lv_file_explorer_create(ui->screen_file);
    lv_file_explorer_set_quick_access_path(ui->screen_file_file_explorer, LV_EXPLORER_FS_DIR,
                                           "0:/");
    lv_file_explorer_set_quick_access_path(ui->screen_file_file_explorer, LV_EXPLORER_HOME_DIR,
                                           "0:/lvgl_app/");
    lv_obj_set_size(ui->screen_file_file_explorer, 1024, 550);
    lv_obj_set_pos(ui->screen_file_file_explorer, 0, 50);
}

void custom_screen_init(lv_ui *ui) {

}

void custom_screen_serial_init(lv_ui *ui) {
    
}

void custom_screen_file_init(lv_ui *ui) {
    init_file_explorer(ui);
    usb_reset();
    MX_USB_DEVICE_Init();
}

void custom_timer(lv_timer_t *timer) {
    lv_obj_t *current_screen = lv_scr_act();
    if (current_screen == ui->screen) {
        char str[20];
        float humidity = 0, temperature = 0;
        aht10_read_data(&humidity, &temperature);
        sprintf(str, "%.02f", temperature);
        lv_label_set_text(ui->screen_label_temp, str);
        sprintf(str, "%.02f", humidity);
        lv_label_set_text(ui->screen_label_hum, str);
    }
}

void custom_init(lv_ui *ui) {
    bool custom_timer_enabled = false;
    RTC_TimeTypeDef s_time;
    HAL_RTC_GetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
    screen_digital_clock_1_sec_value  = s_time.Seconds;
    screen_digital_clock_1_min_value  = s_time.Minutes;
    screen_digital_clock_1_hour_value = s_time.Hours;
    lv_timer_create(custom_timer, 1000, NULL);
    custom_timer_enabled = true;
}

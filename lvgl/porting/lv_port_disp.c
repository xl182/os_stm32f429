/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "sections.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES 320
#endif

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_VER_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES 240
#endif

#define BYTE_PER_PIXEL                                                                             \
    (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565                      \
                                                        */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_display_t *g_disp_drv;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void) {
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;
    lv_display_set_buffers(disp, buf_1, buf_2, sizeof(buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void) { /*You code here*/
    MX_LTDC_Init();
    MX_DMA2D_Init();
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void) { disp_flush_enabled = true; }

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void) { disp_flush_enabled = false; }

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the
 *display. You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map) {
    if (disp_flush_enabled) {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

        uint16_t width  = area->x2 - area->x1 + 1;
        uint16_t height = area->y2 - area->y1 + 1;

        dma2d_draw_bitmap(area->x1, area->y1, width, height, (uint16_t *)px_map);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

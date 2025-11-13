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
#include "lvgl/src/display/lv_display_private.h"
#include "sections.h"
#include <stdbool.h>
#include "spi.h"
#include "dma.h"
#include "lcd_spi_096.h"
#include "stdint.h"

lv_display_t *disp_drv_spi;

/*********************
 *      DEFINES
 *********************/
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

static void disp_flush_ltdc(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void disp_flush_spi(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_display_t *disp_drv_ltdc;
lv_display_t *disp_drv_spi;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
#include <stdio.h>
void lv_port_disp_init(void) {
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    printf("lv_port_disp_init start\r\n");
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp1 = lv_display_create(DISP1_HOR_RES, DISP1_VER_RES);
    disp_drv_ltdc         = disp1;
    lv_display_set_flush_cb(disp1, disp_flush_ltdc);

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t disp1_buf_1[DISP1_HOR_RES * DISP1_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t disp1_buf_2[DISP1_HOR_RES * DISP1_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;
    lv_display_set_buffers(disp1, disp1_buf_1, disp1_buf_2, sizeof(disp1_buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_t *disp2 = lv_display_create(DISP2_HOR_RES, DISP2_VER_RES);
    disp_drv_spi        = disp2;
    lv_display_set_flush_cb(disp2, disp_flush_spi);

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t disp2_buf_1[DISP2_HOR_RES * DISP2_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t disp2_buf_2[DISP2_HOR_RES * DISP2_VER_RES * BYTE_PER_PIXEL] LCD_FRAMEBUF;
    lv_display_set_buffers(disp2, disp2_buf_1, disp2_buf_2, sizeof(disp2_buf_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_obj_add_flag(disp_drv_spi->mem_label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(disp_drv_spi->perf_label, LV_OBJ_FLAG_HIDDEN);
    if (disp_drv_spi == NULL) {
        printf("Error: disp_drv_spi is NULL! SPI is not initialized in init\r\n");
        return;  // 避免后续错误
    } else {
        printf("disp_drv_spi initialized successfully in init\r\n");
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void) { 
    /*You code here*/
    // ltdc disp1 initialization
    MX_LTDC_Init();
    MX_DMA2D_Init();
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);

    // spi disp2 initialization
    SPI_LCD_Init();
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
static void disp_flush_ltdc(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map) {
    if (disp_flush_enabled) {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/

        uint16_t width  = area->x2 - area->x1 + 1;
        uint16_t height = area->y2 - area->y1 + 1;

        dma2d_draw_bitmap(area->x1, area->y1, width, height, (uint16_t *)px_map);
        // HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)px_map,
        //                    (uint32_t)(LTDC_Layer1->CFBAR +
        //                               (area->y1 * DISP1_HOR_RES + area->x1) * BYTE_PER_PIXEL),
        //                    width, height);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

void LCD_Color_Fill_DMA(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t *color_p) {
    uint16_t width, height;
    width = xend - xsta + 1;
    height = yend - ysta + 1;
    uint32_t size = width * height;

    LCD_SetAddress(xsta, ysta, xend, yend);

    LCD_CS_L;
    LCD_SPI.Init.DataSize = SPI_DATASIZE_16BIT;
    LCD_SPI.Instance->CR1 |= SPI_CR1_DFF;
    HAL_SPI_Transmit_DMA(&hspi2, (uint8_t *) color_p, size);
    while (__HAL_DMA_GET_COUNTER(&hdma_spi2_tx) != 0);
    LCD_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
    LCD_SPI.Instance->CR1 &= ~SPI_CR1_DFF;
    LCD_CS_H;
}

static void disp_flush_spi(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map) {
    if (disp_flush_enabled) {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
        LCD_Color_Fill_DMA(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif

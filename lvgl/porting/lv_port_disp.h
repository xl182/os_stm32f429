/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H
/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "lcd.h"
#include "dma2d.h"
/*********************
 *      DEFINES
 *********************/
#define DISP1_HOR_RES 1024
#define DISP1_VER_RES 600

#define DISP2_HOR_RES 240
#define DISP2_VER_RES 240
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Initialize low level display driver */
void DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma);
void DMA2D_ErrorCallback(DMA2D_HandleTypeDef *hdma);
extern lv_display_t *disp_drv_spi;

void lv_port_disp_init(void);

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void);

/**********************
 *      MACROS
 **********************/
#endif /*LV_PORT_DISP_TEMPL_H*/

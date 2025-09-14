#include "lcd.h"
#if USE_LTDC_LCD
#include "dma2d.h"
#include "ltdc.h"

/**
 * @brief 清屏函数
 * @param color 填充颜色值
 */
void lcd_clear(uint16_t color) {
    // 等待垂直同步避免撕裂效应
    while (LTDC->CDSR != LTDC_CDSR_VDES) {
    }

    // 使用DMA2D加速清屏
    dma2d_fill_rect(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, color);
}

/**
 * @brief 绘制单个像素点
 * @param x X坐标
 * @param y Y坐标
 * @param color 像素颜色值
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint32_t color) {
    if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
        frame_buffer[y * SCREEN_WIDTH + x] = (uint16_t)color;
    }
}

/**
 * @brief 使用DMA2D填充矩形区域
 * @param start_x 起始X坐标
 * @param start_y 起始Y坐标
 * @param end_x 结束X坐标
 * @param end_y 结束Y坐标
 * @param color 填充颜色值
 */
void dma2d_fill_rect(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y,
                     uint32_t color) {
    // 参数校验
    if (start_x >= SCREEN_WIDTH || start_y >= SCREEN_HEIGHT || end_x >= SCREEN_WIDTH ||
        end_y >= SCREEN_HEIGHT || start_x > end_x || start_y > end_y) {
        return;
    }

    uint32_t width  = end_x - start_x + 1;
    uint32_t height = end_y - start_y + 1;
    uint32_t offset = SCREEN_WIDTH - width;
    uint32_t addr   = (uint32_t)frame_buffer + 2 * (start_y * SCREEN_WIDTH + start_x);

    // 配置并启动DMA2D
    __HAL_RCC_DMA2D_CLK_ENABLE();
    DMA2D->CR &= ~DMA2D_CR_START;
    DMA2D->CR     = DMA2D_R2M;
    DMA2D->OPFCCR = LTDC_PIXEL_FORMAT_RGB565;
    DMA2D->OMAR   = addr;
    DMA2D->OOR    = offset;
    DMA2D->NLR    = (width << 16) | height;
    DMA2D->OCOLR  = color;
    DMA2D->CR |= DMA2D_CR_START;
    // 等待传输完成
    while ((DMA2D->ISR & DMA2D_FLAG_TC) == 0)
        ;
    DMA2D->IFCR |= DMA2D_FLAG_TC;
}

/**
 * @brief 使用DMA2D绘制位图
 * @param start_x 起始X坐标
 * @param start_y 起始Y坐标
 * @param width 位图宽度
 * @param height 位图高度
 * @param bitmap 位图数据指针
 */
void dma2d_draw_bitmap(uint16_t start_x, uint16_t start_y, uint16_t width, uint16_t height,
                       uint16_t *bitmap) {
    if (start_x >= SCREEN_WIDTH || start_y >= SCREEN_HEIGHT || width == 0 || height == 0 ||
        !bitmap || (start_x + width) > SCREEN_WIDTH || (start_y + height) > SCREEN_HEIGHT) {
        return;
    }

    uint32_t offset = SCREEN_WIDTH - width;
    uint32_t addr   = (uint32_t)frame_buffer + 2 * (start_y * SCREEN_WIDTH + start_x);

    __HAL_RCC_DMA2D_CLK_ENABLE();
    DMA2D->CR &= ~DMA2D_CR_START;
    DMA2D->CR      = DMA2D_M2M;
    DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB565;
    DMA2D->FGMAR   = (uint32_t)bitmap;
    DMA2D->OMAR    = addr;
    DMA2D->FGOR    = 0;
    DMA2D->OOR     = offset;
    DMA2D->NLR     = (width << 16) | height;
    DMA2D->CR |= DMA2D_CR_START;

    while ((DMA2D->ISR & DMA2D_FLAG_TC) == 0)
        ;
    DMA2D->IFCR |= DMA2D_FLAG_TC;
}
#endif

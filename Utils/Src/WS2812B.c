#include "ws2812b.h"
#if USE_WS2812B
uint16_t WS2812_RGB_Buff[LED_NUM * DATA_LEN + WS_REST] = {0};

const RGB_Color_TypeDef WS_RED     = {255, 0, 0};
const RGB_Color_TypeDef WS_ORANGE  = {127, 106, 0};
const RGB_Color_TypeDef WS_YELLOW  = {127, 216, 0};
const RGB_Color_TypeDef WS_GREEN   = {0, 255, 0};
const RGB_Color_TypeDef WS_CYAN    = {0, 255, 255};
const RGB_Color_TypeDef WS_BLUE    = {0, 0, 255};
const RGB_Color_TypeDef WS_PURPLE  = {238, 130, 238};
const RGB_Color_TypeDef WS_BLACK   = {0, 0, 0};
const RGB_Color_TypeDef WS_WHITE   = {255, 255, 255};
const RGB_Color_TypeDef WS_MAGENTA = {255, 0, 220};

void WS2812_set(uint16_t num, RGB_Color_TypeDef color) {
    const uint32_t index = (num * (3 * 8));
    for (uint8_t i = 0; i < 8; i++) {
        WS2812_RGB_Buff[index + i]      = (color.G << i) & (0x80) ? WS_H : WS_L;
        WS2812_RGB_Buff[index + i + 8]  = (color.R << i) & (0x80) ? WS_H : WS_L;
        WS2812_RGB_Buff[index + i + 16] = (color.B << i) & (0x80) ? WS_H : WS_L;
    }
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (const uint32_t *)WS2812_RGB_Buff,
                          sizeof(WS2812_RGB_Buff) / sizeof(uint16_t));
}

void WS2812_init() {
    RGB_Color_TypeDef color = {0, 0, 0};
    for (int i = 0; i < LED_NUM; i++) {
        WS2812_set(i, color);
    }

    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (const uint32_t *)WS2812_RGB_Buff,
                          sizeof(WS2812_RGB_Buff) / sizeof(uint16_t));
}
#endif
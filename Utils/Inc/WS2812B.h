#ifndef WS2812B_H
#define WS2812B_H

#include "setting.h"
#if USE_WS2812B
#include "main.h"
#include "tim.h"

#define TIM_FREQ 72 * 1000 * 1000
#define TIMER_PERIOD 90 - 1

/* T0H 220ns-380ns  T0L 580ns-1600ns
 * T1H 580ns-1600ns T1L 220ns-420ns
 * T0  700ns-1980ns
 * T1  700ns-1980ns
 * T0H+T0L、T1H+T1L≥1.25µs
 * T_RES >=280µs
 */

// WS_H + WS_L = TIMER_PERIOD
// TIM_FREQ / (TIMER_PERIOD) = 800k  1250ns = 972ns + 277ns

#define WS_H ((225) * 7 / 9)
#define WS_L ((225) - WS_H)
#define WS_REST 240
#define LED_NUM 8
#define DATA_LEN 24

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} RGB_Color_TypeDef;

extern const RGB_Color_TypeDef WS_RED;
extern const RGB_Color_TypeDef WS_ORANGE;
extern const RGB_Color_TypeDef WS_YELLOW;
extern const RGB_Color_TypeDef WS_GREEN;
extern const RGB_Color_TypeDef WS_CYAN;
extern const RGB_Color_TypeDef WS_BLUE;
extern const RGB_Color_TypeDef WS_PURPLE;
extern const RGB_Color_TypeDef WS_BLACK;
extern const RGB_Color_TypeDef WS_WHITE;
extern const RGB_Color_TypeDef WS_MAGENTA;

void WS2812_init(void);

void WS2812_set(uint16_t num, RGB_Color_TypeDef color);

#endif
#endif

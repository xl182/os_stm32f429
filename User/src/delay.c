#include "delay.h"

#define SYS_TICK_AS_TIMEBASE 0

#if SYS_TICK_AS_TIMEBASE
void delay_ms(uint32_t ms) { HAL_Delay(ms); }

void delay_us(uint32_t us) {
    uint32_t tickstart     = SysTick->VAL;
    uint32_t tickNum       = 0;
    uint32_t tickMax       = SysTick->LOAD + 1;
    uint32_t delay_usvalue = (tickMax / 1000) * us;
    while (1) {
        uint32_t cur_tick = SysTick->VAL;
        if (cur_tick > tickstart) {
            tickNum = tickstart + (tickMax - cur_tick);
        } else {
            tickNum = tickstart - cur_tick;
        }

        if (tickNum > delay_usvalue) {
            return;
        }
    }
}
#else
void delay_us(uint32_t us) {
    uint32_t tickend = (TIM7->CNT + us) % 1000;
    while (tickend != TIM7->CNT)
        ;
}

void delay_ms(uint32_t ms) {
    for (int i = 0; i < ms; i++)
        delay_us(999);
}
#endif

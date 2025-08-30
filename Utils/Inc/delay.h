#ifndef __DELAY_H__
#define __DELAY_H__
#include "stdint.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);

#endif

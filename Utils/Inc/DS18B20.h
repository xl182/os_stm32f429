#ifndef PROGRAM_DS18B20_H
#define PROGRAM_DS18B20_H

#include "setting.h"
#if USE_DS18B20 == 1

#include "main.h"
#include "delay.h"
#include <stdint-gcc.h>
#include "gpio.h"

typedef struct {
    uint8_t humi_int;        //湿度的整数部分
    uint8_t humi_deci;        //湿度的小数部分
    uint8_t temp_int;        //温度的整数部分
    uint8_t temp_deci;        //温度的小数部分
    uint8_t check_sum;        //校验和
} DS18B20_data_typeDef;

uint8_t DS18B20_init(void);
void DS18B20_start();
void DS18B20_reset(void);

float DS18B20_get_temp(void);

#endif //PROGRAM_DS18B20_H
#endif

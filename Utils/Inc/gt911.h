#ifndef __GT911_H
#define __GT911_H

#include "setting.h"
#if USE_GT911
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"

// 引脚定义
#define T_SDA_PIN GPIO_PIN_3
#define T_SDA_PORT GPIOI
#define T_SCK_PIN GPIO_PIN_6
#define T_SCK_PORT GPIOH
#define T_RST_PIN GPIO_PIN_8
#define T_RST_PORT GPIOI
#define T_INT_PIN GPIO_PIN_7
#define T_INT_PORT GPIOH

// I2C地址定义
#define GT911_WRITE_ADDR 0x28
#define GT911_READ_ADDR 0x29

// 寄存器定义
#define GT911_CTRL_REG 0x8040
#define GT911_CFGS_REG 0x8047
#define GT911_CHECK_REG 0x80FF
#define GT911_PID_REG 0x8140
#define GT911_GSTID_REG 0x814E
#define GT911_TP1_REG 0x8150
#define GT911_TP2_REG 0x8158
#define GT911_TP3_REG 0x8160
#define GT911_TP4_REG 0x8168
#define GT911_TP5_REG 0x8170
#define GT911_TP6_REG 0x8178
#define GT911_TP7_REG 0x8180
#define GT911_TP8_REG 0x8188
#define GT911_TP9_REG 0x8190
#define GT911_TP10_REG 0x8198

// 屏幕尺寸
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 600

// 触摸点结构体
typedef struct {
    uint16_t x[10];     // X坐标
    uint16_t y[10];     // Y坐标
    uint8_t status[10]; // 每个触摸点的状态
    uint8_t points;     // 当前触摸点数
    uint8_t valid;      // 数据有效性标志
} gt911_touch_t;

// 函数声明
void gt911_init(void);
void gt911_reset(void);
uint8_t gt911_read_reg(uint16_t reg, uint8_t *buf, uint8_t len);
uint8_t gt911_write_reg(uint16_t reg, uint8_t *data, uint8_t len);
uint8_t gt911_scan(gt911_touch_t *touch);
void gt911_calibrate_coordinates(gt911_touch_t *touch);
void touch_test(void);
#endif
#endif

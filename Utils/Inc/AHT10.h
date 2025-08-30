#ifndef AHT10_H
#define AHT10_H
#include "setting.h"
#if USE_AHT10
#include "i2c.h"
#include "main.h"
#include "stdbool.h"

/* AHT10 寄存器地址 */
#define AHT10_I2C_ADDR 0x38                          // 7位I2C地址
#define AHT10_WRITE_ADDR (AHT10_I2C_ADDR << 1)       // 写地址（0x70）
#define AHT10_READ_ADDR (AHT10_I2C_ADDR << 1 | 0x01) // 读地址（0x71）

/* AHT10 命令定义 */
#define AHT10_INIT_CMD 0xE1       // 初始化命令
#define AHT10_MEASURE_CMD 0xAC    // 触发测量命令
#define AHT10_SOFT_RESET_CMD 0xBA // 软复位命令

/* AHT10 状态标志位掩码 */
#define AHT10_STATUS_BUSY 0x80 // 忙标志（bit7）
#define AHT10_STATUS_CAL 0x08  // 校准标志（bit3）

/**
 * @brief 初始化 AHT10 传感器
 * @return 0: 成功; 非0: 失败
 */
uint8_t aht10_init(void);

/**
 * @brief 软复位 AHT10 传感器
 */
void aht10_soft_reset(void);

/**
 * @brief 读取 AHT10 温湿度数据
 * @param humidity: 湿度值指针（单位：%RH）
 * @param temperature: 温度值指针（单位：°C）
 * @return 0: 成功; 1: 设备忙; 2: 未校准; 其他: I2C错误
 */
uint8_t aht10_read_data(float *humidity, float *temperature);
#endif
#endif /* AHT10_H */

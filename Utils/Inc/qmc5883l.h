#ifndef QMC5883L_H
#define QMC5883L_H
#include "setting.h"
#if USE_QMC5883L
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <math.h>
#include "i2c.h"
#include "usart.h"

/* QMC5883L I2C地址 */
#define QMC5883L_ADDR (0x0D << 1)  // 7位地址左移1位

/* 寄存器地址定义 */
#define QMC5883L_XOUT_LSB 0x00
#define QMC5883L_XOUT_MSB 0x01
#define QMC5883L_YOUT_LSB 0x02
#define QMC5883L_YOUT_MSB 0x03
#define QMC5883L_ZOUT_LSB 0x04
#define QMC5883L_ZOUT_MSB 0x05
#define QMC5883L_STATUS   0x06
#define QMC5883L_TOUT_LSB 0x07
#define QMC5883L_TOUT_MSB 0x08
#define QMC5883L_CONFIG1   0x09
#define QMC5883L_CONFIG2   0x0A
#define QMC5883L_RESET     0x0B
#define QMC5883L_CHIP_ID   0x0D

/* 配置寄存器值定义 */
#define QMC5883L_MODE_CONTINUOUS 0x01  // 连续测量模式
#define QMC5883L_ODR_10HZ        0x00  // 输出数据速率10Hz
#define QMC5883L_ODR_50HZ        0x04  // 输出数据速率50Hz
#define QMC5883L_ODR_100HZ       0x08  // 输出数据速率100Hz
#define QMC5883L_ODR_200HZ       0x0C  // 输出数据速率200Hz
#define QMC5883L_RNG_2G          0x00  // 量程±2高斯(2000 milligauss)
#define QMC5883L_RNG_8G          0x10  // 量程±8高斯(8000 milligauss)
#define QMC5883L_OSR_512         0x00  // 过采样率512
#define QMC5883L_OSR_256         0x40  // 过采样率256
#define QMC5883L_OSR_128         0x80  // 过采样率128
#define QMC5883L_OSR_64          0xC0  // 过采样率64

/* 状态寄存器位定义 */
#define QMC5883L_STATUS_DRDY     0x01  // 数据就绪标志
#define QMC5883L_STATUS_OVL      0x02  // 溢出标志
#define QMC5883L_STATUS_DOR      0x04  // 数据过时标志

/* 错误码定义 */
typedef enum {
    QMC5883L_OK = 0,
    QMC5883L_ERROR_COMM,
    QMC5883L_ERROR_OVERFLOW,
    QMC5883L_ERROR_ID,
    QMC5883L_ERROR_PARAM
} QMC5883L_Error_t;

/* 数据结构定义 */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t odr;        // 输出数据速率
    uint8_t range;      // 测量量程
    uint8_t osr;        // 过采样率
    float scale;        // 比例因子，用于单位转换
    int16_t x_raw;      // X轴原始数据
    int16_t y_raw;      // Y轴原始数据
    int16_t z_raw;      // Z轴原始数据
    float x;            // X轴磁场强度(milligauss)
    float y;            // Y轴磁场强度(milligauss)
    float z;            // Z轴磁场强度(milligauss)
    int16_t temperature_raw;  // 温度原始数据
    float temperature;        // 温度(°C)
} QMC5883L_Handle_t;

/* 函数声明 */
QMC5883L_Error_t qmc5883l_init(QMC5883L_Handle_t *hdev, I2C_HandleTypeDef *hi2c);
QMC5883L_Error_t qmc5883l_configure(QMC5883L_Handle_t *hdev, uint8_t odr, uint8_t range, uint8_t osr);
QMC5883L_Error_t qmc5883l_reset(QMC5883L_Handle_t *hdev);
QMC5883L_Error_t qmc5883l_read_raw(QMC5883L_Handle_t *hdev);
QMC5883L_Error_t qmc5883l_read(QMC5883L_Handle_t *hdev);
QMC5883L_Error_t qmc5883l_is_data_ready(QMC5883L_Handle_t *hdev, uint8_t *ready);
QMC5883L_Error_t qmc5883l_get_temperature(QMC5883L_Handle_t *hdev);
QMC5883L_Error_t qmc5883l_set_mode(QMC5883L_Handle_t *hdev, uint8_t mode);
QMC5883L_Error_t qmc5883l_set_sampling_rate(QMC5883L_Handle_t *hdev, uint8_t odr);
QMC5883L_Error_t qmc5883l_set_range(QMC5883L_Handle_t *hdev, uint8_t range);
QMC5883L_Error_t qmc5883l_set_oversampling(QMC5883L_Handle_t *hdev, uint8_t osr);
float calculate_heading(float x, float y);
void qmc8553_test();
#endif
#endif /* QMC5883L_H */    

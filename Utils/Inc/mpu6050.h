#ifndef __MPU6050_H
#define __MPU6050_H
#include "setting.h"
#if USE_MPU6050
#include "stm32f4xx_hal.h"

/* MPU6050 I2C地址 (AD0引脚接地时为0x68，接VCC时为0x69) */
#define MPU6050_ADDR            0x69        /* 7位地址 */
#define MPU6050_WRITE_ADDR      (MPU6050_ADDR << 1)  /* 写地址 */
#define MPU6050_READ_ADDR       ((MPU6050_ADDR << 1) | 1)  /* 读地址 */

/* 寄存器地址定义 */
#define MPU6050_REG_PWR_MGMT_1  0x6B
#define MPU6050_REG_SMPLRT_DIV  0x19
#define MPU6050_REG_CONFIG      0x1A
#define MPU6050_REG_GYRO_CONFIG 0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_INT_ENABLE  0x38
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_TEMP_OUT_H   0x41
#define MPU6050_REG_WHO_AM_I     0x75

/* 陀螺仪量程配置 */
typedef enum {
    MPU6050_GYRO_SCALE_250DPS = 0x00,
    MPU6050_GYRO_SCALE_500DPS = 0x08,
    MPU6050_GYRO_SCALE_1000DPS = 0x10,
    MPU6050_GYRO_SCALE_2000DPS = 0x18
} mpu6050_gyro_scale_t;

/* 加速度计量程配置 */
typedef enum {
    MPU6050_ACCEL_SCALE_2G = 0x00,
    MPU6050_ACCEL_SCALE_4G = 0x08,
    MPU6050_ACCEL_SCALE_8G = 0x10,
    MPU6050_ACCEL_SCALE_16G = 0x18
} mpu6050_accel_scale_t;

/* 设备状态枚举 */
typedef enum {
    MPU6050_OK = 0,
    MPU6050_ERROR,
    MPU6050_ERROR_COMM,
    MPU6050_ERROR_ID,
    MPU6050_ERROR_TIMEOUT
} mpu6050_status_t;

/* MPU6050设备结构体 */
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    mpu6050_gyro_scale_t gyro_scale;
    mpu6050_accel_scale_t accel_scale;
    float gyro_scale_factor;
    float accel_scale_factor;
    
    /* 原始数据 */
    int16_t accel_raw[3];
    int16_t gyro_raw[3];
    int16_t temp_raw;
    
    /* 转换后的数据 */
    float accel[3];      /* m/s² */
    float gyro[3];       /* °/s */
    float temperature;   /* °C */
} mpu6050_handle_t;

/* 函数声明 */
mpu6050_status_t mpu6050_init(mpu6050_handle_t *hdev, I2C_HandleTypeDef *hi2c);
mpu6050_status_t mpu6050_read_accel(mpu6050_handle_t *hdev);
mpu6050_status_t mpu6050_read_gyro(mpu6050_handle_t *hdev);
mpu6050_status_t mpu6050_read_temp(mpu6050_handle_t *hdev);
mpu6050_status_t mpu6050_read_all(mpu6050_handle_t *hdev);
mpu6050_status_t mpu6050_set_gyro_scale(mpu6050_handle_t *hdev, mpu6050_gyro_scale_t scale);
mpu6050_status_t mpu6050_set_accel_scale(mpu6050_handle_t *hdev, mpu6050_accel_scale_t scale);
#endif
#endif /* __MPU6050_H */

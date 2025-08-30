#include "mpu6050.h"

#if USE_MPU6050
#include "i2c.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"

/* 向MPU6050写入一个字节数据 */
static mpu6050_status_t mpu6050_write_byte(mpu6050_handle_t *hdev, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};

    if (HAL_I2C_Master_Transmit(hdev->hi2c, hdev->address << 1, buffer, 2, 100) != HAL_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

/* 从MPU6050读取一个字节数据 */
static mpu6050_status_t mpu6050_read_byte(mpu6050_handle_t *hdev, uint8_t reg, uint8_t *data) {
    if (HAL_I2C_Master_Transmit(hdev->hi2c, hdev->address << 1, &reg, 1, 100) != HAL_OK) {
        return MPU6050_ERROR_COMM;
    }

    if (HAL_I2C_Master_Receive(hdev->hi2c, (hdev->address << 1) | 1, data, 1, 100) != HAL_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

/* 从MPU6050读取多个字节数据 */
static mpu6050_status_t mpu6050_read_bytes(mpu6050_handle_t *hdev, uint8_t reg, uint8_t *data,
                                           uint8_t length) {
    if (HAL_I2C_Master_Transmit(hdev->hi2c, hdev->address << 1, &reg, 1, 100) != HAL_OK) {
        return MPU6050_ERROR_COMM;
    }

    if (HAL_I2C_Master_Receive(hdev->hi2c, (hdev->address << 1) | 1, data, length, 100) != HAL_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

/* 初始化MPU6050 */
mpu6050_status_t mpu6050_init(mpu6050_handle_t *hdev, I2C_HandleTypeDef *hi2c) {
    uint8_t whoami;
    uint8_t buffer[14];

    /* 保存I2C句柄和地址 */
    hdev->hi2c    = hi2c;
    hdev->address = MPU6050_ADDR;

    /* 设置默认量程 */
    hdev->gyro_scale  = MPU6050_GYRO_SCALE_250DPS;
    hdev->accel_scale = MPU6050_ACCEL_SCALE_2G;

    /* 计算量程因子 */
    switch (hdev->gyro_scale) {
    case MPU6050_GYRO_SCALE_250DPS:
        hdev->gyro_scale_factor = 250.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_500DPS:
        hdev->gyro_scale_factor = 500.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_1000DPS:
        hdev->gyro_scale_factor = 1000.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_2000DPS:
        hdev->gyro_scale_factor = 2000.0f / 32768.0f;
        break;
    default:
        hdev->gyro_scale_factor = 250.0f / 32768.0f;
        break;
    }

    switch (hdev->accel_scale) {
    case MPU6050_ACCEL_SCALE_2G:
        hdev->accel_scale_factor = 2.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_4G:
        hdev->accel_scale_factor = 4.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_8G:
        hdev->accel_scale_factor = 8.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_16G:
        hdev->accel_scale_factor = 16.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    default:
        hdev->accel_scale_factor = 2.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    }

    /* 读取设备ID */
    if (mpu6050_read_byte(hdev, MPU6050_REG_WHO_AM_I, &whoami) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 验证设备ID */
    if (whoami != 0x68) {
        return MPU6050_ERROR_ID;
    }

    /* 唤醒设备 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_PWR_MGMT_1, 0x00) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 设置采样率分频 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_SMPLRT_DIV, 0x07) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 设置配置寄存器 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_CONFIG, 0x00) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 设置陀螺仪配置寄存器 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_GYRO_CONFIG, hdev->gyro_scale) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 设置加速度计配置寄存器 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_ACCEL_CONFIG, hdev->accel_scale) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 启用数据就绪中断 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_INT_ENABLE, 0x01) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

/* 读取加速度数据 */
mpu6050_status_t mpu6050_read_accel(mpu6050_handle_t *hdev) {
    uint8_t buffer[6];

    /* 读取加速度数据 */
    if (mpu6050_read_bytes(hdev, MPU6050_REG_ACCEL_XOUT_H, buffer, 6) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 组合高低字节 */
    hdev->accel_raw[0] = (int16_t)((buffer[0] << 8) | buffer[1]);
    hdev->accel_raw[1] = (int16_t)((buffer[2] << 8) | buffer[3]);
    hdev->accel_raw[2] = (int16_t)((buffer[4] << 8) | buffer[5]);

    /* 转换为实际物理单位 (m/s²) */
    hdev->accel[0] = (float)hdev->accel_raw[0] * hdev->accel_scale_factor;
    hdev->accel[1] = (float)hdev->accel_raw[1] * hdev->accel_scale_factor;
    hdev->accel[2] = (float)hdev->accel_raw[2] * hdev->accel_scale_factor;

    return MPU6050_OK;
}

/* 读取陀螺仪数据 */
mpu6050_status_t mpu6050_read_gyro(mpu6050_handle_t *hdev) {
    uint8_t buffer[6];

    /* 读取陀螺仪数据 */
    if (mpu6050_read_bytes(hdev, MPU6050_REG_GYRO_XOUT_H, buffer, 6) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 组合高低字节 */
    hdev->gyro_raw[0] = (int16_t)((buffer[0] << 8) | buffer[1]);
    hdev->gyro_raw[1] = (int16_t)((buffer[2] << 8) | buffer[3]);
    hdev->gyro_raw[2] = (int16_t)((buffer[4] << 8) | buffer[5]);

    /* 转换为实际物理单位 (°/s) */
    hdev->gyro[0] = (float)hdev->gyro_raw[0] * hdev->gyro_scale_factor;
    hdev->gyro[1] = (float)hdev->gyro_raw[1] * hdev->gyro_scale_factor;
    hdev->gyro[2] = (float)hdev->gyro_raw[2] * hdev->gyro_scale_factor;

    return MPU6050_OK;
}

/* 读取温度数据 */
mpu6050_status_t mpu6050_read_temp(mpu6050_handle_t *hdev) {
    uint8_t buffer[2];

    /* 读取温度数据 */
    if (mpu6050_read_bytes(hdev, MPU6050_REG_TEMP_OUT_H, buffer, 2) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 组合高低字节 */
    hdev->temp_raw = (int16_t)((buffer[0] << 8) | buffer[1]);

    /* 转换为实际温度值 (°C) */
    hdev->temperature = (float)hdev->temp_raw / 340.0f + 36.53f;

    return MPU6050_OK;
}

/* 读取所有数据（加速度、陀螺仪、温度） */
mpu6050_status_t mpu6050_read_all(mpu6050_handle_t *hdev) {
    uint8_t buffer[14];

    /* 读取所有数据 */
    if (mpu6050_read_bytes(hdev, MPU6050_REG_ACCEL_XOUT_H, buffer, 14) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    /* 组合加速度数据 */
    hdev->accel_raw[0] = (int16_t)((buffer[0] << 8) | buffer[1]);
    hdev->accel_raw[1] = (int16_t)((buffer[2] << 8) | buffer[3]);
    hdev->accel_raw[2] = (int16_t)((buffer[4] << 8) | buffer[5]);

    /* 组合温度数据 */
    hdev->temp_raw = (int16_t)((buffer[6] << 8) | buffer[7]);

    /* 组合陀螺仪数据 */
    hdev->gyro_raw[0] = (int16_t)((buffer[8] << 8) | buffer[9]);
    hdev->gyro_raw[1] = (int16_t)((buffer[10] << 8) | buffer[11]);
    hdev->gyro_raw[2] = (int16_t)((buffer[12] << 8) | buffer[13]);

    /* 转换为实际物理单位 */
    hdev->accel[0] = (float)hdev->accel_raw[0] * hdev->accel_scale_factor;
    hdev->accel[1] = (float)hdev->accel_raw[1] * hdev->accel_scale_factor;
    hdev->accel[2] = (float)hdev->accel_raw[2] * hdev->accel_scale_factor;

    hdev->temperature = (float)hdev->temp_raw / 340.0f + 36.53f;

    hdev->gyro[0] = (float)hdev->gyro_raw[0] * hdev->gyro_scale_factor;
    hdev->gyro[1] = (float)hdev->gyro_raw[1] * hdev->gyro_scale_factor;
    hdev->gyro[2] = (float)hdev->gyro_raw[2] * hdev->gyro_scale_factor;

    return MPU6050_OK;
}

/* 设置陀螺仪量程 */
mpu6050_status_t mpu6050_set_gyro_scale(mpu6050_handle_t *hdev, mpu6050_gyro_scale_t scale) {
    /* 更新量程设置 */
    hdev->gyro_scale = scale;

    /* 计算新的量程因子 */
    switch (hdev->gyro_scale) {
    case MPU6050_GYRO_SCALE_250DPS:
        hdev->gyro_scale_factor = 250.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_500DPS:
        hdev->gyro_scale_factor = 500.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_1000DPS:
        hdev->gyro_scale_factor = 1000.0f / 32768.0f;
        break;
    case MPU6050_GYRO_SCALE_2000DPS:
        hdev->gyro_scale_factor = 2000.0f / 32768.0f;
        break;
    default:
        hdev->gyro_scale_factor = 250.0f / 32768.0f;
        break;
    }

    /* 写入配置寄存器 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_GYRO_CONFIG, hdev->gyro_scale) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

/* 设置加速度计量程 */
mpu6050_status_t mpu6050_set_accel_scale(mpu6050_handle_t *hdev, mpu6050_accel_scale_t scale) {
    /* 更新量程设置 */
    hdev->accel_scale = scale;

    /* 计算新的量程因子 */
    switch (hdev->accel_scale) {
    case MPU6050_ACCEL_SCALE_2G:
        hdev->accel_scale_factor = 2.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_4G:
        hdev->accel_scale_factor = 4.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_8G:
        hdev->accel_scale_factor = 8.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    case MPU6050_ACCEL_SCALE_16G:
        hdev->accel_scale_factor = 16.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    default:
        hdev->accel_scale_factor = 2.0f * 9.80665f / 32768.0f; /* 转换为m/s² */
        break;
    }

    /* 写入配置寄存器 */
    if (mpu6050_write_byte(hdev, MPU6050_REG_ACCEL_CONFIG, hdev->accel_scale) != MPU6050_OK) {
        return MPU6050_ERROR_COMM;
    }

    return MPU6050_OK;
}

mpu6050_handle_t mpu6050;
void mpu6050_test() {
    char buffer[256];
    /* 初始化MPU6050传感器 */
    if (mpu6050_init(&mpu6050, &hi2c2) != MPU6050_OK) {
        sprintf(buffer, "MPU6050初始化失败！\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        while (1)
            ;
    }

    /* 配置传感器（可选，使用默认配置） */
    mpu6050_set_gyro_scale(&mpu6050, MPU6050_GYRO_SCALE_2000DPS);
    mpu6050_set_accel_scale(&mpu6050, MPU6050_ACCEL_SCALE_4G);

    sprintf(buffer, "MPU6050初始化成功！\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

    while (1) {
        /* 读取所有传感器数据 */
        if (mpu6050_read_all(&mpu6050) == MPU6050_OK) {
            /* 输出加速度数据 */
            sprintf(buffer, "Accel: X=%.2f m/s^2, Y=%.2f m/s^2, Z=%.2f m/s^2\r\n", mpu6050.accel[0],
                    mpu6050.accel[1], mpu6050.accel[2]);
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

            /* 输出陀螺仪数据 */
            sprintf(buffer, "Gyro: X=%.2f '/s, Y=%.2f '/s, Z=%.2f '/s\r\n", mpu6050.gyro[0],
                    mpu6050.gyro[1], mpu6050.gyro[2]);
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

            /* 输出温度数据 */
            sprintf(buffer, "Temp: %.2f 'C\r\n", mpu6050.temperature);
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

            sprintf(buffer, "------------------------\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        } else {
            sprintf(buffer, "读取传感器数据失败！\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        }

        HAL_Delay(500); /* 延时500ms */
    }
}
#endif

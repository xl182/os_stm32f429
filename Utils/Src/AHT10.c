#include "aht10.h"
#if USE_AHT10
/**
 * @brief 等待 AHT10 设备就绪
 * @return 0: 就绪; 1: 超时
 */
static uint8_t aht10_wait_for_ready(void) {
    uint8_t status   = 0;
    uint32_t timeout = 1000; // 最大等待时间（ms）

    while (timeout--) {
        if (HAL_I2C_Master_Receive(&hi2c2, AHT10_READ_ADDR, &status, 1, 100) != HAL_OK) {
            return 2; // I2C通信错误
        }

        if (!(status & AHT10_STATUS_BUSY)) {
            return 0; // 设备就绪
        }

        HAL_Delay(1);
    }

    return 1; // 超时
}

/**
 * @brief 初始化 AHT10 传感器
 * @return 0: 成功; 非0: 失败
 */
uint8_t aht10_init(void) {
    uint8_t cmd[3] = {AHT10_INIT_CMD, 0x08, 0x00}; // 初始化命令+启用校准

    if (HAL_I2C_Master_Transmit(&hi2c2, AHT10_WRITE_ADDR, cmd, 3, 100) != HAL_OK) {
        return 1;
    }

    // 等待初始化完成
    HAL_Delay(100);

    // 检查校准状态
    uint8_t status = 0;
    if (HAL_I2C_Master_Receive(&hi2c2, AHT10_READ_ADDR, &status, 1, 100) != HAL_OK) {
        return 2;
    }

    if (!(status & AHT10_STATUS_CAL)) {
        return 3; // 校准未启用
    }

    return 0; // 初始化成功
}

/**
 * @brief 软复位 AHT10 传感器
 */
void aht10_soft_reset(void) {
    uint8_t cmd = AHT10_SOFT_RESET_CMD;

    // 发送软复位命令
    HAL_I2C_Master_Transmit(&hi2c2, AHT10_WRITE_ADDR, &cmd, 1, 100);

    // 等待复位完成
    HAL_Delay(20);
}

// 读取 AHT10 状态寄存器
uint8_t aht10_get_status(void) {
    uint8_t status = 0;
    HAL_I2C_Master_Receive(&hi2c2, AHT10_READ_ADDR, &status, 1, 100);
    return status;
}

// 检查测量是否完成
bool aht10_is_measurement_done(void) {
    uint8_t status = aht10_get_status();
    return !(status & 0x80); // Busy 标志为 0 时表示完成
}

/**
 * @brief 读取 AHT10 温湿度数据
 * @param humidity: 湿度值指针（单位：%RH）
 * @param temperature: 温度值指针（单位：°C）
 * @return 0: 成功; 1: 设备忙; 2: 未校准; 其他: I2C错误
 */
uint8_t aht10_read_data(float *humidity, float *temperature) {
    uint8_t cmd[3]  = {AHT10_MEASURE_CMD, 0x33, 0x00}; // 触发测量命令
    uint8_t data[6] = {0};
    uint32_t raw_humidity, raw_temperature;

    // 检查设备是否就绪
    uint8_t status = aht10_wait_for_ready();
    if (status != 0) {
        return status;
    }

    // 发送测量命令
    if (HAL_I2C_Master_Transmit(&hi2c2, AHT10_WRITE_ADDR, cmd, 3, 10) != HAL_OK) {
        return 4;
    }

    while (!aht10_is_measurement_done()) {
        HAL_Delay(10);
    }

    // 读取测量结果（6字节）
    if (HAL_I2C_Master_Receive(&hi2c2, AHT10_READ_ADDR, data, 6, 10) != HAL_OK) {
        return 5;
    }

    // 检查状态位
    if (data[0] & AHT10_STATUS_BUSY) {
        return 1; // 设备仍在忙
    }

    if (!(data[0] & AHT10_STATUS_CAL)) {
        return 2; // 设备未校准
    }

    // 解析湿度数据
    raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    *humidity    = (float)raw_humidity * 100.0f / 1048576.0f; // 2^20 = 1048576

    // 解析温度数据
    raw_temperature = ((uint32_t)(data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];
    *temperature    = (float)raw_temperature * 200.0f / 1048576.0f - 50.0f;

    return 0; // 成功
}
#endif

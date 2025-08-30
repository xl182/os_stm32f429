#include "qmc5883l.h"

#if USE_QMC5883L
#include "stdio.h"
#include "string.h"

char buffer[256];
QMC5883L_Handle_t qmc5883l;
/* 向指定寄存器写入数据 */
static QMC5883L_Error_t qmc5883l_write_reg(QMC5883L_Handle_t *hdev, uint8_t reg, uint8_t data) {
    uint8_t buffer[2] = {reg, data};
    if (HAL_I2C_Master_Transmit(hdev->hi2c, QMC5883L_ADDR, buffer, 2, 1000) != HAL_OK) {
        return QMC5883L_ERROR_COMM;
    }
    return QMC5883L_OK;
}

/* 从指定寄存器读取数据 */
static QMC5883L_Error_t qmc5883l_read_reg(QMC5883L_Handle_t *hdev, uint8_t reg, uint8_t *data) {
    if (HAL_I2C_Master_Transmit(hdev->hi2c, QMC5883L_ADDR, &reg, 1, 1000) != HAL_OK) {
        return QMC5883L_ERROR_COMM;
    }
    if (HAL_I2C_Master_Receive(hdev->hi2c, QMC5883L_ADDR, data, 1, 1000) != HAL_OK) {
        return QMC5883L_ERROR_COMM;
    }
    return QMC5883L_OK;
}

/* 从指定寄存器读取多个字节数据 */
static QMC5883L_Error_t qmc5883l_read_regs(QMC5883L_Handle_t *hdev, uint8_t reg, uint8_t *data,
                                           uint8_t len) {
    if (HAL_I2C_Master_Transmit(hdev->hi2c, QMC5883L_ADDR, &reg, 1, 1000) != HAL_OK) {
        return QMC5883L_ERROR_COMM;
    }
    if (HAL_I2C_Master_Receive(hdev->hi2c, QMC5883L_ADDR, data, len, 1000) != HAL_OK) {
        return QMC5883L_ERROR_COMM;
    }
    return QMC5883L_OK;
}

/* 初始化QMC5883L传感器 */
QMC5883L_Error_t qmc5883l_init(QMC5883L_Handle_t *hdev, I2C_HandleTypeDef *hi2c) {
    uint8_t id;

    hdev->hi2c  = hi2c;
    hdev->odr   = QMC5883L_ODR_10HZ;
    hdev->range = QMC5883L_RNG_2G;
    hdev->osr   = QMC5883L_OSR_512;

    /* 读取芯片ID */
    if (qmc5883l_read_reg(hdev, QMC5883L_CHIP_ID, &id) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 验证芯片ID */
    if (id != 0xFF) {
        return QMC5883L_ERROR_ID;
    }

    /* 复位传感器 */
    if (qmc5883l_reset(hdev) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 配置传感器 */
    return qmc5883l_configure(hdev, hdev->odr, hdev->range, hdev->osr);
}

/* 配置QMC5883L传感器 */
QMC5883L_Error_t qmc5883l_configure(QMC5883L_Handle_t *hdev, uint8_t odr, uint8_t range,
                                    uint8_t osr) {
    uint8_t config;

    /* 保存配置参数 */
    hdev->odr   = odr;
    hdev->range = range;
    hdev->osr   = osr;

    /* 设置比例因子，用于单位转换 */
    if (range == QMC5883L_RNG_2G) {
        hdev->scale = 0.3; // 0.3 milligauss/LSB
    } else if (range == QMC5883L_RNG_8G) {
        hdev->scale = 1.2; // 1.2 milligauss/LSB
    } else {
        return QMC5883L_ERROR_PARAM;
    }

    /* 配置寄存器1: 模式、数据速率、量程、过采样率 */
    config = osr | range | odr | QMC5883L_MODE_CONTINUOUS;
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG1, config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 配置寄存器2: 中断使能、设置/复位周期 */
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG2, 0x01) != QMC5883L_OK) { // SET/RESET周期=1
        return QMC5883L_ERROR_COMM;
    }

    return QMC5883L_OK;
}

/* 复位QMC5883L传感器 */
QMC5883L_Error_t qmc5883l_reset(QMC5883L_Handle_t *hdev) {
    /* 写入复位命令 */
    if (qmc5883l_write_reg(hdev, QMC5883L_RESET, 0x01) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 等待复位完成 */
    HAL_Delay(2);

    return QMC5883L_OK;
}

/* 读取原始数据 */
QMC5883L_Error_t qmc5883l_read_raw(QMC5883L_Handle_t *hdev) {
    uint8_t buffer[6];
    uint8_t status;

    /* 检查状态寄存器 */
    if (qmc5883l_read_reg(hdev, QMC5883L_STATUS, &status) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 检查是否有溢出 */
    if (status & QMC5883L_STATUS_OVL) {
        return QMC5883L_ERROR_OVERFLOW;
    }

    /* 检查数据是否就绪 */
    if (!(status & QMC5883L_STATUS_DRDY)) {
        return QMC5883L_OK; // 数据未就绪，但不是错误
    }

    /* 读取X、Y、Z轴数据 */
    if (qmc5883l_read_regs(hdev, QMC5883L_XOUT_LSB, buffer, 6) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 转换为有符号16位整数 */
    hdev->x_raw = (int16_t)((buffer[1] << 8) | buffer[0]);
    hdev->y_raw = (int16_t)((buffer[3] << 8) | buffer[2]);
    hdev->z_raw = (int16_t)((buffer[5] << 8) | buffer[4]);

    return QMC5883L_OK;
}

/* 读取并转换为实际磁场强度值 */
QMC5883L_Error_t qmc5883l_read(QMC5883L_Handle_t *hdev) {
    QMC5883L_Error_t error;

    /* 读取原始数据 */
    error = qmc5883l_read_raw(hdev);
    if (error != QMC5883L_OK) {
        return error;
    }

    /* 转换为实际磁场强度(milligauss) */
    hdev->x = (float)hdev->x_raw * hdev->scale;
    hdev->y = (float)hdev->y_raw * hdev->scale;
    hdev->z = (float)hdev->z_raw * hdev->scale;

    return QMC5883L_OK;
}

/* 检查数据是否就绪 */
QMC5883L_Error_t qmc5883l_is_data_ready(QMC5883L_Handle_t *hdev, uint8_t *ready) {
    uint8_t status;

    /* 读取状态寄存器 */
    if (qmc5883l_read_reg(hdev, QMC5883L_STATUS, &status) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 检查数据就绪标志 */
    *ready = (status & QMC5883L_STATUS_DRDY) ? 1 : 0;

    return QMC5883L_OK;
}

/* 读取温度数据 */
QMC5883L_Error_t qmc5883l_get_temperature(QMC5883L_Handle_t *hdev) {
    uint8_t buffer[2];

    /* 读取温度数据 */
    if (qmc5883l_read_regs(hdev, QMC5883L_TOUT_LSB, buffer, 2) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 转换为有符号16位整数 */
    hdev->temperature_raw = (int16_t)((buffer[1] << 8) | buffer[0]);

    /* 转换为实际温度值(°C) */
    hdev->temperature = 25.0f + (float)hdev->temperature_raw / 100.0f;

    return QMC5883L_OK;
}

/* 设置工作模式 */
QMC5883L_Error_t qmc5883l_set_mode(QMC5883L_Handle_t *hdev, uint8_t mode) {
    uint8_t config;

    /* 读取当前配置 */
    if (qmc5883l_read_reg(hdev, QMC5883L_CONFIG1, &config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 清除模式位并设置新模式 */
    config &= ~0x03;
    config |= mode;

    /* 写入新配置 */
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG1, config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    return QMC5883L_OK;
}

/* 设置数据输出速率 */
QMC5883L_Error_t qmc5883l_set_sampling_rate(QMC5883L_Handle_t *hdev, uint8_t odr) {
    uint8_t config;

    /* 读取当前配置 */
    if (qmc5883l_read_reg(hdev, QMC5883L_CONFIG1, &config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 清除输出数据速率位并设置新值 */
    config &= ~0x0C;
    config |= odr;

    /* 保存配置参数 */
    hdev->odr = odr;

    /* 写入新配置 */
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG1, config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    return QMC5883L_OK;
}

/* 设置测量量程 */
QMC5883L_Error_t qmc5883l_set_range(QMC5883L_Handle_t *hdev, uint8_t range) {
    uint8_t config;

    /* 读取当前配置 */
    if (qmc5883l_read_reg(hdev, QMC5883L_CONFIG1, &config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 清除量程位并设置新值 */
    config &= ~0x10;
    config |= range;

    /* 保存配置参数 */
    hdev->scale = 1.2; // 1.2 milligauss/LSB
    hdev->range = range;

    /* 设置比例因子，用于单位转换 */
    if (range == QMC5883L_RNG_2G) {
        hdev->scale = 0.3; // 0.3 milligauss/LSB
    } else if (range == QMC5883L_RNG_8G) {
    } else {
        return QMC5883L_ERROR_PARAM;
    }

    /* 写入新配置 */
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG1, config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    return QMC5883L_OK;
}

/* 设置过采样率 */
QMC5883L_Error_t qmc5883l_set_oversampling(QMC5883L_Handle_t *hdev, uint8_t osr) {
    uint8_t config;

    /* 读取当前配置 */
    if (qmc5883l_read_reg(hdev, QMC5883L_CONFIG1, &config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    /* 清除过采样率位并设置新值 */
    config &= ~0xC0;
    config |= osr;

    /* 保存配置参数 */
    hdev->osr = osr;

    /* 写入新配置 */
    if (qmc5883l_write_reg(hdev, QMC5883L_CONFIG1, config) != QMC5883L_OK) {
        return QMC5883L_ERROR_COMM;
    }

    return QMC5883L_OK;
}

float calculate_heading(float x, float y) {
    float heading_rad = atan2(y, x);

    /* 将弧度转换为度数 */
    float heading_deg = heading_rad * (180.0 / M_PI);

    /* 确保角度在0-360度之间 */
    if (heading_deg < 0) {
        heading_deg += 360.0;
    }

    return heading_deg;
}


void qmc8553_test() {
    /* 初始化QMC5883L传感器 */
    if (qmc5883l_init(&qmc5883l, &hi2c2) != QMC5883L_OK) {
        sprintf(buffer, "QMC5883L init error\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
        while (1)
            ;
    }

    /* 配置传感器（可选，使用默认配置） */
    qmc5883l_configure(&qmc5883l, QMC5883L_ODR_10HZ, /* 输出数据速率：10Hz */
                       QMC5883L_RNG_2G,              /* 量程：2G */
                       QMC5883L_OSR_512);            /* 过采样率：512 */

    sprintf(buffer, "QMC5883L ini success\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

    while (1) {
        uint8_t data_ready = 0;

        /* 检查数据是否就绪 */
        if (qmc5883l_is_data_ready(&qmc5883l, &data_ready) != QMC5883L_OK) {
            sprintf(buffer, "读取数据就绪状态失败！\r\n");
            HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
            HAL_Delay(1000);
            continue;
        }

        /* 如果数据就绪，读取并打印磁场数据 */
        if (data_ready) {
            if (qmc5883l_read(&qmc5883l) == QMC5883L_OK) {
                /* 计算磁方位角（度数） */
                float heading = calculate_heading(qmc5883l.x, qmc5883l.y);

                /* 读取并打印温度数据 */
                if (qmc5883l_get_temperature(&qmc5883l) == QMC5883L_OK) {
                    sprintf(buffer,
                            "X: %.2f mG, Y: %.2f mG, Z: %.2f mG, Heading: %.2f°, Temp: %.2f°C\r\n",
                            qmc5883l.x, qmc5883l.y, qmc5883l.z, heading, qmc5883l.temperature);
                } else {
                    sprintf(buffer, "X: %.2f mG, Y: %.2f mG, Z: %.2f mG, Heading: %.2f°\r\n",
                            qmc5883l.x, qmc5883l.y, qmc5883l.z, heading);
                }
                HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
            } else {
                sprintf(buffer, "读取传感器数据失败！\r\n");
                HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
            }
        }
    }
}
#endif

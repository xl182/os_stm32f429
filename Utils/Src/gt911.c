#include "gt911.h"
#if USE_GT911
// 私有函数声明
static void i2c_delay(void);
static void i2c_start(void);
static void i2c_stop(void);
static uint8_t i2c_wait_ack(void);
static void i2c_send_byte(uint8_t data);
static uint8_t i2c_read_byte(uint8_t ack);
static void i2c_sda_input_mode(void);
static void i2c_sda_output_mode(void);

void gt911_gpio_init(void) {
    // 初始化GPIO时钟
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    // SDA配置
    GPIO_InitTypeDef gpio_init;
    gpio_init.Pin   = T_SDA_PIN;
    gpio_init.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pull  = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(T_SDA_PORT, &gpio_init);

    // SCL配置
    gpio_init.Pin = T_SCK_PIN;
    HAL_GPIO_Init(T_SCK_PORT, &gpio_init);

    // RST配置
    gpio_init.Pin  = T_RST_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(T_RST_PORT, &gpio_init);

    // INT配置
    gpio_init.Pin  = T_INT_PIN;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(T_INT_PORT, &gpio_init);
}

// 初始化函数
void gt911_init(void) {
    gt911_gpio_init();

    // 复位触摸芯片
    gt911_reset();

    // 检查触摸芯片ID
    uint8_t id[4] = {0};
    uint8_t retry = 3;

    while (retry--) {
        if (gt911_read_reg(GT911_PID_REG, id, 4) == 0) {
            id[4] = '\0';
            break;
        }
        delay_ms(10);
    }

    if (retry == 0) {
        printf("GT911 not detected!\r\n");
    } else {
        printf("GT911 detected, PID: %s\r\n", id);
    }

    // 发送配置命令
    uint8_t cmd = 0x02;
    gt911_write_reg(GT911_CTRL_REG, &cmd, 1);
    delay_ms(5);
    cmd = 0x00;
    gt911_write_reg(GT911_CTRL_REG, &cmd, 1);
    delay_ms(5);
}

// 复位触摸芯片
void gt911_reset(void) {
    // 复位序列
    HAL_GPIO_WritePin(T_RST_PORT, T_RST_PIN, GPIO_PIN_RESET); // 拉低复位
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET);   // 释放SDA
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);   // 释放SCL
    delay_ms(20);                                            // 延长复位时间

    HAL_GPIO_WritePin(T_RST_PORT, T_RST_PIN, GPIO_PIN_SET); // 释放复位
    delay_ms(60);                                          // 延长复位后等待时间
}

// 读寄存器 (带重试机制)
uint8_t gt911_read_reg(uint16_t reg, uint8_t *buf, uint8_t len) {
    uint8_t retry       = 3;
    uint8_t reg_addr[2] = {reg >> 8, reg & 0xFF};

    while (retry--) {
        i2c_start();

        // 发送设备地址 (写模式)
        i2c_send_byte(GT911_WRITE_ADDR);
        if (!i2c_wait_ack()) goto error;

        // 发送寄存器地址高字节
        i2c_send_byte(reg_addr[0]);
        if (!i2c_wait_ack()) goto error;

        // 发送寄存器地址低字节
        i2c_send_byte(reg_addr[1]);
        if (!i2c_wait_ack()) goto error;

        i2c_start();

        // 发送设备地址 (读模式)
        i2c_send_byte(GT911_READ_ADDR);
        if (!i2c_wait_ack()) goto error;

        // 读取数据
        for (uint8_t i = 0; i < len; i++) {
            buf[i] = i2c_read_byte(i == (len - 1) ? 0 : 1);
        }

        i2c_stop();
        return 0; // 成功

    error:
        printf("GT911 read reg error\r\n");
        i2c_stop();
        delay_ms(1);
    }
    return 1; // 失败
}

// 写寄存器 (带重试机制)
uint8_t gt911_write_reg(uint16_t reg, uint8_t *data, uint8_t len) {
    uint8_t retry       = 3;
    uint8_t reg_addr[2] = {reg >> 8, reg & 0xFF};

    while (retry--) {
        i2c_start();

        // 发送设备地址 (写模式)
        i2c_send_byte(GT911_WRITE_ADDR);
        if (!i2c_wait_ack()) goto error;

        // 发送寄存器地址高字节
        i2c_send_byte(reg_addr[0]);
        if (!i2c_wait_ack()) goto error;

        // 发送寄存器地址低字节
        i2c_send_byte(reg_addr[1]);
        if (!i2c_wait_ack()) goto error;

        // 发送数据
        for (uint8_t i = 0; i < len; i++) {
            i2c_send_byte(data[i]);
            if (!i2c_wait_ack()) goto error;
        }

        i2c_stop();
        return 0; // 成功

    error:
        i2c_stop();
        delay_ms(1);
    }
    return 1; // 失败
}

// 扫描触摸点 (带错误处理)
uint8_t gt911_scan(gt911_touch_t *touch) {
    uint8_t status;
    uint8_t buf[7];

    // 清零触摸结构体
    memset(touch, 0, sizeof(gt911_touch_t));

    // 读取状态寄存器 (带重试)
    if (gt911_read_reg(GT911_GSTID_REG, &status, 1)) {
        touch->valid = 0;
        return 0;
    }

    touch->points = status & 0x0F;

    // 清除状态位 (如果需要)
    if (status & 0x80) {
        uint8_t clear = 0;
        gt911_write_reg(GT911_GSTID_REG, &clear, 1);
    }

    // 只处理有效触摸点 (1-10点)
    if (touch->points > 0 && touch->points <= 10) {
        for (uint8_t i = 0; i < touch->points; i++) {
            uint16_t reg = GT911_TP1_REG + i * 8;

            // 带重试的读取
            uint8_t retry = 2;
            while (retry--) {
                if (gt911_read_reg(reg, buf, 7) == 0) {
                    touch->x[i]      = ((uint16_t)buf[3] << 8) | buf[2];
                    touch->y[i]      = ((uint16_t)buf[1] << 8) | buf[0];
                    touch->status[i] = buf[4];
                    break;
                }

                // 读取失败时重置芯片
                if (retry == 0) {
                    touch->x[i]      = 0xFFFF;
                    touch->y[i]      = 0xFFFF;
                    touch->status[i] = 0xFF;
                    gt911_reset();
                    delay_ms(10);
                }
            }
        }

        // 坐标校准
        gt911_calibrate_coordinates(touch);
        return touch->points;
    }

    touch->valid = 0;
    return 0;
}

// 坐标校准函数 (全面错误处理)
void gt911_calibrate_coordinates(gt911_touch_t *touch) {
    uint8_t valid_points = 0;

    for (uint8_t i = 0; i < touch->points; i++) {
        // 1. 过滤无效状态 (Status > 128 为无效)
        if (touch->status[i] > 128) {
            touch->x[i] = 0xFFFF;
            touch->y[i] = 0xFFFF;
            continue;
        }
        uint16_t temp = touch->x[i];
        touch->x[i]   = touch->y[i];
        touch->y[i]   = temp;

        // 2. 过滤特殊错误值 (0xFFFF)
        if (touch->x[i] == 0xFFFF || touch->y[i] == 0xFFFF ||
            ((touch->x[i] == 0) || (touch->y[i] == 0))) {
            continue;
        }

        // 3. 边界检查
        if (touch->x[i] >= SCREEN_WIDTH) {
            touch->x[i] = SCREEN_WIDTH - 1;
        }

        if (touch->y[i] >= SCREEN_HEIGHT) {
            touch->y[i] = SCREEN_HEIGHT - 1;
        }

        // 4. 特殊处理接近边界的值
        if (touch->y[i] > SCREEN_HEIGHT - 5) {
            touch->y[i] = SCREEN_HEIGHT - 1;
        }

        if (touch->x[i] > SCREEN_WIDTH - 5) {
            touch->x[i] = SCREEN_WIDTH - 1;
        }

        // 5. 处理起始位置异常
        if (touch->x[i] < 10 && touch->y[i] < 10) {
            touch->x[i] = 0;
            touch->y[i] = 0;
        }

        // 记录有效点
        valid_points++;
    }

    // 更新有效点数
    touch->points = valid_points;
    touch->valid  = (valid_points > 0) ? 1 : 0;
}

// ============================================================
// I2C 底层驱动实现
// ============================================================

// I2C延时 (约1us)
static void i2c_delay(void) {
    delay_us(4);
}

// I2C起始信号
static void i2c_start(void) {
    // 设置SDA和SCL为高
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
    i2c_delay();

    // 拉低SDA（起始条件）
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_RESET);
    i2c_delay();

    // 拉低SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
    i2c_delay();
}

// I2C停止信号
static void i2c_stop(void) {
    // 确保SCL为低
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_RESET);
    i2c_delay();

    // 拉高SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
    i2c_delay();

    // 拉高SDA（停止条件）
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET);
    i2c_delay();
}

// 发送一个字节
static void i2c_send_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        // 拉低SCL
        HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
        i2c_delay();

        // 设置SDA
        if (data & 0x80) {
            HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_RESET);
        }
        data <<= 1;
        i2c_delay();

        HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
        i2c_delay();
    }

    // 最后将SCL拉低，为ACK做准备
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
    i2c_delay();
}

// 设置SDA为输入模式
static void i2c_sda_input_mode(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = T_SDA_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull             = GPIO_PULLUP;
    HAL_GPIO_Init(T_SDA_PORT, &GPIO_InitStruct);
}

// 设置SDA为输出模式
static void i2c_sda_output_mode(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin              = T_SDA_PIN;
    GPIO_InitStruct.Mode             = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull             = GPIO_NOPULL;
    GPIO_InitStruct.Speed            = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(T_SDA_PORT, &GPIO_InitStruct);
}

// 等待ACK
static uint8_t i2c_wait_ack(void) {
    uint8_t ack = 0;

    // 释放SDA（设置为输入）
    i2c_sda_input_mode();

    // 拉高SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
    i2c_delay();

    // 读取SDA (0表示ACK, 1表示NACK)
    ack = HAL_GPIO_ReadPin(T_SDA_PORT, T_SDA_PIN);

    // 拉低SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
    i2c_delay();

    // 将SDA重新设置为输出
    i2c_sda_output_mode();

    return (ack == GPIO_PIN_RESET); // 返回1表示收到ACK
}

// 读取一个字节
static uint8_t i2c_read_byte(uint8_t ack) {
    uint8_t data = 0;

    i2c_sda_input_mode();

    for (uint8_t i = 0; i < 8; i++) {
        // 拉高SCL
        HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
        i2c_delay();

        // 读取SDA
        data <<= 1;
        if (HAL_GPIO_ReadPin(T_SDA_PORT, T_SDA_PIN)) {
            data |= 0x01;
        }

        // 拉低SCL
        HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
        i2c_delay();
    }

    // 将SDA设置为输出
    i2c_sda_output_mode();

    // 发送ACK/NACK
    if (ack) {
        HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_RESET); // ACK
    } else {
        HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET); // NACK
    }

    // 拉高SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_SET);
    i2c_delay();

    // 拉低SCL
    HAL_GPIO_WritePin(T_SCK_PORT, T_SCK_PIN, GPIO_PIN_RESET);
    i2c_delay();

    // 释放SDA
    HAL_GPIO_WritePin(T_SDA_PORT, T_SDA_PIN, GPIO_PIN_SET);

    return data;
}
#if 1
void gt911_i2c_scan(void) {
    uint8_t found = 0;
    printf("Scanning I2C bus...\r\n");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        i2c_start();
        // 发送设备地址（写模式，即左移1位后最低位为0）
        i2c_send_byte(addr << 1);
        if (i2c_wait_ack() == 1) { // 注意：i2c_wait_ack返回1表示收到ACK
            printf("Device found at address: 0x%02X (7-bit)\r\n", addr);
            found = 1;
        }
        i2c_stop();
        delay_ms(1); // 稍微延时，确保总线释放
    }

    if (!found) {
        printf("No I2C devices found\r\n");
    }
}

gt911_touch_t touch;
void touch_test() {
    printf("touch test\r\n");
    // 初始化GT911
    gt911_gpio_init();
    gt911_i2c_scan();
    gt911_init();

    while (1) {
        // 扫描触摸点
        if (gt911_scan(&touch)) {
            for (uint8_t i = 0; i < touch.points; i++) {
                printf("Touch %d: X=%d, Y=%d, Status=%u\r\n", i + 1, touch.x[i], touch.y[i],
                       touch.status);
            }
        }

        delay_ms(20); // 20ms扫描间隔
    }
}
#endif
#endif

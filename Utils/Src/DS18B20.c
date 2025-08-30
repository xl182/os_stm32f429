#include "DS18B20.h"

#if USE_DS18B20 == 1
#define read_temp_bus() HAL_GPIO_ReadPin(TEMP_BUS_GPIO_Port, TEMP_BUS_Pin)
#define write_temp_bus(status) HAL_GPIO_WritePin(TEMP_BUS_GPIO_Port, TEMP_BUS_Pin, status)

void set_temp_bus_input() {
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = TEMP_BUS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(TEMP_BUS_GPIO_Port, &GPIO_InitStruct);
}

void set_temp_bus_output() {
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = TEMP_BUS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TEMP_BUS_GPIO_Port, &GPIO_InitStruct);
}

void DS18B20_reset(void) {
    set_temp_bus_output();

    write_temp_bus(GPIO_PIN_RESET);
    delay_us(750);
    write_temp_bus(GPIO_PIN_SET);
    delay_us(15);
}

/**************************************************************************************
* 描  述 : 检测从机给主机返回的存在脉冲
* 入  参 : 无
* 返回值 : 0：成功   1：失败
**************************************************************************************/
static uint8_t DS18B20_presence(void) {
    uint8_t pulse_time = 0;

    set_temp_bus_input();

    /* 等待存在脉冲的到来，存在脉冲为一个60~240us的低电平信号
     * 如果存在脉冲没有来则做超时处理，从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
     */
    while (read_temp_bus() && pulse_time < 100) {
        pulse_time++;
        delay_us(1);
    }

    if (pulse_time >= 100)  //经过100us后，存在脉冲都还没有到来
        return 1;             //读取失败
    else                 //经过100us后，存在脉冲到来
        pulse_time = 0;    //清零计时变量

    while (!read_temp_bus() && pulse_time < 240)  // 存在脉冲到来，且存在的时间不能超过240us
    {
        pulse_time++;
        delay_us(1);
    }
    if (pulse_time >= 240) // 存在脉冲到来，且存在的时间超过了240us
        return 1;        //读取失败
    else
        return 0;
}

/**************************************************************************************
* 描  述 : 从DS18B20读取一个bit
* 入  参 : 无
* 返回值 : uint8_t
**************************************************************************************/
static uint8_t DS18B20_read_bit(void) {
    uint8_t dat;

    /* 读0和读1的时间至少要大于60us */
    set_temp_bus_output();
    /* 读时间的起始：必须由主机产生 >1us <15us 的低电平信号 */
    write_temp_bus(GPIO_PIN_RESET);
    delay_us(10);

    /* 设置成输入，释放总线，由外部上拉电阻将总线拉高 */
    set_temp_bus_input();

    if (read_temp_bus() == SET)
        dat = 1;
    else
        dat = 0;

    /* 这个延时参数请参考时序图 */
    delay_us(45);

    return dat;
}

/**************************************************************************************
* 描  述 : 从DS18B20读一个字节，低位先行
* 入  参 : 无
* 返回值 : uint8_t
**************************************************************************************/
uint8_t DS18B20_read_byte(void) {
    uint8_t i, j, dat = 0;

    for (i = 0; i < 8; i++) {
        j = DS18B20_read_bit();        //从DS18B20读取一个bit
        dat = (dat) | (j << i);
    }

    return dat;
}

/**************************************************************************************
* 描  述 : 写一个字节到DS18B20，低位先行
* 入  参 : uint8_t
* 返回值 : 无
**************************************************************************************/
void DS18B20_write_byte(uint8_t dat) {
    uint8_t i, testb;
    set_temp_bus_output();

    for (i = 0; i < 8; i++) {
        testb = dat & 0x01;
        dat = dat >> 1;
        /* 写0和写1的时间至少要大于60us */
        if (testb) {
            write_temp_bus(GPIO_PIN_RESET);
            delay_us(8);   //1us < 这个延时 < 15us

            write_temp_bus(GPIO_PIN_SET);
            delay_us(58);    //58us+8us>60us
        } else {
            write_temp_bus(GPIO_PIN_RESET);
            /* 60us < Tx 0 < 120us */
            delay_us(70);

            write_temp_bus(GPIO_PIN_SET);
            /* 1us < Trec(恢复时间) < 无穷大*/
            delay_us(2);
        }
    }
}

/**************************************************************************************
* 描  述 : 起始DS18B20
* 入  参 : 无
* 返回值 : 无
**************************************************************************************/
void DS18B20_start(void) {
    DS18B20_reset();               //主机给从机发送复位脉冲
    DS18B20_presence();           //检测从机给主机返回的存在脉冲
    DS18B20_write_byte(0XCC);         // 跳过 ROM
    DS18B20_write_byte(0X44);         // 开始转换
}

/**************************************************************************************
* 描  述 : DS18B20初始化函数
* 入  参 : 无
* 返回值 : uint8_t
**************************************************************************************/
uint8_t DS18B20_init(void) {
    DS18B20_reset();
    return DS18B20_presence();
}

/**************************************************************************************
* 描  述 : 从DS18B20读取温度值
* 入  参 : 无
* 返回值 : float
**************************************************************************************/
float DS18B20_get_temp(void) {
    uint8_t tpmsb, tplsb;
    uint16_t s_tem;
    float f_tem;

    DS18B20_reset();
    DS18B20_presence();
    DS18B20_write_byte(0XCC);                /* 跳过 ROM */
    DS18B20_write_byte(0X44);                /* 开始转换 */

    DS18B20_reset();
    DS18B20_presence();
    DS18B20_write_byte(0XCC);                /* 跳过 ROM */
    DS18B20_write_byte(0XBE);                /* 读温度值 */

    tplsb = DS18B20_read_byte();
    tpmsb = DS18B20_read_byte();

    s_tem = tpmsb << 8;
    s_tem = s_tem | tplsb;

    if (s_tem < 0)        /* 负温度 */
        f_tem = (float) ((~s_tem + 1) * 0.0625);
    else
        f_tem = (float) (s_tem * 0.0625);
    return f_tem;
}
#endif

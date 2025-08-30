// #include "utils.h"


// void i2c_scan(I2C_HandleTypeDef *hi2c) {
//     char buffer[256];
//     uint8_t device_count = 0;

//     sprintf(buffer, "i2c bus scanning...\r\n");
//     HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);

//     /* 遍历所有可能的I2C地址 (8-0x77) */
//     for (uint16_t address = 8; address < 120; address++) {
//         /* 尝试与设备通信 */
//         if (HAL_I2C_IsDeviceReady(hi2c, (address << 1), 1, 10) == HAL_OK) {
//             sprintf(buffer, "found i2c device: address = 0x%02X\r\n", address);
//             HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
//             device_count++;
//         }
//         HAL_Delay(1);
//     }

//     sprintf(buffer, "scan done, found %d devices\r\n", device_count);
//     HAL_UART_Transmit(&huart1, (uint8_t *)buffer, strlen(buffer), 1000);
// }
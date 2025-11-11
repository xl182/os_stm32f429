#include "callback.h"

char c, rx_data[64];
extern QueueHandle_t uartQueueHandle;
extern QueueHandle_t uartDisplayQueue;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
    static uint8_t receive_byte         = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    UartDisplayMsg msg;
    if (c != '\r' && receive_byte <= 63) {
        rx_data[receive_byte] = c;
        receive_byte += 1;
    }
    if (c == '\n' || receive_byte >= 63) {
        rx_data[receive_byte - 1] = '\0';
        strcpy(msg.data, rx_data);
        xQueueSendFromISR(uartQueueHandle, rx_data, &xHigherPriorityTaskWoken);
        xQueueSendFromISR(uartDisplayQueue, &msg, &xHigherPriorityTaskWoken);
        memset(rx_data, 0, sizeof(rx_data));
        receive_byte = 0;
    }
    HAL_UART_Receive_DMA(&huart1, &c, 1);
}

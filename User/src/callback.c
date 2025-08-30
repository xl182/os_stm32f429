#include "callback.h"

char c, rx_data[64];
extern QueueHandle_t uartQueueHandle;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
    static uint8_t receive_byte         = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (c != '\r') {
        receive_byte += 1;
        rx_data[receive_byte - 1] = c;
    }
    if (c == '\n') {
        rx_data[receive_byte - 1] = '\0';
        xQueueSendFromISR(uartQueueHandle, rx_data, &xHigherPriorityTaskWoken);
        memset(rx_data, 0, sizeof(rx_data));
        receive_byte = 0;
    }
    c = 0;
    HAL_UART_Receive_DMA(&huart1, &c, 1);
}

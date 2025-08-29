#include "user.h"

char c, rx_data[64];
extern QueueHandle_t uartQueueHandle;
FATFS USBFatFs;

void StartUartTask(void *pvParameters) {
    for (;;) {
        uint8_t task_rx_data[64];
        if (xQueueReceive(uartQueueHandle, task_rx_data, portMAX_DELAY)) {
            printf("%s\r\n", task_rx_data);
        }
    }
}

void StartInitTask(void *pvParameters) {
    MX_SDIO_SD_Init();
    MX_FATFS_Init();
    FRESULT res = f_mount(&USBFatFs, "0:", 1);
    if (res == FR_OK) {
        printf("SD card mounted successfully.\n");
    } else {
        printf("Failed to mount SD card. Error: %d\n", res);
    }
    MX_USB_DEVICE_Init();
    vTaskDelete(NULL);
}

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    printf("printf test\r\n");
    MX_FMC_Init();
    MX_RTC_Init();

    MX_FREERTOS_Init();
    vTaskStartScheduler();
    while (1) {
    }
}

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

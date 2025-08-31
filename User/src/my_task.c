#include "my_task.h"

extern QueueHandle_t uartQueueHandle;
FATFS USBFatFs;

void StartDefaultTask(void *argument) {
    for (;;) {
        osDelay(1000);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    }
}

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
    usb_reset();
    MX_USB_DEVICE_Init();
    while (1)
        ;
}

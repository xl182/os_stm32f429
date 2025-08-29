#include "user.h"

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SDIO_SD_Init();
    MX_USART1_UART_Init();
    printf("printf test\r\n");
    MX_FMC_Init();
    MX_RTC_Init();

    MX_FATFS_Init();
    static FATFS USBFatFs;
    FRESULT res = f_mount(&USBFatFs, "0:", 1);
    if(res == FR_OK) {
        printf("SD card mounted successfully.\n");
    } else {
        printf("Failed to mount SD card. Error: %d\n", res);
    }
    MX_USB_DEVICE_Init();
    while (1) {
    }
}

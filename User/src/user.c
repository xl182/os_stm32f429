#include "user.h"

extern char c;

int main() {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    printf("printf test\r\n");
    MX_FMC_Init();
    MX_RTC_Init();
    HAL_UART_Receive_DMA(&huart1, &c, 1);
    MX_FREERTOS_Init();
    vTaskStartScheduler();
    while (1) {
    }
}

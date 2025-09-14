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

    void *addr = sdram_alloc(9045); 
    printf("sdram alloc addr: %x\r\n", addr);
    for(int i = 0; i < 9045; i++) {
        ((uint8_t *)addr)[i] = i % 256;
    }
    for(int i = 0; i < 9045; i++) {
        if(((uint8_t *)addr)[i] != (i % 256)) {
            printf("sdram alloc addr check error at %d\r\n", i);
            break;
        }
    }

    MX_FREERTOS_Init();
    vTaskStartScheduler();
    while (1) {
    }
}

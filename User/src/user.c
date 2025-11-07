#include "user.h"

bool enter_os = false;
extern char c;

void HAL_Delay(uint32_t Delay) {
    if (enter_os) {
        osDelay(Delay);
    } else {
        uint32_t tickstart = HAL_GetTick();
        uint32_t wait      = Delay;
        /* Add a freq to guarantee minimum wait */
        if (wait < HAL_MAX_DELAY) {
            wait += (uint32_t)(uwTickFreq);
        }
        while ((HAL_GetTick() - tickstart) < wait) {
        }
    }
}

int main() {
    HAL_Init();
    SystemClock_Config();
    command_init();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    printf("printf test\r\n");
    MX_FMC_Init();
    MX_RTC_Init();
    MX_I2C2_Init();

    // custom init
    aht10_init();

    HAL_UART_Receive_DMA(&huart1, &c, 1);
    enter_os = true;
    MX_FREERTOS_Init();
    vTaskStartScheduler();
    while (1) {
    }
}

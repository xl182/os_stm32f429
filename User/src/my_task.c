#include "my_task.h"

FATFS USB_FatFs;    /* File system object for USB logical drive */

char USB_Path[4];   /* USB logical drive path */

extern QueueHandle_t uartQueueHandle;

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

uint8_t log_flag = 0;
void StartInitTask(void *pvParameters) {
    lv_init();
    printf("lv init finished\r\n");
    lv_port_disp_init();
    printf("lv disp init finished\r\n");
    lv_port_indev_init();
    printf("lv indev init finished\r\n");
    lv_port_fs_init();
    printf("lv fs init finished\r\n");

    printf("start to load font\r\n");
    load_font();
    gui_init();
    printf("gui init finished\r\n");
    setup_ui(ui);
    printf("setup ui finished\r\n");
    events_init(ui);
    printf("events init finished\r\n");
    custom_init(ui);
    printf("lvgl init finished\r\n");

    // log_flag = 1;
    usb_reset();
    MX_USB_DEVICE_Init();

    while (1) {
        osDelay(10);
        lv_task_handler();
    }
}

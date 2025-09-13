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

uint8_t log_flag = 0;
void StartInitTask(void *pvParameters) {
    // usb_reset();
    // MX_USB_DEVICE_Init();

    // FRESULT res = f_mount(&USBFatFs, "0:", 1);
    // if (res == FR_OK) {
    //     printf("SD card mounted successfully.\r\n");
    // } else {
    //     printf("Failed to mount SD card. Error: %d\r\n", res);
    // }
    // FIL file;
    // FRESULT result = f_open(&file, "0:/test.txt", FA_WRITE | FA_OPEN_ALWAYS);
    // if(result != FR_OK) {
    //     printf("Failed to open file. Error: %d\r\n", result);
    // }
    // result = f_write(&file, "Hello, 2208\r\n", 13, NULL);
    // if(result != FR_OK) {
    //     printf("Failed to write to file. Error: %d\r\n", result);
    // }
    // result = f_close(&file);
    // if(result != FR_OK) {
    //     printf("Failed to close file. Error: %d\r\n", result);
    // }
    // result = f_open(&file, "0:/test.txt", FA_READ | FA_OPEN_ALWAYS);
    // if(result != FR_OK) {
    //     printf("Failed to open file. Error: %d\r\n", result);
    // }
    // char read_data[50];
    // int real_read_num = 0;
    // result = f_read(&file, read_data, 13, &real_read_num);
    // if(result != FR_OK) {
    //     printf("Failed to read file. Error: %d\r\n", result);
    // }
    // result = f_close(&file);
    // read_data[real_read_num] = 0;  // Null-terminate the string
    // if(result != FR_OK) {
    //     printf("Failed to close file. Error: %d\r\n", result);
    // }
    // printf("Read from file: %s, real read num %d\r\n", read_data, real_read_num);

    lv_init();
    printf("lv init finished\r\n");
    lv_port_disp_init();
    printf("lv disp init finished\r\n");
    lv_port_indev_init();
    printf("lv indev init finished\r\n");
    lv_port_fs_init();
    printf("lv fs init finished\r\n");

    FIL fil;
    if (f_open(&fil, "0:/test.txt", FA_READ) != FR_OK) {
        printf("File not found: %s\r\n", "0:/test.txt");
    };
    uint8_t read_data[100], bytes_read;
    FRESULT res     = f_read(&fil, read_data, 13, &bytes_read);
    read_data[bytes_read] = 0;
    if (res != FR_OK) {
        printf("read font file: %s, error code %d, size: %d\r\n", "0:/test.txt", res, 13);
    }
    printf("Read from file: %s, real read num %d\r\n", read_data, bytes_read);
    f_close(&fil);

    log_flag = 1;
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
    while (1) {
    }
}

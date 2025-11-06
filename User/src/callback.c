#include "callback.h"

char c, rx_data[64];
extern QueueHandle_t uartQueueHandle;

extern int screen_digital_clock_1_hour_value;
extern int screen_digital_clock_1_min_value;
extern int screen_digital_clock_1_sec_value;

void timestamp_to_rtc(uint32_t timestamp, int8_t timezone_offset, RTC_TimeTypeDef *sTime,
                      RTC_DateTypeDef *sDate) {
    if (sTime == NULL || sDate == NULL) return;

    struct tm timeinfo;
    time_t t_val                   = (time_t)timestamp;
    const time_t *__restrict t_ptr = &t_val;
    gmtime_r(t_ptr, &timeinfo);

    timeinfo.tm_hour += timezone_offset;
    if (timeinfo.tm_hour >= 24) {
        timeinfo.tm_hour -= 24;
        timeinfo.tm_mday += 1;
    } else if (timeinfo.tm_hour < 0) {
        timeinfo.tm_hour += 24;
        timeinfo.tm_mday -= 1;
    }

    sTime->Hours          = timeinfo.tm_hour;
    sTime->Minutes        = timeinfo.tm_min;
    sTime->Seconds        = timeinfo.tm_sec;
    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;

    sDate->WeekDay = (timeinfo.tm_wday == 0) ? 7 : timeinfo.tm_wday;
    sDate->Month   = timeinfo.tm_mon + 1;
    sDate->Date    = timeinfo.tm_mday;

    uint16_t full_year = timeinfo.tm_year + 1900;
    if (full_year >= 2000 && full_year <= 2099) {
        sDate->Year = full_year % 100;
    } else {
        sDate->Year = 0;
    }
}

void execute_command() {
    char cmd[128];
    double value;
    sscanf((const char *)rx_data, "%s %lf\r\n", cmd, &value);
    printf("execute command %s, value: %.2f\r\n", cmd, value);
    if (strcmp("CORRECT_CLOCK", cmd) == 0) {
        RTC_DateTypeDef sDateStructure;
        RTC_TimeTypeDef sTimeStructure;
        timestamp_to_rtc((uint32_t)value, 0, &sTimeStructure, &sDateStructure);
        HAL_RTC_SetTime(&hrtc, &sTimeStructure, RTC_FORMAT_BCD);
        HAL_RTC_SetDate(&hrtc, &sDateStructure, RTC_FORMAT_BCD);
        screen_digital_clock_1_hour_value = sTimeStructure.Hours;
        screen_digital_clock_1_min_value  = sTimeStructure.Minutes;
        screen_digital_clock_1_sec_value  = sTimeStructure.Seconds;
        printf("set time to %d-%d-%d %d:%d:%d\r\n", sDateStructure.Year, sDateStructure.Month,
               sDateStructure.Date, screen_digital_clock_1_hour_value,
               screen_digital_clock_1_min_value, screen_digital_clock_1_sec_value);
    } else if (strcmp("TEST", cmd) == 0) {
        printf("test response\r\n");
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
        execute_command();
        memset(rx_data, 0, sizeof(rx_data));
        receive_byte = 0;
    }
    c = 0;
    HAL_UART_Receive_DMA(&huart1, &c, 1);
}

#include "command.h"

extern int screen_digital_clock_1_hour_value;
extern int screen_digital_clock_1_min_value;
extern int screen_digital_clock_1_sec_value;
extern char rx_data[64];

command_t commands[20];
int command_count = 0;

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

void time_calibration_callback(double value) {
    // Implementation for time calibration callback can be added here
    RTC_DateTypeDef sDateStructure;
    RTC_TimeTypeDef sTimeStructure;
    timestamp_to_rtc((uint32_t)value, 8, &sTimeStructure, &sDateStructure);
    HAL_RTC_SetTime(&hrtc, &sTimeStructure, RTC_FORMAT_BCD);
    HAL_RTC_SetDate(&hrtc, &sDateStructure, RTC_FORMAT_BCD);
    screen_digital_clock_1_hour_value = sTimeStructure.Hours;
    screen_digital_clock_1_min_value  = sTimeStructure.Minutes;
    screen_digital_clock_1_sec_value  = sTimeStructure.Seconds;
    printf("set time to %d-%d-%d %d:%d:%d\r\n", sDateStructure.Year, sDateStructure.Month,
           sDateStructure.Date, screen_digital_clock_1_hour_value, screen_digital_clock_1_min_value,
           screen_digital_clock_1_sec_value);
}

void test_callback(double value) {
    // Implementation for test callback can be added here
    printf("Test command executed with value: %.2f\r\n", value);
}

void execute_command(char *rx_data, double value) {
    char cmd[128];
    sscanf((const char *)rx_data, "%s %lf\r\n", cmd, &value);
    for (int i = 0; i < command_count; i++) {
        if (strcmp(commands[i].cmd, cmd) == 0) {
            if (commands[i].func != NULL) {
                void (*func_ptr)(double) = commands[i].func;
                func_ptr(value);
                printf("execute command %s, value: %.2f\r\n", cmd, value);
            }
            break;
        }
    }
}

void register_command(const char *cmd, void *func) {
    // Implementation for registering a command can be added here
    command_t new_command = {0};
    strncpy(new_command.cmd, cmd, sizeof(new_command.cmd) - 1);
    new_command.func = func;

    // Add the new command to a command list or table
    commands[command_count++] = new_command;
}

extern uint16_t GT911_I2C_DELAY_US;
void touchpad_wait_time_callback(double value) {
    // Implementation for touchpad wait time callback can be added here
    GT911_I2C_DELAY_US = (uint16_t)value;
    printf("Set GT911 I2C delay to %d us\r\n", GT911_I2C_DELAY_US);
}

void command_init() {
    register_command("CORRECT_CLOCK", time_calibration_callback);
    register_command("TEST", test_callback);
    register_command("TOUCH_WAIT_TIME", touchpad_wait_time_callback);
}

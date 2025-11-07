#ifndef COMMAND_H
#define COMMAND_H

#include "main.h"
#include "rtc.h"
#include "time.h"
#include <stdint.h>
#include <string.h>

typedef struct {
    /* data */
    char cmd[128];
    void *func;
} command_t;

void execute_command();
void command_init();

#endif // COMMAND_H

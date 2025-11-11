#ifndef __MY_TASK_H__
#define __MY_TASK_H__

#include "user.h"
#define MAX_RX_LEN 64
typedef struct {
    char data[MAX_RX_LEN];
} UartDisplayMsg;

#endif //__MY_TASK_H__
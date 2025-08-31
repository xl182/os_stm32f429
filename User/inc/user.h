// @file user.h
#ifndef __USER_H__
#define __USER_H__

// stdlib
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

// generated files
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "dma.h"
#include "sdio.h"
#include "fmc.h"
#include "rtc.h"
#include "fatfs.h"
#include "ff.h"
#include "usb_device.h"

// freertos
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "queue.h"

// Gui files
#include "custom.h"
#include "events_init.h"
#include "gui.h"
#include "gui_guider.h"
#include "lv_port_disp.h"
#include "lv_port_fs.h"
#include "lv_port_indev.h"
#include "widgets_init.h"

// custom files
#include "delay.h"

#endif /* __USER_H__ */
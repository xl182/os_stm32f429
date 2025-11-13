#ifndef MANUAL_UI_H
#define MANUAL_UI_H
#include "lvgl.h"

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *status_label;
} manual_ui_t;

void setup_manual_ui(manual_ui_t *ui);
#endif // MANUAL_UI_H
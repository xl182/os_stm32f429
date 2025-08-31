#include "lv_port_indev.h"
#include "gt911.h"

/*********************
 *      DEFINES
 *********************/
#define TOUCHPAD_MIN_X 0
#define TOUCHPAD_MAX_X 1024
#define TOUCHPAD_MIN_Y 0
#define TOUCHPAD_MAX_Y 800

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_init(void);
static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data);

/**********************
 *  STATIC VARIABLES
 **********************/
static gt911_touch_t touch_data;
static bool last_touch_state = false;
static lv_coord_t last_x     = 0;
static lv_coord_t last_y     = 0;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_port_indev_init(void) {
    // 初始化触摸控制器
    touchpad_init();

    // 创建输入设备
    lv_indev_t *indev = lv_indev_create();

    // 设置设备类型和读取回调
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touchpad_read);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void touchpad_init(void) { gt911_init(); }

static void touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
    LV_UNUSED(indev);

    static uint32_t last_read = 0;
    uint32_t now              = lv_tick_get();

    // 每20ms读取一次触摸数据
    if (now - last_read > 33) {
        gt911_scan(&touch_data);

        if (touch_data.valid && touch_data.points > 0) {
            last_touch_state = true;

            // 取第一个触摸点
            last_x = touch_data.x[0];
            last_y = touch_data.y[0];

            // 坐标边界检查
            last_x = LV_CLAMP(TOUCHPAD_MIN_X, last_x, TOUCHPAD_MAX_X);
            last_y = LV_CLAMP(TOUCHPAD_MIN_Y, last_y, TOUCHPAD_MAX_Y);
        } else {
            last_touch_state = false;
        }

        last_read = now;
    }

    data->state   = last_touch_state ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    data->point.x = last_x;
    data->point.y = last_y;
}

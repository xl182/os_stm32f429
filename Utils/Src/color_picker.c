#include "color_picker.h"
#if USE_LVGL
#include "lvgl/lvgl.h"


static void update_color_cb(lv_event_t *e);
static void close_color_picker(lv_event_t *e);
static void draw_color_palette();

static color_picker_t color_picker_ctx;
uint8_t initial_brush_width = 5;

static void update_brush_width_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    uint8_t width = (uint8_t)lv_slider_get_value(slider);
    color_picker_ctx.brush_width = width;
    lv_label_set_text_fmt(color_picker_ctx.brush_label, "%dpx", width);
}


lv_obj_t *create_color_picker(lv_obj_t *parent, lv_color_t initial_color) {
    lv_obj_t *window = lv_win_create(parent);
    lv_obj_set_size(window, 300, 500);
    lv_obj_align(window, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(window, LV_OPA_90, 0);

    lv_obj_t *header = lv_win_add_title(window, "Color Picker");

    color_picker_ctx.preview = lv_obj_create(lv_win_get_content(window));
    lv_obj_set_size(color_picker_ctx.preview, 80, 80);
    lv_obj_align(color_picker_ctx.preview, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(color_picker_ctx.preview, initial_color, 0);
    lv_obj_set_style_radius(color_picker_ctx.preview, 5, 0);

    color_picker_ctx.palette_area = lv_obj_create(lv_win_get_content(window));
    lv_obj_set_size(color_picker_ctx.palette_area, 150, 150);
    lv_obj_align(color_picker_ctx.palette_area, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_pad_all(color_picker_ctx.palette_area, 0, 0);
    lv_obj_set_style_border_width(color_picker_ctx.palette_area, 0, 0);

    color_picker_ctx.selected_color = initial_color;
    color_picker_ctx.window         = window;

    lv_obj_t *hue_label = lv_label_create(lv_win_get_content(window));
    lv_label_set_text(hue_label, "Hue");
    lv_obj_align(hue_label, LV_ALIGN_LEFT_MID, 10, -10);
    color_picker_ctx.hue_bar = lv_slider_create(lv_win_get_content(window));
    lv_slider_set_range(color_picker_ctx.hue_bar, 0, 360);
    lv_slider_set_value(color_picker_ctx.hue_bar, lv_color_to_hsv(initial_color).h, LV_ANIM_OFF);
    lv_obj_set_size(color_picker_ctx.hue_bar, 150, 20);
    lv_obj_align_to(color_picker_ctx.hue_bar, hue_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    lv_obj_add_event_cb(color_picker_ctx.hue_bar, update_color_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *brightness_label = lv_label_create(lv_win_get_content(window));
    lv_label_set_text(brightness_label, "Brightness");
    lv_obj_align_to(brightness_label, color_picker_ctx.hue_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 15);
    color_picker_ctx.brightness_bar = lv_slider_create(lv_win_get_content(window));
    lv_slider_set_range(color_picker_ctx.brightness_bar, 0, 100);
    lv_slider_set_value(color_picker_ctx.brightness_bar, lv_color_to_hsv(initial_color).v,
                        LV_ANIM_OFF);
    lv_obj_set_size(color_picker_ctx.brightness_bar, 150, 20);
    lv_obj_align_to(color_picker_ctx.brightness_bar, brightness_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
                    5);
    lv_obj_add_event_cb(color_picker_ctx.brightness_bar, update_color_cb, LV_EVENT_VALUE_CHANGED,
                        NULL);

    lv_obj_t *ok_btn = lv_btn_create(lv_win_get_content(window));
    lv_obj_set_size(ok_btn, 80, 30);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t *ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "OK");
    lv_obj_add_event_cb(ok_btn, close_color_picker, LV_EVENT_CLICKED, window);

    lv_obj_t *cancel_btn = lv_btn_create(lv_win_get_content(window));
    lv_obj_set_size(cancel_btn, 80, 30);
    lv_obj_align_to(cancel_btn, ok_btn, LV_ALIGN_OUT_LEFT_MID, -10, 0);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_add_event_cb(cancel_btn, close_color_picker, LV_EVENT_CLICKED, window);

    // 笔刷宽度标签
    lv_obj_t *brush_label = lv_label_create(lv_win_get_content(window));
    lv_label_set_text(brush_label, "Brush Width");
    lv_obj_align_to(brush_label, brightness_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 40);

    // 笔刷宽度滑块（1-20像素）
    color_picker_ctx.brush_slider = lv_slider_create(lv_win_get_content(window));
    lv_slider_set_range(color_picker_ctx.brush_slider, 1, 10);
    lv_slider_set_value(color_picker_ctx.brush_slider, initial_brush_width, LV_ANIM_OFF);
    lv_obj_set_size(color_picker_ctx.brush_slider, 150, 20);
    lv_obj_align_to(color_picker_ctx.brush_slider, brush_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);

    // 宽度值显示标签
    color_picker_ctx.brush_label = lv_label_create(lv_win_get_content(window));
    lv_label_set_text_fmt(color_picker_ctx.brush_label, "%dpx", initial_brush_width);
    lv_obj_align_to(color_picker_ctx.brush_label, color_picker_ctx.brush_slider,
                    LV_ALIGN_OUT_RIGHT_MID, 10, 0);

    // 绑定滑块事件
    lv_obj_add_event_cb(color_picker_ctx.brush_slider, update_brush_width_cb,
                        LV_EVENT_VALUE_CHANGED, NULL);

    draw_color_palette();
    return window;
}

void color_picker_event(lv_event_t *e) {
    lv_obj_t *btn                   = lv_event_get_target(e);
    color_picker_ctx.selected_color = lv_obj_get_style_bg_color(btn, 0);
    lv_obj_set_style_bg_color(color_picker_ctx.preview, color_picker_ctx.selected_color, 0);
    draw_color_palette();
}

static void draw_color_palette() {
    lv_obj_clean(color_picker_ctx.palette_area);

    lv_color_t palette_colors[] = {
        lv_color_hex(0xFF0000), lv_color_hex(0xFFA500), lv_color_hex(0xFFFF00),
        lv_color_hex(0x7FFF00), lv_color_hex(0x00FF00), lv_color_hex(0x00FF7F),
        lv_color_hex(0x00FFFF), lv_color_hex(0x0000FF), lv_color_hex(0x800080),
        lv_color_hex(0xFF00FF), lv_color_hex(0xFFC0CB), lv_color_hex(0x000000),
        lv_color_hex(0x808080), lv_color_hex(0xC0C0C0), lv_color_hex(0xFFFFFF),
        lv_color_hex(0xA52A2A)};
    const char *color_names[] = {"Red",  "Orange", "Yellow", "Chartreuse", "Green", "Spring Green",
                                 "Cyan", "Blue",   "Purple", "Magenta",    "Pink",  "Black",
                                 "Gray", "Silver", "White",  "Brown"};
    const int cols            = 4;
    const int rows            = 4;
    const int button_size     = 30;
    const int spacing         = 5;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            if (idx >= sizeof(palette_colors) / sizeof(lv_color_t)) break;
            lv_obj_t *btn = lv_btn_create(color_picker_ctx.palette_area);
            lv_obj_set_size(btn, button_size, button_size);
            lv_obj_set_style_bg_color(btn, palette_colors[idx], 0);
            lv_obj_set_style_radius(btn, 3, 0);
            lv_obj_set_pos(btn, j * (button_size + spacing), i * (button_size + spacing));

            if (palette_colors[idx].blue == color_picker_ctx.selected_color.blue &&
                palette_colors[idx].green == color_picker_ctx.selected_color.green &&
                palette_colors[idx].red == color_picker_ctx.selected_color.red) {
                lv_obj_set_style_border_width(btn, 2, 0);
                lv_obj_set_style_border_color(btn, lv_color_hex(0x000000), 0);
            }
            lv_obj_add_event_cb(btn, color_picker_event, LV_EVENT_CLICKED, NULL);
        }
    }
}

static void update_color_cb(lv_event_t *e) {
    uint16_t hue        = lv_slider_get_value(color_picker_ctx.hue_bar);
    uint16_t brightness = lv_slider_get_value(color_picker_ctx.brightness_bar);

    lv_color_hsv_t hsv              = {hue, 100, brightness};
    color_picker_ctx.selected_color = lv_color_hsv_to_rgb(hsv.h, hsv.s, hsv.v);

    lv_obj_set_style_bg_color(color_picker_ctx.preview, color_picker_ctx.selected_color, 0);
}

static void close_color_picker(lv_event_t *e) {
    lv_obj_t *btn    = lv_event_get_target(e);
    const char *text = lv_label_get_text(lv_obj_get_child(btn, 0));

    lv_obj_del_async(color_picker_ctx.window);

    if (strcmp(text, "OK") == 0) {
        lv_obj_send_event(color_picker_ctx.window, LV_EVENT_READY,
                          &color_picker_ctx);
    }

    color_picker_ctx.window = NULL;
}

lv_color_t color_picker_get_selected() { return color_picker_ctx.selected_color; }

void show_color_picker(lv_obj_t *parent, lv_color_t initial_color, lv_event_cb_t callback) {
    if (color_picker_ctx.window) {
        lv_obj_del_async(color_picker_ctx.window);
    }

    lv_obj_t *picker = create_color_picker(parent, initial_color);

    if (callback) {
        lv_obj_add_event_cb(picker, callback, LV_EVENT_READY, NULL);
    }
}
#endif

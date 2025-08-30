#include "paint_board.h"
#if USE_LVGL
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    lv_point_t last_point;
    bool drawing;
    lv_color_t pen_color;
    uint8_t pen_size;
    lv_area_t refresh_area;
    lv_point_t *brush_mask;    // 笔刷掩模数组
    uint16_t brush_mask_count; // 掩模中的点数
    bool mask_dirty;
} paint_ctx_t;

static paint_ctx_t paint_ctx;
static lv_obj_t *canvas = NULL;
void paint_set_pen_width(uint8_t width);

// 生成圆形笔刷掩模
static void generate_brush_mask() {
    // 释放旧掩模
    if (paint_ctx.brush_mask) {
        lv_free(paint_ctx.brush_mask);
        paint_ctx.brush_mask = NULL;
    }

    const uint8_t r  = paint_ctx.pen_size / 2;
    const int16_t r2 = r * r;

    // 计算圆形笔刷中的点数
    paint_ctx.brush_mask_count = 0;
    for (int16_t dy = -r; dy <= r; dy++) {
        for (int16_t dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r2) paint_ctx.brush_mask_count++;
        }
    }

    // 分配并填充掩模数组
    paint_ctx.brush_mask = lv_malloc(sizeof(lv_point_t) * paint_ctx.brush_mask_count);
    uint16_t idx         = 0;
    for (int16_t dy = -r; dy <= r; dy++) {
        for (int16_t dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r2) {
                paint_ctx.brush_mask[idx].x = dx;
                paint_ctx.brush_mask[idx].y = dy;
                idx++;
            }
        }
    }
}

// 使用掩模绘制笔刷
static inline void draw_brush(uint16_t *buf, int16_t cx, int16_t cy, lv_color_t color) {
    const int16_t width  = 900;
    const int16_t height = 600;

    // 计算有效区域边界
    const int16_t min_x = MAX(cx + paint_ctx.brush_mask[0].x, 0);
    const int16_t max_x =
        MIN(cx + paint_ctx.brush_mask[paint_ctx.brush_mask_count - 1].x, width - 1);
    const int16_t min_y = MAX(cy + paint_ctx.brush_mask[0].y, 0);
    const int16_t max_y =
        MIN(cy + paint_ctx.brush_mask[paint_ctx.brush_mask_count - 1].y, height - 1);

    // 如果笔刷完全在画布内
    if (min_x >= 0 && max_x < width && min_y >= 0 && max_y < height) {
        const uint16_t color16 = lv_color_to_u16(color);
        for (uint16_t i = 0; i < paint_ctx.brush_mask_count; i++) {
            const int16_t x    = cx + paint_ctx.brush_mask[i].x;
            const int16_t y    = cy + paint_ctx.brush_mask[i].y;
            buf[y * width + x] = color16;
        }
    }
    // 部分在画布外的情况
    else {
        for (uint16_t i = 0; i < paint_ctx.brush_mask_count; i++) {
            const int16_t x = cx + paint_ctx.brush_mask[i].x;
            const int16_t y = cy + paint_ctx.brush_mask[i].y;
            if (x >= 0 && x < width && y >= 0 && y < height) {
                buf[y * width + x] = lv_color_to_u16(color);
            }
        }
    }
}

// 优化的画线函数
static void draw_line(uint16_t *buf, lv_point_t p1, lv_point_t p2) {
    const int16_t dx = LV_ABS(p2.x - p1.x);
    const int16_t dy = LV_ABS(p2.y - p1.y);
    const int16_t sx = (p1.x < p2.x) ? 1 : -1;
    const int16_t sy = (p1.y < p2.y) ? 1 : -1;
    int16_t err      = dx - dy;

    while (1) {
        draw_brush(buf, p1.x, p1.y, paint_ctx.pen_color);
        if (p1.x == p2.x && p1.y == p2.y) break;

        int16_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            p1.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            p1.y += sy;
        }
    }
}

static void paint_event_handler(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj        = lv_event_get_target(e);
    if (obj != canvas) return;
    lv_indev_t *indev = lv_indev_get_act();
    if (!indev) return;

    lv_area_t canvas_coords;
    lv_obj_get_coords(obj, &canvas_coords);
    lv_point_t screen_point;
    lv_indev_get_point(indev, &screen_point);

    lv_point_t canvas_point;
    canvas_point.x = screen_point.x - canvas_coords.x1;
    canvas_point.y = screen_point.y - canvas_coords.y1;
    canvas_point.x = LV_CLAMP(0, canvas_point.x, 899);
    canvas_point.y = LV_CLAMP(0, canvas_point.y, 599);

    if (code == LV_EVENT_PRESSED) {
        paint_ctx.last_point = canvas_point;
        paint_ctx.drawing    = true;
        uint16_t *buf        = (uint16_t *)lv_canvas_get_buf(canvas);

        // 使用笔刷绘制
        draw_brush(buf, canvas_point.x, canvas_point.y, paint_ctx.pen_color);

        // 设置初始刷新区域
        const int16_t r           = paint_ctx.pen_size / 2;
        paint_ctx.refresh_area.x1 = canvas_point.x - r - 1;
        paint_ctx.refresh_area.y1 = canvas_point.y - r - 1;
        paint_ctx.refresh_area.x2 = canvas_point.x + r + 1;
        paint_ctx.refresh_area.y2 = canvas_point.y + r + 1;
    } else if (code == LV_EVENT_RELEASED) {
        if (paint_ctx.drawing) {
            lv_obj_invalidate_area(canvas, &paint_ctx.refresh_area);
            paint_ctx.drawing = false;
        }
    } else if (code == LV_EVENT_PRESSING && paint_ctx.drawing) {
        if (paint_ctx.mask_dirty) {
            generate_brush_mask();
            paint_ctx.mask_dirty = false;
        }
        // 只在点之间有足够距离时才绘制
        if (LV_ABS(canvas_point.x - paint_ctx.last_point.x) > 1 ||
            LV_ABS(canvas_point.y - paint_ctx.last_point.y) > 1) {

            uint16_t *buf = (uint16_t *)lv_canvas_get_buf(canvas);

            // 绘制从上一个点到当前点的线
            draw_line(buf, paint_ctx.last_point, canvas_point);

            // 更新刷新区域
            const int16_t r    = paint_ctx.pen_size / 2;
            lv_area_t new_area = {.x1 = LV_MIN(paint_ctx.last_point.x, canvas_point.x) - r - 1,
                                  .y1 = LV_MIN(paint_ctx.last_point.y, canvas_point.y) - r - 1,
                                  .x2 = LV_MAX(paint_ctx.last_point.x, canvas_point.x) + r + 1,
                                  .y2 = LV_MAX(paint_ctx.last_point.y, canvas_point.y) + r + 1};

            // 合并到总刷新区域
            _lv_area_join(&paint_ctx.refresh_area, &paint_ctx.refresh_area, &new_area);

            paint_ctx.last_point = canvas_point;
        }

        // 立即刷新
        lv_area_t clipped_area = {.x1 = LV_CLAMP(0, paint_ctx.refresh_area.x1, 899),
                                  .y1 = LV_CLAMP(0, paint_ctx.refresh_area.y1, 599),
                                  .x2 = LV_CLAMP(0, paint_ctx.refresh_area.x2, 899),
                                  .y2 = LV_CLAMP(0, paint_ctx.refresh_area.y2, 599)};

        lv_obj_invalidate_area(canvas, &clipped_area);

        // 重置刷新区域为当前点周围
        const int16_t r           = paint_ctx.pen_size / 2;
        paint_ctx.refresh_area.x1 = canvas_point.x - r - 1;
        paint_ctx.refresh_area.y1 = canvas_point.y - r - 1;
        paint_ctx.refresh_area.x2 = canvas_point.x + r + 1;
        paint_ctx.refresh_area.y2 = canvas_point.y + r + 1;
    }
}

extern lv_ui *ui;
static void color_selected_cb(lv_event_t *e) {
    color_picker_t *ctx = (color_picker_t *)lv_event_get_param(e);
    paint_set_pen_color(ctx->selected_color);
    paint_set_pen_width(ctx->brush_width); // 触发更新

    lv_obj_t *color_btn = ui->screen_paint_btn_choose_color;
    lv_obj_set_style_bg_color(color_btn, ctx->selected_color, 0);

    // 强制刷新画布
    if (canvas) lv_obj_invalidate(canvas);
}

static void color_button_cb(lv_event_t *e) {
    lv_color_t current_color = color_picker_get_selected();
    show_color_picker(lv_layer_top(), current_color, color_selected_cb);
}

LCD_FRAMEBUF uint16_t draw_area_buffer[900 * 600];

void custom_screen_paint_init(lv_ui *ui) {
    paint_ctx.pen_color  = lv_color_hex(0x0000FF);
    paint_ctx.pen_size   = 3;
    paint_ctx.drawing    = false;
    paint_ctx.brush_mask = NULL;

    canvas = ui->screen_paint_paint_area = lv_canvas_create(ui->screen_paint);
    lv_obj_set_size(canvas, 900, 600);
    lv_obj_set_pos(canvas, 0, 0);
    lv_canvas_set_buffer(canvas, draw_area_buffer, 900, 600, LV_COLOR_FORMAT_RGB565);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(canvas, LV_DIR_NONE);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(canvas, paint_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_opa(canvas, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(canvas, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_opa(canvas, LV_OPA_TRANSP, 0);
    lv_obj_set_style_outline_opa(canvas, LV_OPA_TRANSP, 0);
    lv_obj_set_style_anim_time(canvas, 0, 0);
    lv_obj_set_style_radius(canvas, 0, 0);
    lv_obj_set_style_clip_corner(canvas, false, 0);
    lv_obj_set_style_img_recolor_opa(canvas, LV_OPA_TRANSP, 0);

    lv_obj_add_event_cb(ui->screen_paint_btn_choose_color, color_button_cb, LV_EVENT_CLICKED, NULL);
    // 生成初始笔刷掩模
    generate_brush_mask();
}

void paint_set_pen_color(lv_color_t color) { paint_ctx.pen_color = color; }
void paint_set_pen_width(uint8_t width) {
    if (width != paint_ctx.pen_size) {
        paint_ctx.pen_size   = LV_CLAMP(1, width, 10);
        paint_ctx.mask_dirty = true; // 标记需要更新掩模
    }
}

void paint_set_pen_size(uint8_t size) {
    paint_ctx.pen_size = LV_CLAMP(1, size, 10);
    generate_brush_mask(); // 更新笔刷掩模
}

void paint_clear(lv_color_t color) {
    uint32_t size    = 900 * 600;
    uint16_t *buf    = (uint16_t *)lv_canvas_get_buf(canvas);
    uint16_t color16 = lv_color_to_u16(color);

    // 使用内存设置提高效率
    for (uint32_t i = 0; i < size; i++) {
        buf[i] = color16;
    }

    lv_obj_invalidate(canvas);
}
#endif

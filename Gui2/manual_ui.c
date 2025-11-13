#include "manual_ui.h"
#include "lv_port_disp.h"

// 初始化disp2的UI
void setup_manual_ui(manual_ui_t *ui) {
    if (disp_drv_spi == NULL) {
        printf("Error: disp_drv_spi is NULL! SPI is not initialized\r\n");
        return;  // 避免后续错误
    }
    // 1. 临时将默认显示设备切换为disp2
    printf("setup_manual_ui\n");
    lv_display_t *prev_default = lv_display_get_default(); // 保存之前的默认设备
    lv_display_set_default(disp_drv_spi);

    // 2. 创建屏幕（此时默认设备是disp2，屏幕会关联到disp2）
    ui->screen = lv_obj_create(NULL);

    // 3. 创建UI元素（父对象为ui->screen）
    ui->status_label = lv_label_create(ui->screen);
    lv_label_set_text(ui->status_label, "Manual UI (disp2)");
    lv_obj_set_pos(ui->status_label, 20, 20);

    // 4. 加载屏幕到当前默认显示设备（disp2）
    lv_screen_load(ui->screen);
    printf("Screen loaded on disp2\n");
    // 5. 恢复默认显示设备为之前的（避免影响其他UI）

    lv_display_set_default(prev_default);
}
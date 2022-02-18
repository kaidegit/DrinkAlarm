//
// Created by yekai on 2022/2/18.
//

#include <esp32-hal-log.h>
#include <esp_task_wdt.h>
#include <esp32-hal.h>
#include "lvgl_port.h"
#include "lvgl.h"
#include "oled.h"

#define OLED_Width 128
#define OLED_Height 64

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[OLED_Width * 10];
uint8_t oled_full_buffer[8 * 128];

//#define BIT_SET(a,b) ((a) |= (1U<<(b)))
//#define BIT_CLEAR(a,b) ((a) &= ~(1U<<(b)))
//
//static void ex_disp_rounder(lv_disp_drv_t * disp_drv, lv_area_t *area)
//{
//    uint8_t hor_max = disp_drv->hor_res;
//    uint8_t ver_max = disp_drv->ver_res;
//
//    area->x1 = 0;
//    area->y1 = 0;
//    area->x2 = hor_max - 1;
//    area->y2 = ver_max - 1;
//}
//
//static void ex_disp_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
//                           lv_color_t color, lv_opa_t opa)
//{
//    uint16_t byte_index = x + (( y>>3 ) * buf_w);
//    uint8_t  bit_index  = y & 0x7;
//
//    if ((color.full == 0) && (LV_OPA_TRANSP != opa)) {
//        BIT_SET(buf[byte_index], bit_index);
//    } else {
//        BIT_CLEAR(buf[byte_index], bit_index);
//    }
//}
//
///*Write the internal buffer (VDB) to the display. 'lv_flush_ready()' has to be called when finished*/
//static void ex_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
//{
//    /* For SSD1306 Divide by 8 根据LV_PORT_ESP32 更改 变成8的倍数*/
//    // printf("x1:%d, y1:%d x2:%d y2:%d\n", area->x1, area->y1, area->x2, area->y2);
//    uint8_t row1 = area->y1 >> 3;
//    uint8_t row2 = (area->y2 >> 3) + 1;    // 加1是因为63/8=7 7*8=56这样会少绘制一段
//    row1 = row1 << 3;
//    row2 = (row2 << 3) - 1;     // 不减1的话后面会因为不是8的倍数报错，这里也不确定到底对不对，反正目前能用
//    // printf("row1:%d, row2:%d\n", row1, row2);
//    lcd_obj.draw_bitmap(area->x1, row1, (uint16_t)(area->x2 - area->x1 + 1), (uint16_t)(row2 - row1 + 1), (uint16_t *)color_map);
//
//    /* IMPORTANT!!!
//     * Inform the graphics library that you are ready with the flushing*/
//    lv_disp_flush_ready(drv);
//}

// LVGL出的图是一个8位的灰度图，转换为full-buffer中的单色，再刷全屏
#define GRAY_SINGLE_THRESHOLD 10

void oled_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    for (auto i = area->x1; i <= area->x2; i++) {
        for (auto j = area->y1; j <= area->y2; j++) {
            (color_p->full < GRAY_SINGLE_THRESHOLD) ?
                    oled_full_buffer[i + (j / 8) * 128] &= ~(1 << (j % 8)) :
                    oled_full_buffer[i + (j / 8) * 128] |= 1 << (j % 8);
            color_p++;
        }
    }
    OLED_FullFlash((const char *) oled_full_buffer);

    lv_disp_flush_ready(disp);
}

void GUI_Init() {
    lv_init(); // 初始化LVGL

    lv_log_register_print_cb((lv_log_print_g_cb_t) log_printf); // 注册log输出组件

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, OLED_Width * 10);

    // 注册屏幕组件
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = OLED_Width;
    disp_drv.ver_res = OLED_Height;
    disp_drv.flush_cb = oled_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void GUI_Run(void *parm) {
    while (true) {
        lv_tick_inc(10);
        lv_task_handler();
        esp_task_wdt_reset();
        delay(10);
    }
}

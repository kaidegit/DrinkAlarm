//
// Created by yekai on 2022/2/18.
//

#ifndef MAIN_LVGL_PORT_H
#define MAIN_LVGL_PORT_H

void GUI_Init();

void GUI_Run(void *parm);

extern uint8_t oled_full_buffer[8 * 128];

#endif //MAIN_LVGL_PORT_H

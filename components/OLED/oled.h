//
// Created by yekai on 2022/2/18.
//

#ifndef MAIN_OLED_H
#define MAIN_OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void OLED_ShowWaitToSet(uint8_t time, bool shouldPlace);

void OLED_ShowTimeSet(uint8_t time);

void OLED_WaitTimeSync();

void OLED_WaitWifiConn();

void OLED_Time_Task(void *param);

#ifdef __cplusplus
}
#endif

#endif //MAIN_OLED_H

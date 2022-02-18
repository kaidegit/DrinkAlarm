//
// Created by yekai on 2022/2/18.
//

// should be opened with utf-8

#include "oled.h"
#include "u8g2_port.h"
#include "time_sync.h"
#include <cstdio>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../alarm/alarm.h"

void OLED_ShowWaitToSet(uint8_t time, bool shouldPlace) {
    char ch[50];
    u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312a);
    u8g2_ClearBuffer(&u8g2);
    sprintf(ch, "请在%d秒内", time);
    u8g2_DrawUTF8(&u8g2, 6, 20, ch);
    sprintf(ch, "移%s杯子", shouldPlace ? "回" : "开");
    u8g2_DrawUTF8(&u8g2, 6, 40, ch);
    sprintf(ch, "以进入设置");
    u8g2_DrawUTF8(&u8g2, 6, 60, ch);
    u8g2_SendBuffer(&u8g2);
}

void OLED_ShowTimeSet(uint8_t time) {
    char ch[50];
    u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312a);
    u8g2_ClearBuffer(&u8g2);
    sprintf(ch, "可使用ESPTouch配网");
    u8g2_DrawUTF8(&u8g2, 6, 12, ch);
    sprintf(ch, "通知时间设定:");
    u8g2_DrawUTF8(&u8g2, 6, 24, ch);
    sprintf(ch, "%dmin", time);
    u8g2_DrawUTF8(&u8g2, 6, 36, ch);
    sprintf(ch, "移回20秒以确定");
    u8g2_DrawUTF8(&u8g2, 6, 48, ch);
    u8g2_SendBuffer(&u8g2);
}

void OLED_WaitTimeSync() {
    char ch[50];
    u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312a);
    u8g2_ClearBuffer(&u8g2);
    sprintf(ch, "等待时间同步");
    u8g2_DrawUTF8(&u8g2, 6, 36, ch);
    u8g2_SendBuffer(&u8g2);
}

void OLED_WaitWifiConn() {
    char ch[50];
    u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312a);
    u8g2_ClearBuffer(&u8g2);
    sprintf(ch, "等待WiFi连接");
    u8g2_DrawUTF8(&u8g2, 6, 36, ch);
    u8g2_SendBuffer(&u8g2);
}

void OLED_Time_Task(void *param) {
    while (true) {
        uint8_t hr, min, sec;
        uint16_t time;
        char ch[50];
        u8g2_ClearBuffer(&u8g2);
        time = get_time(&hr, &min, &sec);
        if (timeSyncState == TIME_SYNC_SUCCESS) {
            u8g2_SetFont(&u8g2, u8g2_font_logisoso24_tn);
            sprintf(ch, "%02d:%02d:%02d", hr, min, sec);
            u8g2_DrawUTF8(&u8g2, 12, 24, ch);
        }
        // todo:进度条
        extern bool isAlarmOn;
        extern uint16_t alarmStartTime;
        if (isAlarmOn) {
            auto boxlength = (time - alarmStartTime) * 104 / alarm_time / 60;
            if (boxlength > 104){
                boxlength = 104;
            }
            u8g2_DrawFrame(&u8g2, 12, 42, 104, 12);
            u8g2_DrawBox(&u8g2, 12, 42, boxlength, 12);
        }
        u8g2_SendBuffer(&u8g2);

        vTaskDelay(250 / portTICK_RATE_MS);
    }
}
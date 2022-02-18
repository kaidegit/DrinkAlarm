//
// Created by yekai on 2022/2/12.
//

#include "arduino_main.h"
#include <Wire.h>
#include <nvs_flash.h>
#include "esp_task_wdt.h"
#include "wifi_smartconfig.h"
#include "alarm.h"
#include "time_sync.h"
#include "peripheral_init.h"
#include "oled.h"
#include "lvgl_port.h"
#include "gui_guider.h"
#include "oledfont.h"
#include "u8g2.h"
#include "u8g2_port.h"

//lv_ui guider_ui;


void setup() {
    ESP_ERROR_CHECK(nvs_flash_init());
    pinMode(LED_Pin, OUTPUT);
    pinMode(SENSOR_Pin, INPUT);
    initialise_wifi();
//    OLED_Init();
//    char ch[] = "helloworld";
//    OLED_Clear();
//    OLED_FullFlash((char *) test);
//    OLED_ShowString(0, 0, (uint8_t *) ch, 16);
//    OLED_ShowString(0, 2, (uint8_t *) ch, 16);
//    OLED_ShowString(0, 4, (uint8_t *) ch, 16);
//    OLED_ShowString(0, 6, (uint8_t *) ch, 16);
//    GUI_Init();
//    setup_ui(&guider_ui);
//    xTaskCreate(
//            GUI_Run,
//            "GUI_Run",
//            4096 * 4,
//            NULL,
//            3,
//            NULL
//            );
    U8G2_Init();

    // 启动检查手势来进入设置，设置完后重启
    settings_action_catch();
    // 尝试使用flash中信息连接WiFi，读不到信息则进入smartconfig
    if (connect_wifi_from_flash() != ESP_OK) {
        start_smartconfig();
    }
    //读flash中的闹钟时长
    read_alarm_from_flash();
    // 同步时间
    sync_time_sntp();
//    for (auto i :oled_full_buffer){
//        printf("%x ",i);
//    }
    u8g2_SetFont(&u8g2, u8g2_font_helvB08_tr);
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawStr(&u8g2, 10, 30, "Hello, world!");
    u8g2_DrawStr(&u8g2, 10, 100, "Hello, larger world!");
    u8g2_SendBuffer(&u8g2);
}

void loop() {
    static int state = LOW;
    if (state != digitalRead(SENSOR_Pin)) {
        delay(10);
        if (state != digitalRead(SENSOR_Pin)) {
            state = digitalRead(SENSOR_Pin);
            if (state == HIGH) {
                digitalWrite(LED_Pin, HIGH);
                ESP_LOGI("sensor", "sensor get high");
            } else {
                digitalWrite(LED_Pin, LOW);
                ESP_LOGI("sensor", "sensor get low");
            }
        }
    }
    delay(10);
}

// 上电后杯子存在，5秒内杯子移开，移开后5秒内杯子放回，则进入
void settings_action_catch() {
    // 杯子存在检测
    if (IS_CUP_ON()) {
        delay(10);
        if (IS_CUP_ON()) {
            // 5秒内杯子移开检测
            uint16_t count = 0;
            while (count++ < 500) {
                esp_task_wdt_reset();
                delay(10);
                if (IS_CUP_OFF()) {
                    delay(10);
                    if (IS_CUP_OFF()) {
                        // 5秒内杯子放回检测
                        count = 0;
                        while (count++ < 500) {
                            esp_task_wdt_reset();
                            delay(10);
                            if (IS_CUP_ON()) {
                                delay(10);
                                if (IS_CUP_ON()) {
                                    // 进入设置
                                    ESP_LOGI("settings", "entry settings");
                                    start_smartconfig();
                                    set_alarm_time();
                                    count = 500;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//
// Created by yekai on 2022/2/12.
//

#include "arduino_main.h"
#include <Wire.h>
#include <nvs_flash.h>
#include "U8g2lib.h"
#include "esp_task_wdt.h"
#include "wifi_smartconfig.h"
#include "alarm.h"

//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
//        U8G2_R0,
//        /* reset=*/ U8X8_PIN_NONE,
//        /* clock=*/ OLED_SCL_Pin,
//        /* data=*/ OLED_SDA_Pin
//);

void setup() {
    ESP_ERROR_CHECK(nvs_flash_init());
    pinMode(LED_Pin, OUTPUT);
    pinMode(SENSOR_Pin, INPUT);
    initialise_wifi();
//    u8g2.begin();
    // 启动检查手势来进入设置，设置完后重启
    settings_action_catch();
    // 检查WiFi状态（是否进入smart config），选择是否读取flash来连接WiFi
    connect_wifi_from_flash();
    //读flash中的闹钟时长
    read_alarm_from_flash();
    ESP_LOGI("alarm", "get alarm time", alarm_time);

    connect_wifi_from_flash();
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
//    u8g2.clearBuffer();					// clear the internal memory
//    u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
//    u8g2.drawStr(0,10,"Hello World!");	// write something to the internal memory
//    u8g2.sendBuffer();					// transfer internal memory to the display
//    delay(1000);
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

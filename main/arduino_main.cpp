//
// Created by yekai on 2022/2/12.
//

#include "arduino_main.h"
#include <nvs_flash.h>
#include <esp_log.h>
#include "esp_task_wdt.h"
#include "wifi_smartconfig.h"
#include "alarm.h"
#include "time_sync.h"
#include "peripheral_init.h"
#include "u8g2.h"
#include "u8g2_port.h"
#include "oled.h"

bool isAlarmOn = false;
bool isAlarm = false;
uint16_t alarmStartTime;

#define delay(_ms) vTaskDelay((_ms) / portTICK_RATE_MS);

void setup() {
    ESP_ERROR_CHECK(nvs_flash_init());
    LED_GPIO_Init();
    SENSOR_GPIO_Init();
    initialise_wifi();
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

    xTaskCreate(
            OLED_Time_Task,
            "oled_time_task",
            4096,
            nullptr,
            3,
            nullptr
    );
}

void loop() {
    static int state = 1;
    if (state != gpio_get_level((gpio_num_t)SENSOR_Pin)) {
        delay(10);
        if (state != gpio_get_level((gpio_num_t)SENSOR_Pin)) {
            state = gpio_get_level((gpio_num_t)SENSOR_Pin);
            if (IS_CUP_ON()) {
                // should set alarm
                ESP_LOGI("sensor", "sensor get low");
                isAlarmOn = true;
                alarmStartTime = get_time(nullptr,nullptr,nullptr);
            } else {
                // should reset alarm
                ESP_LOGI("sensor", "sensor get high");
                isAlarmOn = false;
                isAlarm = false;
            }
        }
    }
    delay(10);
}

// 上电后杯子存在，5秒内杯子移开，移开后5秒内杯子放回，则进入
#define WAIT_TIME_MS 5000 // 5s
#define WAIT_COUNT (WAIT_TIME_MS/10)
#define COUNT2TIME(_c) ((_c)/100)

void settings_action_catch() {
    // 杯子存在检测
    if (IS_CUP_ON()) {
        delay(10);
        if (IS_CUP_ON()) {
            // 5秒内杯子移开检测
            uint16_t count = 0;
            while (count++ < WAIT_COUNT) {
                if (count % 100 == 0) {
                    OLED_ShowWaitToSet(COUNT2TIME(WAIT_COUNT - count) + 1, false);
                }
                esp_task_wdt_reset();
                delay(10);
                if (IS_CUP_OFF()) {
                    delay(10);
                    if (IS_CUP_OFF()) {
                        // 5秒内杯子放回检测
                        count = 0;
                        while (count++ < WAIT_COUNT) {
                            if (count % 100 == 0) {
                                OLED_ShowWaitToSet(COUNT2TIME(WAIT_COUNT - count) + 1, true);
                            }
                            esp_task_wdt_reset();
                            delay(10);
                            if (IS_CUP_ON()) {
                                delay(10);
                                if (IS_CUP_ON()) {
                                    // 进入设置
                                    ESP_LOGI("settings", "entry settings");
                                    start_smartconfig();
                                    set_alarm_time();
                                    count = WAIT_COUNT;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//
// Created by yekai on 2022/2/15.
//

#include <nvs.h>
#include <esp_task_wdt.h>
#include "alarm.h"
#include "alarm_url.h"

#ifndef ALARM_URL
    #error Please set alarm url string with define
#endif
uint8_t alarm_time;

static char *TAG = "alarm";

static esp_err_t save_alarm_to_flash() {
    nvs_handle_t save_alarm_nvs_handle;
    if (nvs_open("nvs", NVS_READWRITE, &save_alarm_nvs_handle) != ESP_OK) {
        ESP_LOGE(TAG, "open nvs fail");
        nvs_close(save_alarm_nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "open nvs success,saving.....");
    if (nvs_set_u8(save_alarm_nvs_handle, "alarm", alarm_time) != ESP_OK) {
        ESP_LOGE(TAG, "nvs save alarm fail");
        nvs_close(save_alarm_nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    nvs_close(save_alarm_nvs_handle);
    return ESP_OK;
}

esp_err_t read_alarm_to_flash() {
    nvs_handle_t get_alarm_nvs_handler;
    if (nvs_open("nvs", NVS_READONLY, &get_alarm_nvs_handler) != ESP_OK) {
        ESP_LOGE(TAG, "nvs open fail");
        nvs_close(get_alarm_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    size_t required_size = 1;
    if (nvs_get_u8(get_alarm_nvs_handler, "alarm", &alarm_time) != ESP_OK) {
        ESP_LOGE(TAG, "nvs get ssid fail");
        nvs_close(get_alarm_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "nvs get alarm:%d", alarm_time);
    nvs_close(get_alarm_nvs_handler);
    return ESP_OK;
}

const uint8_t alarm_time_setting_list[] = {
        15, 30, 60, 90, 120, 240
};

// 调时间逻辑：进入设置时杯子为放回状态，抬起一次下一个设定时间，放回20秒后为设定时间。
void set_alarm_time() {
    uint8_t set_time = 0;
    uint16_t count = 0;
    while (count++ < 2000) {
        esp_task_wdt_reset();
        delay(10);
        // 检测移开
        if (IS_CUP_OFF()) {
            delay(10);
            if (IS_CUP_OFF()) {
                set_time++;
                if (set_time >= 6) {
                    set_time = 0;
                }
                ESP_LOGI("setting", "select %d", alarm_time_setting_list[set_time]);
            }
        }
        // 等待放回（移开结束）
        while (IS_CUP_OFF()){
            esp_task_wdt_reset();
            delay(10);
        }
    }
    alarm_time = alarm_time_setting_list[set_time];
    save_alarm_to_flash();
    delay(1000);
    esp_restart();
}

void alarm_callback() {

}
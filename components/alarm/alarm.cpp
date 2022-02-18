//
// Created by yekai on 2022/2/15.
//

#include <nvs.h>
#include <esp_task_wdt.h>
#include <esp_log.h>
#include "alarm.h"
#include "alarm_url.h"
#include "oled.h"
#include "io_config.h"

#ifndef ALARM_URL
    #error Please set alarm url string with define
#endif
// 闹钟时长，单位分钟
uint8_t alarm_time;

static char *TAG = (char *) "alarm";

static esp_err_t save_alarm_to_flash() {
    nvs_handle_t save_alarm_nvs_handle;
    static char *TAG = (char *) "NSV save alarm";

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

    ESP_LOGI(TAG, "saved alarm:%d", (int) alarm_time);
    nvs_close(save_alarm_nvs_handle);
    return ESP_OK;
}

esp_err_t read_alarm_from_flash() {
    nvs_handle_t get_alarm_nvs_handler;
    static char *TAG = (char *) "NSV read alarm";

    if (nvs_open("nvs", NVS_READONLY, &get_alarm_nvs_handler) != ESP_OK) {
        ESP_LOGE(TAG, "nvs open fail");
        nvs_close(get_alarm_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    if (nvs_get_u8(get_alarm_nvs_handler, "alarm", &alarm_time) != ESP_OK) {
        ESP_LOGE(TAG, "nvs get alarm fail");
        nvs_close(get_alarm_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    ESP_LOGI(TAG, "read alarm:%d", alarm_time);
    nvs_close(get_alarm_nvs_handler);
    return ESP_OK;
}

const uint8_t alarm_time_setting_list[] = {
        15, 30, 60, 90, 120, 240
};
#define ALARM_TIME_LIST_LENGTH 6

// 调时间逻辑：进入设置时杯子为放回状态，抬起一次下一个设定时间，放回20秒后为设定时间。
void set_alarm_time() {
    uint8_t set_time_index = 0;
    uint16_t count = 0;

    read_alarm_from_flash();
    for (auto i = 0; i < ALARM_TIME_LIST_LENGTH; i++) {
        if (alarm_time == alarm_time_setting_list[i]) {
            set_time_index = i;
            break;
        }
    }
    OLED_ShowTimeSet(alarm_time_setting_list[set_time_index]);
    while (count++ < 2000) {
        esp_task_wdt_reset();
        vTaskDelay(10 / portTICK_RATE_MS);
        // 检测移开
        if (IS_CUP_OFF()) {
            vTaskDelay(10 / portTICK_RATE_MS);
            if (IS_CUP_OFF()) {
                set_time_index++;
                if (set_time_index >= ALARM_TIME_LIST_LENGTH) {
                    set_time_index = 0;
                }
                OLED_ShowTimeSet(alarm_time_setting_list[set_time_index]);
                ESP_LOGI(TAG, "select %d", alarm_time_setting_list[set_time_index]);
            }
        }
        // 等待放回（移开结束）
        while (IS_CUP_OFF()) {
            esp_task_wdt_reset();
            vTaskDelay(10 / portTICK_RATE_MS);
        }
    }
    alarm_time = alarm_time_setting_list[set_time_index];
    save_alarm_to_flash();
    vTaskDelay(1000 / portTICK_RATE_MS);
    esp_restart();
}

void alarm_callback() {

}
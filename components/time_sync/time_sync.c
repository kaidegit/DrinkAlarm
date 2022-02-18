//
// Created by yekai on 2021/8/9.
//
#include <stdio.h>
#include <time.h>
#include <esp_wifi.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/apps/sntp.h"
#include "esp_log.h"
#include "time_sync.h"
#include "wifi_smartconfig.h"
#include "oled.h"

time_sync_state_t timeSyncState = 0;

static const char *TAG = "sntp";

static void initialize_sntp(void) {
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "ntp1.aliyun.com");
    sntp_setservername(1, "ntp.tuna.tsinghua.edu.cn");
    sntp_setservername(2, "210.72.145.44");
    sntp_init();
}

esp_err_t sync_time_sntp(void) {
    timeSyncState = TIME_NOT_SYNC;

    EventBits_t conn_flag = xEventGroupWaitBits(
            s_wifi_event_group,
            CONNECTED_BIT,
            true,
            false,
            10000 / portTICK_RATE_MS
    );
    if (!(conn_flag & CONNECTED_BIT)) {
        return ESP_ERR_WIFI_NOT_CONNECT;
    }

    OLED_WaitTimeSync();

    char strftime_buf[64];
    initialize_sntp();

    time_t now = 0;
    int retry = 0;
    struct tm timeinfo = {0};

    while (timeinfo.tm_year < (2019 - 1900)) {
        // 15s超时
        if (retry > 1500) {
            ESP_LOGW(TAG, "sntp failed");
            timeSyncState = TIME_SYNC_FAILED;
            sntp_stop();
            return ESP_ERR_TIMEOUT;
        }
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d)", ++retry);
        vTaskDelay(100 / portTICK_RATE_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    // set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);

    timeSyncState = TIME_SYNC_SUCCESS;
    sntp_stop();
    return ESP_OK;
}

uint16_t  get_time(uint8_t *hr, uint8_t *min, uint8_t *sec) {
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);
    if ((hr != NULL) && (min != NULL) && (sec != NULL)) {
        *hr = timeinfo.tm_hour;
        *min = timeinfo.tm_min;
        *sec = timeinfo.tm_sec;
    }
    return timeinfo.tm_hour * 60 * 60 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
}
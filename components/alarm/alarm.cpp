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

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

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
        5, 15, 30, 60, 90, 120, 240
};
#define ALARM_TIME_LIST_LENGTH 7

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
    gpio_set_level((gpio_num_t) LED_Pin, 1);
    xTaskCreate(
            http_get_task,
            "http_get_task",
            4096,
            nullptr,
            3,
            nullptr
    );
}

// defines' format examples (ServerChan:) are as follows:
// #define WEB_URL "sctapi.ftqq.com"
// #define WEB_PATH "/<your_sendkey>?title=<your_title>&desp=<long_content>"
//
// more informations about ServerChan:https://sct.ftqq.com/
#ifndef WEB_URL
    #error Please set alarm url string with define
#endif
#ifndef WEB_PATH
    #error Please set alarm url string with define
#endif
#define WEB_PORT "80"
static const char *REQUEST =
        "GET " WEB_PATH " HTTP/1.0\r\n"
        "Host: " WEB_URL":" WEB_PORT"\r\n"
        "User-Agent: esp-idf/1.0 esp32\r\n"
        "\r\n";

void http_get_task(void *pvParameters) {
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r(0);
    char recv_buf[64];

    int err = getaddrinfo((const char *) WEB_URL, WEB_PORT, &hints, &res);

    if (err != 0 || res == nullptr) {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        vTaskDelete(nullptr);
    }

    /* Code to print the resolved IP.

       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    addr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
    ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0) {
        ESP_LOGE(TAG, "... Failed to allocate socket.");
        freeaddrinfo(res);
        vTaskDelete(nullptr);
    }
    ESP_LOGI(TAG, "... allocated socket");

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
        close(s);
        freeaddrinfo(res);
        vTaskDelete(nullptr);
    }

    ESP_LOGI(TAG, "... connected");
    freeaddrinfo(res);

    if (write(s, REQUEST, strlen(REQUEST)) < 0) {
        ESP_LOGE(TAG, "... socket send failed");
        close(s);
        vTaskDelete(nullptr);
    }
    ESP_LOGI(TAG, "... socket send success");

    struct timeval receiving_timeout = {};
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                   sizeof(receiving_timeout)) < 0) {
        ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        close(s);
        vTaskDelete(nullptr);
    }
    ESP_LOGI(TAG, "... set socket receiving timeout success");

    /* Read HTTP response */
    do {
        bzero(recv_buf, sizeof(recv_buf));
        r = read(s, recv_buf, sizeof(recv_buf) - 1);
        for (int i = 0; i < r; i++) {
            putchar(recv_buf[i]);
        }
    } while (r > 0);

    ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
    close(s);
    ESP_LOGI(TAG, "Http Request done");
    gpio_set_level((gpio_num_t) LED_Pin, 0);
    vTaskDelete(nullptr);
}
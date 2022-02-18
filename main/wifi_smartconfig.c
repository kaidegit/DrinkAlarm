/* Esptouch example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "wifi_smartconfig.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;
const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "smartconfig";

#define WIFI_SSID_LENGTH_MAX 32
#define WIFI_PASS_LENGTH_MAX 64

static esp_err_t save_wifi_to_flash(char *ssid, char *password) {
    nvs_handle_t save_wifi_nvs_handle;
    static char *TAG = "NSV save wifi";

    char *date_to_save;
    date_to_save = malloc(WIFI_PASS_LENGTH_MAX);
    if (nvs_open("nvs", NVS_READWRITE, &save_wifi_nvs_handle) != ESP_OK) {
        ESP_LOGE(TAG, "open nvs fail");
        nvs_close(save_wifi_nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    ESP_LOGI(TAG, "open nvs success,saving.....");
    memset(date_to_save, 0, WIFI_PASS_LENGTH_MAX);
    strcpy(date_to_save, ssid);
    if (nvs_set_str(save_wifi_nvs_handle, "ssid", date_to_save) != ESP_OK) {
        ESP_LOGE(TAG, "nvs save ssid fail");
        nvs_close(save_wifi_nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "saved ssid:%s", date_to_save);

    memset(date_to_save, 0, WIFI_PASS_LENGTH_MAX);
    strcpy(date_to_save, password);
    if (nvs_set_str(save_wifi_nvs_handle, "password", date_to_save) != ESP_OK) {
        ESP_LOGE(TAG, "nvs save password fail");
        nvs_close(save_wifi_nvs_handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "saved password:%s", date_to_save);

    free(date_to_save);
    nvs_close(save_wifi_nvs_handle);
    return ESP_OK;
}

static esp_err_t read_wifi_from_flash(wifi_config_t *nvs_wifi_config) {
    nvs_handle_t read_wifi_nvs_handler;
    static char *TAG = "NVS read WiFi";

    if (nvs_open("nvs", NVS_READONLY, &read_wifi_nvs_handler) != ESP_OK) {
        ESP_LOGE(TAG, "nvs open fail");
        nvs_close(read_wifi_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    size_t length_to_read = WIFI_SSID_LENGTH_MAX;
    if (nvs_get_str(read_wifi_nvs_handler, "ssid", (char *) nvs_wifi_config->sta.ssid, &length_to_read) != ESP_OK) {
        ESP_LOGE(TAG, "nvs get ssid fail");
        nvs_close(read_wifi_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "nvs get ssid:%s", nvs_wifi_config->sta.ssid);

    length_to_read = WIFI_PASS_LENGTH_MAX;
    if (nvs_get_str(read_wifi_nvs_handler, "password", (char *) nvs_wifi_config->sta.password, &length_to_read) !=
        ESP_OK) {
        ESP_LOGE(TAG, "nvs get password fail");
        nvs_close(read_wifi_nvs_handler);
        return ESP_ERR_NVS_NOT_FOUND;
    }
    ESP_LOGI(TAG, "nvs get password:%s", nvs_wifi_config->sta.password);

    nvs_close(read_wifi_nvs_handler);
    return ESP_OK;
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE) {
        ESP_LOGI(TAG, "Scan done");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL) {
        ESP_LOGI(TAG, "Found channel");
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD) {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *) event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        save_wifi_to_flash((char *) ssid, (char *) password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2) {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++) {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }
    } else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE) {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

void initialise_wifi(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL
    ));
    ESP_ERROR_CHECK(esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &event_handler,
            NULL
    ));
    ESP_ERROR_CHECK(esp_event_handler_register(
            SC_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL
    ));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t connect_wifi_from_flash() {
    wifi_config_t wifi_config = {0};
    if (read_wifi_from_flash(&wifi_config) != ESP_OK) {
        return ESP_ERR_WIFI_NVS;
    }
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_connect();
    return ESP_OK;
}

void smartconfig_task(void *parm) {
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    while (1) {
        uxBits = xEventGroupWaitBits(
                s_wifi_event_group,
                CONNECTED_BIT | ESPTOUCH_DONE_BIT,
                true,
                false,
                portMAX_DELAY
        );
        if (uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if (uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

void start_smartconfig() {
    xTaskCreate(
            smartconfig_task,
            "smartconfig_task",
            4096,
            NULL,
            3,
            NULL
    );

}
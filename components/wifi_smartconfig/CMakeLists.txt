idf_component_register(
        SRC_DIRS "."
        INCLUDE_DIRS "."
        REQUIRES esp_wifi esp_netif wpa_supplicant nvs_flash OLED
)
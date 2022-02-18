//
// Created by yekai on 2022/2/18.
//


#include "u8g2_port.h"
#include "io_config.h"
#include "u8g2.h"
#include "freertos/FreeRTOS.h"
#include "driver/spi_master.h"
#include "peripheral_init.h"
#include <driver/gpio.h>
#include <freertos/task.h>

u8g2_t u8g2;
spi_transaction_t t;

uint8_t u8x8_byte_4wire_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    auto data_p = (uint8_t *) arg_ptr;
    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            for (auto i = 0; i < arg_int; i++) {
                t.length = 8;
                t.tx_buffer = data_p;
                spi_device_polling_transmit(oled_spi_handle, &t);
                data_p++;
            }
            break;
        case U8X8_MSG_BYTE_INIT:
            break;
        case U8X8_MSG_BYTE_SET_DC:
            gpio_set_level((gpio_num_t) OLED_DC_Pin, (uint32_t) arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            gpio_set_level((gpio_num_t) OLED_CS_Pin, 0);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            gpio_set_level((gpio_num_t) OLED_CS_Pin, 1);
            break;
        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_esp32_gpio_and_delay(
        U8X8_UNUSED u8x8_t *u8x8, U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int, U8X8_UNUSED void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            vTaskDelay(1 / portTICK_RATE_MS);
            break;
        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(arg_int / portTICK_RATE_MS);
            break;
        case U8X8_MSG_GPIO_CS:
            gpio_set_level((gpio_num_t) OLED_CS_Pin, (uint32_t) arg_int);
        case U8X8_MSG_GPIO_DC:
            gpio_set_level((gpio_num_t) OLED_DC_Pin, (uint32_t) arg_int);
            break;
        case U8X8_MSG_GPIO_RESET:
            gpio_set_level((gpio_num_t) OLED_RES_Pin, (uint32_t) arg_int);
            break;
    }
    return 1;
}

void U8G2_Init() {
    OLED_GPIO_Init();
    OLED_SPI_Init();
    u8g2_Setup_ssd1306_128x64_noname_f(
            &u8g2,
            U8G2_R0,
            u8x8_byte_4wire_hw_spi,
            u8x8_esp32_gpio_and_delay
    );
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearDisplay(&u8g2);
}

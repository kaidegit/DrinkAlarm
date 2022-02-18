//
// Created by yekai on 2022/2/17.
//

#include "peripheral_init.h"
#include "io_config.h"

spi_device_handle_t oled_spi_handle;
void OLED_SPI_Init() {
    spi_bus_config_t buscfg = {
            .miso_io_num=-1,
            .mosi_io_num=OLED_SDA_Pin,
            .sclk_io_num=OLED_SCL_Pin,
            .quadwp_io_num=-1,
            .quadhd_io_num=-1,
            .max_transfer_sz=0
    };
    spi_device_interface_config_t devcfg = {
            .clock_speed_hz=1 * 1000 * 1000,        //Clock out at 1 MHz
            .mode=0,                                //SPI mode 0
            .spics_io_num=OLED_CS_Pin,                       //CS pin
            .queue_size=7,                          //We want to be able to queue 7 transactions at a time
    };
    ESP_ERROR_CHECK(spi_bus_initialize(OLED_SPI_HOST, &buscfg, SPI_DMA_DISABLED));
    ESP_ERROR_CHECK(spi_bus_add_device(OLED_SPI_HOST, &devcfg, &oled_spi_handle));
}

void OLED_GPIO_Init() {
    uint64_t gpio_mask;
    gpio_config_t io_conf;
    gpio_mask = (1ULL << OLED_DC_Pin) |
                (1ULL << OLED_RES_Pin) |
                (1ULL << OLED_CS_Pin);
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    io_conf.pin_bit_mask = gpio_mask;
    gpio_config(&io_conf);
}

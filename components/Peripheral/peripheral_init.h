//
// Created by yekai on 2022/2/17.
//

#ifndef MAIN_PERIPHERAL_INIT_H
#define MAIN_PERIPHERAL_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/spi_master.h"
#include "driver/gpio.h"

void OLED_SPI_Init();

void OLED_GPIO_Init();

void LED_GPIO_Init();

void SENSOR_GPIO_Init();

extern spi_device_handle_t oled_spi_handle;

#ifdef __cplusplus
}
#endif

#endif //MAIN_PERIPHERAL_INIT_H

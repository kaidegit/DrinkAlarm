//
// Created by yekai on 2022/2/12.
//

#ifndef MAIN_IO_CONFIG_H
#define MAIN_IO_CONFIG_H

// LED & Sensor Pins
#define LED_Pin 33
#define SENSOR_Pin 35

// OLED Pins
#define OLED_SDA_Pin 23
#define OLED_SCL_Pin 18
#define OLED_RES_Pin 21
#define OLED_DC_Pin 5
#define OLED_CS_Pin 17
#define OLED_SPI_HOST VSPI_HOST

// Cup state
#define IS_CUP_ON() (digitalRead(SENSOR_Pin) == LOW)
#define IS_CUP_OFF() (digitalRead(SENSOR_Pin) == HIGH)

#endif //MAIN_IO_CONFIG_H

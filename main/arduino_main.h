//
// Created by yekai on 2022/2/12.
//

#ifndef MAIN_ARDUINO_MAIN_H
#define MAIN_ARDUINO_MAIN_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "io_config.h"

#ifdef __cplusplus
extern "C" {
#endif

void setup();

void loop();

void settings_action_catch();

#ifdef __cplusplus
}
#endif

#endif //MAIN_ARDUINO_MAIN_H

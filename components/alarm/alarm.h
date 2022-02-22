//
// Created by yekai on 2022/2/15.
//

#ifndef MAIN_ALARM_H
#define MAIN_ALARM_H

void set_alarm_time();

esp_err_t read_alarm_from_flash();

void alarm_callback();

void http_get_task(void *pvParameters);

extern uint8_t alarm_time;

#endif //MAIN_ALARM_H

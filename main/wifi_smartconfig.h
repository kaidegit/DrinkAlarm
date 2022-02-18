//
// Created by yekai on 2022/2/15.
//

#ifndef MAIN_WIFI_SMARTCONFIG_H
#define MAIN_WIFI_SMARTCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

void initialise_wifi(void);

esp_err_t connect_wifi_from_flash();

void smartconfig_task(void *parm);

void start_smartconfig();

extern EventGroupHandle_t s_wifi_event_group;

extern const int CONNECTED_BIT;
extern const int ESPTOUCH_DONE_BIT;

#ifdef __cplusplus
}
#endif

#endif //MAIN_WIFI_SMARTCONFIG_H

//
// Created by yekai on 2022/2/15.
//

#ifndef MAIN_WIFI_SMARTCONFIG_H
#define MAIN_WIFI_SMARTCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

void initialise_wifi(void);

void connect_wifi_from_flash();

void smartconfig_task(void *parm);

void start_smartconfig();

#ifdef __cplusplus
}
#endif

#endif //MAIN_WIFI_SMARTCONFIG_H

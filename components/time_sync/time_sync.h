//
// Created by yekai on 2021/8/9.
//

#ifndef SMART_CONFIG_TIME_CALIBRATE_H
#define SMART_CONFIG_TIME_CALIBRATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

esp_err_t sync_time_sntp(void);

uint16_t get_time(uint8_t *hr, uint8_t *min, uint8_t *sec);

typedef int time_sync_state_t;

#define TIME_NOT_SYNC 0
#define TIME_SYNC_SUCCESS 1
#define TIME_SYNC_FAILED 2

extern time_sync_state_t timeSyncState;

#ifdef __cplusplus
}
#endif


#endif //SMART_CONFIG_TIME_CALIBRATE_H

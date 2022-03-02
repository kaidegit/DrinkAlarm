#include <stdio.h>
#include <esp_task_wdt.h>
#include "arduino_main.h"

void loopTask(void *pvParameters);

void app_main(void) {
    xTaskCreate(
            loopTask,
            "loopTask",
            4096,
            NULL,
            1,
            NULL
    );
}

void loopTask(void *pvParameters) {
    setup();
    for (;;) {
        esp_task_wdt_reset();
        loop();
    }
}
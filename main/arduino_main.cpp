//
// Created by yekai on 2022/2/12.
//

#include "arduino_main.h"

void setup() {
    pinMode(LED_Pin, OUTPUT);
}

void loop() {
    digitalWrite(LED_Pin, 0);
    delay(1000);
    digitalWrite(LED_Pin, 1);
    delay(1000);
}

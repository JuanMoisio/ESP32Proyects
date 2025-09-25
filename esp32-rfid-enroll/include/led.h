#ifndef LED_H
#define LED_H

#include <Arduino.h>

#define LED_GREEN 21
#define LED_RED 32

void ledInit();
void ledGreenOn();
void ledGreenOff();
void ledRedOn();
void ledRedOff();

#endif // LED_H
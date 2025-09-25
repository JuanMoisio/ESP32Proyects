#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>

void connectToWiFi();
bool isWiFiConnected();
String getWiFiIP();

#endif // WIFI_H
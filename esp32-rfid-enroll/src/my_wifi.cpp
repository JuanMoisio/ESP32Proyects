#include <Arduino.h>
#include "my_wifi.h"
#include <WiFi.h>
#include "secrets.h"



void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("Connecting to WiFi");
  unsigned long start = millis();
  const unsigned long timeout = 15000; // 15s
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout) {
    Serial.print('.');
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String getWiFiIP() {
  if (!isWiFiConnected()) return String();
  return WiFi.localIP().toString();
}
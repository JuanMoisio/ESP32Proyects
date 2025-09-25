#include <Arduino.h>
#include "my_wifi.h"
#include "my_webserver.h"   // <- updated include
#include "rfid.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();   // implementada en src/wifi.cpp
  setupWebServer();  // definida en src/webserver.cpp
  setupRFID();       // definida en src/rfid.cpp
  Serial.print("IP: ");
  Serial.println(getWiFiIP());
}

void loop() {
  handleClient(); // procesa peticiones web
  readRFID();     // lee tarjetas
}
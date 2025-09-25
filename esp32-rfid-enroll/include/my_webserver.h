#ifndef MY_WEBSERVER_H
#define MY_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>

// Forward declare WebServer (don't include library header here)
class WebServer;
extern WebServer server;

void setupWebServer();
void handleClient();

void handleRoot();
void handleEnroll();
void handleListCards();
void checkCardStatus(const String& uid);

#endif // MY_WEBSERVER_H
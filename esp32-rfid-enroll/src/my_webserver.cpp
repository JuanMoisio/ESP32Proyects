#include <Arduino.h>
#include <WebServer.h>     // library header (no name clash now)
#include "my_webserver.h"
#include "cards.h"
// Definición del servidor global
WebServer server(80);

void handleRoot() {
  String html = "<html><body><h1>RFID Enroll</h1><ul>";
  for (const String &uid : getEnrolledCards()) {
    html += "<li>" + uid + "</li>";
  }
  html += "</ul></body></html>";
  server.send(200, "text/html", html);
}

void handleEnroll() {
  if (!server.hasArg("uid")) {
    server.send(400, "text/plain", "Missing uid");
    return;
  }
  String uid = server.arg("uid");
  if (enrollCard(uid)) {
    server.send(200, "text/plain", "Enrolled");
  } else {
    server.send(200, "text/plain", "Already enrolled");
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/enroll", HTTP_POST, handleEnroll);
  server.begin();
  Serial.println("WebServer started on port 80");
}

void handleClient() {
  server.handleClient();
}
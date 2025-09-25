#pragma once
#include <Arduino.h>

void webserver_begin();
void webserver_loop();

// Estado de “armado” para la próxima tarjeta
bool isEnrollArmed();
bool isDeleteArmed();
String armedName();

// Setters (consumo automático o manual)
void armEnroll(bool on, const String& name = "");
void armDelete(bool on);

// Mensaje de último evento (se muestra en la UI)
void setLastEvent(const String& msg);

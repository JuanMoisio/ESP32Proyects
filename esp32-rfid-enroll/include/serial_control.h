#pragma once
#include <Arduino.h>

// inicializar (Serial debe estar iniciado en setup antes de llamar)
void serialControlBegin(HardwareSerial &ser, unsigned long baud = 115200);

// llamar continuamente desde loop()
void serialControlLoop();

// Debe llamarse desde el codigo que detecta la UID cuando pasa una tarjeta.
// Devuelve true si el evento de "SEND" consumido por la consola (y ya envio por serie).
void serialOnCardScanned(const String& uid);

// Utiles (opcionales)
void serialPrintln(const String& s);

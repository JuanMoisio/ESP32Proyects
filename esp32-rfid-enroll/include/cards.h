#ifndef CARDS_H
#define CARDS_H

#include <Arduino.h>
#include <MFRC522.h>
#include <vector>

String uidToHex(const MFRC522::Uid &uid);
bool enrollCard(const String &uid);
std::vector<String> getEnrolledCards();

#endif // CARDS_H
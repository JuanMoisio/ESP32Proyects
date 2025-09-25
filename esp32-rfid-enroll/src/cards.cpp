#include <Arduino.h>
#include "cards.h"

static std::vector<String> enrolledCards;

String uidToHex(const MFRC522::Uid &uid) {
  String s;
  for (byte i = 0; i < uid.size; i++) {
    if (uid.uidByte[i] < 0x10) s += "0";
    s += String(uid.uidByte[i], HEX);
  }
  s.toUpperCase();
  return s;
}

bool enrollCard(const String &uid) {
  for (auto &u : enrolledCards) if (u == uid) return false; // ya existe
  enrolledCards.push_back(uid);
  return true;
}

std::vector<String> getEnrolledCards() {
  return enrolledCards;
}
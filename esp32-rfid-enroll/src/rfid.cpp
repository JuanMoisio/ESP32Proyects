#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "rfid.h"
#include "cards.h"

// Ajusta los pines SS y RST según tu placa
constexpr uint8_t RST_PIN = 22;
constexpr uint8_t SS_PIN  = 5;

static MFRC522 mfrc522(SS_PIN, RST_PIN);

void setupRFID() {
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID inicializado");
}

void readRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  String uidHex = uidToHex(mfrc522.uid);
  Serial.print("Card UID: ");
  Serial.println(uidHex);
  // aquí podrías llamar enrollCard(uidHex) en función del modo
  mfrc522.PICC_HaltA();
}
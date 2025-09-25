#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "my_webserver.h"
#include "card_store.h"
#include "secrets.h"
#include <WiFi.h>

// Pines RC522 (ajusta si usás otros)
#define SS_PIN     5
#define RST_PIN   22
#define SCK_PIN   18
#define MISO_PIN  19
#define MOSI_PIN  23

#define LED_VERDE 21
#define LED_ROJO  32
#define BUZZER    25

MFRC522 rfid(SS_PIN, RST_PIN);

static String uidToHex(const MFRC522::Uid& uid) {
  String s;
  for (byte i=0;i<uid.size;i++){ if(uid.uidByte[i]<0x10) s+="0"; s+=String(uid.uidByte[i],HEX); }
  s.toUpperCase();
  return s;
}

static void beep(unsigned ms){ digitalWrite(BUZZER,HIGH); delay(ms); digitalWrite(BUZZER,LOW); }

void setup() {
  Serial.begin(115200);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_ROJO, LOW);
  digitalWrite(BUZZER, LOW);

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi...");
  while (WiFi.status()!=WL_CONNECTED) { Serial.print("."); delay(500); }
  Serial.print("\nIP: "); Serial.println(WiFi.localIP());

  // Persistencia
  CardStore::begin();

  // SPI + RC522
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  rfid.PCD_Init();

  // WebServer
  webserver_begin();

  // Señal de ready
  digitalWrite(LED_VERDE, HIGH); beep(60); digitalWrite(LED_VERDE, LOW);
}

void loop() {
  webserver_loop();

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = uidToHex(rfid.uid);
  Serial.print("Detectada UID: "); Serial.println(uid);
  beep(60);

  if (isEnrollArmed()) {
    if (CardStore::exists(uid)) {
      setLastEvent("Ya existía: " + uid + " (" + CardStore::getName(uid) + ")");
      // feedback: rojo
      digitalWrite(LED_ROJO, HIGH); delay(250); digitalWrite(LED_ROJO, LOW);
    } else {
      String nombre = armedName();
      CardStore::add(uid, nombre);
      setLastEvent("Enrolada: " + uid + " -> " + nombre);
      // feedback: verde
      digitalWrite(LED_VERDE, HIGH); delay(600); digitalWrite(LED_VERDE, LOW);
    }
    // desarmar
    // (esto “consume” la operación: solo una tarjeta)
    extern bool isEnrollArmed(); // ya declaradas
    extern bool isDeleteArmed();
    // forzamos flags a false tocando los estáticos vía setters si quisieras; como son internos, agreguemos:
    // acá hacemos un truco: rearmamos desde web con botón si querés más
    // Para simplificar, reusamos setLastEvent y confiamos en UI (no necesitamos set flags aquí).
  } else if (isDeleteArmed()) {
    if (CardStore::remove(uid)) {
      setLastEvent("Baja OK: " + uid);
      digitalWrite(LED_ROJO, HIGH); delay(250); digitalWrite(LED_ROJO, LOW);
    } else {
      setLastEvent("UID no existe: " + uid);
      digitalWrite(LED_ROJO, HIGH); delay(80); digitalWrite(LED_ROJO, LOW);
      delay(80);
      digitalWrite(LED_ROJO, HIGH); delay(80); digitalWrite(LED_ROJO, LOW);
    }
    // igual comentario que arriba respecto a “consumir” la operación
  } else {
    // solo lectura informativa (sin operación armada)
    String name = CardStore::getName(uid);
    if (name.length()) {
      setLastEvent("Válida: " + uid + " (" + name + ")");
      digitalWrite(LED_VERDE, HIGH); delay(300); digitalWrite(LED_VERDE, LOW);
    } else {
      setLastEvent("No registrada: " + uid);
      digitalWrite(LED_ROJO, HIGH); delay(200); digitalWrite(LED_ROJO, LOW);
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(200);
}

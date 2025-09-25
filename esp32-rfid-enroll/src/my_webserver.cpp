#include "my_webserver.h"
#include <WebServer.h>
#include <WiFi.h>
#include "card_store.h"
#include "secrets.h"

// ----------------- Estado global -----------------
static WebServer server(80);

static volatile bool g_arm_enroll = false;
static volatile bool g_arm_delete = false;
static String g_arm_name;
static String g_last_event;

// Getters/Setters de estado
bool   isEnrollArmed()            { return g_arm_enroll; }
bool   isDeleteArmed()            { return g_arm_delete; }
String armedName()                { return g_arm_name;   }
void   setLastEvent(const String& msg) { g_last_event = msg; }
void   armEnroll(bool on, const String& name) { g_arm_enroll = on; if (on) g_arm_name = name; }
void   armDelete(bool on)                { g_arm_delete = on; }

// ----------------- Helpers -----------------
static bool checkPIN() {
  if (!server.hasArg("pin")) return false;
  return server.arg("pin") == ADMIN_PIN;   // ADMIN_PIN en secrets.h
}

static String htmlEscape(const String& s) {
  String o = s;
  o.replace("&","&amp;");
  o.replace("<","&lt;");
  o.replace(">","&gt;");
  o.replace("\"","&quot;");
  return o;
}

// Callbacks de iteración (se usan en listado/JSON)
static void append_row_html(const String& uid, const String& name, void* ctx) {
  String* page = static_cast<String*>(ctx);
  *page += "<tr><td>" + htmlEscape(uid) + "</td><td>" + htmlEscape(name) + "</td></tr>";
}

static void append_item_json(const String& uid, const String& name, void* ctx) {
  String* json = static_cast<String*>(ctx);
  if (json->length() > 1 && json->charAt(json->length()-1) != '[') *json += ",";
  *json += "{\"uid\":\"" + uid + "\",\"name\":\"" + name + "\"}";
}

// ----------------- Handlers HTTP -----------------
static void handleRoot() {
  String ip = WiFi.localIP().toString();
  String page =
    "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>ESP32 RFID Enroll</title>"
    "<style>body{font-family:system-ui;margin:16px}input{margin:4px}table{border-collapse:collapse}"
    "td,th{border:1px solid #ddd;padding:6px}</style>"
    "</head><body>"
    "<h2>ESP32 RFID Enroll</h2>"
    "<p><b>IP:</b> " + ip + "</p>"
    "<p style='color:steelblue'><b>Evento:</b> " + htmlEscape(g_last_event) + "</p>"
    "<h3>Armar ENROLAR próxima tarjeta</h3>"
    "<form method='POST' action='/arm-enroll'>"
    "Nombre: <input name='name' required>"
    " PIN: <input name='pin' type='password' required>"
    " <button>Armar</button>"
    "</form>"
    "<h3>Armar ELIMINAR próxima tarjeta</h3>"
    "<form method='POST' action='/arm-delete'>"
    "PIN: <input name='pin' type='password' required>"
    " <button>Armar</button>"
    "</form>"
    "<h3>Eliminar por UID</h3>"
    "<form method='POST' action='/delete'>"
    "UID (HEX): <input name='uid' required>"
    " PIN: <input name='pin' type='password' required>"
    " <button>Eliminar</button>"
    "</form>"
    "<h3>Tarjetas</h3><table><tr><th>UID</th><th>Nombre</th></tr>";

  CardStore::forEach(append_row_html, &page);

  page += "</table></body></html>";
  server.send(200, "text/html", page);
}

static void handleArmEnroll() {
  if (!checkPIN()) { server.send(403, "text/plain", "PIN invalido"); return; }
  if (!server.hasArg("name")) { server.send(400, "text/plain", "Falta name"); return; }
  armEnroll(true, server.arg("name"));
  armDelete(false);
  setLastEvent("Enrolamiento ARMADO. Pase una tarjeta.");
  server.sendHeader("Location", "/"); server.send(303);
}

static void handleArmDelete() {
  if (!checkPIN()) { server.send(403, "text/plain", "PIN invalido"); return; }
  armDelete(true);
  armEnroll(false);
  setLastEvent("Baja ARMADA. Pase una tarjeta.");
  server.sendHeader("Location", "/"); server.send(303);
}

static void handleDelete() {
  if (!checkPIN()) { server.send(403, "text/plain", "PIN invalido"); return; }
  if (!server.hasArg("uid")) { server.send(400, "text/plain", "Falta uid"); return; }
  String uid = server.arg("uid"); uid.toUpperCase();
  bool ok = CardStore::remove(uid);
  setLastEvent(ok ? "Baja: " + uid : "UID no existe: " + uid);
  server.sendHeader("Location", "/"); server.send(303);
}

static void handleCardsJson() {
  String json = "[";
  CardStore::forEach(append_item_json, &json);
  json += "]";
  server.send(200, "application/json", json);
}

// ----------------- Arranque / loop -----------------
void webserver_begin() {
  server.on("/",           HTTP_GET,  handleRoot);
  server.on("/arm-enroll", HTTP_POST, handleArmEnroll);
  server.on("/arm-delete", HTTP_POST, handleArmDelete);
  server.on("/delete",     HTTP_POST, handleDelete);
  server.on("/cards",      HTTP_GET,  handleCardsJson);
  server.begin();
}

void webserver_loop() {
  server.handleClient();
}

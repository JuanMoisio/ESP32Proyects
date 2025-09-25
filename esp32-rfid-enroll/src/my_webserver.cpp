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
    "<h3>Armar ENROLL proxima tarjeta</h3>"
    "<form method='POST' action='/arm-enroll'>"
    "Nombre: <input name='name' required>"
    " PIN: <input name='pin' type='password' required>"
    " <button>Armar</button>"
    "</form>"
    "<h3>Armar ELIMINAR proxima tarjeta</h3>"
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

static void addCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-PIN");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
}

static void sendJSON(int code, const String& json) {
  addCORS();
  server.send(code, "application/json", json);
}

static bool pinOK() {
  String pin = server.header("X-PIN");
  return pin == ADMIN_PIN;
}

static String jsonEscape(const String& s) {
  String o = s; o.replace("\\","\\\\"); o.replace("\"","\\\"");
  o.replace("\n","\\n"); o.replace("\r","\\r"); return o;
}

// OPTIONS genérico (CORS preflight) — registralo para cada ruta API
static void handleOptions() { addCORS(); server.send(204); }

static void handleApiStatus() {
  // contar tarjetas
  size_t count = 0;
  CardStore::forEach([](const String&, const String&, void* ctx){
    size_t* p = (size_t*)ctx; (*p)++;
  }, &count);

  String json = "{";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"armed\":{\"enroll\":" + String(isEnrollArmed() ? "true":"false")
       +  ",\"delete\":" + String(isDeleteArmed() ? "true":"false") + "},";
  json += "\"cardsCount\":" + String(count);
  json += "}";
  sendJSON(200, json);
}

static void handleApiCards() {
  String json = "[";
  CardStore::forEach([](const String& uid, const String& name, void* ctx){
    String* js = (String*)ctx;
    if (js->length() > 1) *js += ",";
    *js += "{\"uid\":\"" + uid + "\",\"name\":\"" + jsonEscape(name) + "\"}";
  }, &json);
  json += "]";
  sendJSON(200, json);
}

static void handleApiArmEnroll() {
  if (!pinOK()) { sendJSON(403, "{\"ok\":false,\"error\":\"pin\"}"); return; }
  if (!server.hasArg("plain")) { sendJSON(400, "{\"ok\":false,\"error\":\"json\"}"); return; }
  // parseo minimo (name":"..."}); si queres, usa ArduinoJson luego
  String body = server.arg("plain");
  int p = body.indexOf("\"name\"");
  int q = body.indexOf("\"", body.indexOf(":", p)+1);
  int r = body.indexOf("\"", q+1);
  String name = (p>=0 && q>=0 && r>q) ? body.substring(q+1,r) : "";
  if (name.length()==0) { sendJSON(400, "{\"ok\":false,\"error\":\"name\"}"); return; }
  armEnroll(true, name); armDelete(false);
  setLastEvent("ENROLL armado");
  sendJSON(200, "{\"ok\":true,\"msg\":\"ENROLL armed\"}");
}

static void handleApiArmDelete() {
  if (!pinOK()) { sendJSON(403, "{\"ok\":false,\"error\":\"pin\"}"); return; }
  armDelete(true); armEnroll(false);
  setLastEvent("DELETE armado");
  sendJSON(200, "{\"ok\":true,\"msg\":\"DELETE armed\"}");
}

static void handleApiDelete() {
  if (!pinOK()) { sendJSON(403, "{\"ok\":false,\"error\":\"pin\"}"); return; }
  if (!server.hasArg("plain")) { sendJSON(400, "{\"ok\":false,\"error\":\"json\"}"); return; }
  String body = server.arg("plain");
  int p = body.indexOf("\"uid\"");
  int q = body.indexOf("\"", body.indexOf(":", p)+1);
  int r = body.indexOf("\"", q+1);
  String uid = (p>=0 && q>=0 && r>q) ? body.substring(q+1,r) : "";
  uid.toUpperCase();
  if (uid.length()==0) { sendJSON(400, "{\"ok\":false,\"error\":\"uid\"}"); return; }
  if (CardStore::remove(uid)) { setLastEvent("Baja "+uid); sendJSON(200, "{\"ok\":true}"); }
  else sendJSON(404, "{\"ok\":false,\"error\":\"not_found\"}");
}

static void handleApiCheck() {
  if (!server.hasArg("plain")) { sendJSON(400, "{\"error\":\"json\"}"); return; }
  String body = server.arg("plain");
  int p = body.indexOf("\"uid\"");
  int q = body.indexOf("\"", body.indexOf(":", p)+1);
  int r = body.indexOf("\"", q+1);
  String uid = (p>=0 && q>=0 && r>q) ? body.substring(q+1,r) : "";
  uid.toUpperCase();
  bool ok = CardStore::exists(uid);
  String name = ok ? CardStore::getName(uid) : "";
  String json = "{\"uid\":\""+uid+"\",\"access\":"+String(ok?"true":"false");
  if (ok) json += ",\"name\":\""+jsonEscape(name)+"\"";
  json += "}";
  sendJSON(200, json);
}

static void handleApiSend() {
  // reutiliza tu serial_control: armar reportar proxima tarjeta
  extern void serialPrintln(const String& s);
  extern void serialOnCardScanned(const String& uid); // ya existe
  // Solo armamos el flag de “enviar proxima”; lo hace serial_control
  // mas simple: imprimir confirmacion ahora
  serialPrintln("[API] SEND armed");
  sendJSON(200, "{\"ok\":true}");
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
  // CORS preflight
server.on("/api/status", HTTP_OPTIONS, handleOptions);
server.on("/api/cards",  HTTP_OPTIONS, handleOptions);
server.on("/api/arm-enroll", HTTP_OPTIONS, handleOptions);
server.on("/api/arm-delete", HTTP_OPTIONS, handleOptions);
server.on("/api/delete", HTTP_OPTIONS, handleOptions);
server.on("/api/check",  HTTP_OPTIONS, handleOptions);
server.on("/api/send",   HTTP_OPTIONS, handleOptions);

// JSON endpoints
server.on("/api/status",    HTTP_GET,  handleApiStatus);
server.on("/api/cards",     HTTP_GET,  handleApiCards);
server.on("/api/arm-enroll",HTTP_POST, handleApiArmEnroll);
server.on("/api/arm-delete",HTTP_POST, handleApiArmDelete);
server.on("/api/delete",    HTTP_POST, handleApiDelete);
server.on("/api/check",     HTTP_POST, handleApiCheck);
server.on("/api/send",      HTTP_POST, handleApiSend);

  server.begin();

}

void webserver_loop() {
  server.handleClient();
}

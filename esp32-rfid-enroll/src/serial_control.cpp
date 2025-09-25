#include "serial_control.h"
#include "card_store.h"
#include "my_webserver.h" // para armEnroll / armDelete / armEnroll(false) si se quiere
#include <vector>

static HardwareSerial* SERIAL_PORT = nullptr;
static String lineBuffer;
static bool request_send_next = false;

void serialControlBegin(HardwareSerial &ser, unsigned long baud) {
  SERIAL_PORT = &ser;
  SERIAL_PORT->begin(baud);
  SERIAL_PORT->println("[SerialControl] listo. Comandos: ENROLL <name>, DELETE, UNARM, SEND, CHECK <UID>, LIST");
}

static void println(const String& s) {
  if (SERIAL_PORT) SERIAL_PORT->println(s);
}

// split simple by spaces (max 2 parts: cmd and rest)
static void splitOnce(const String& s, String& cmd, String& rest) {
  int p = s.indexOf(' ');
  if (p < 0) { cmd = s; rest = ""; return; }
  cmd = s.substring(0,p);
  rest = s.substring(p+1);
}

// imprime todos los registros (CSV)
static void do_list() {
  println("UID,NAME");
  CardStore::forEach([](const String& uid, const String& name, void* ctx){
    HardwareSerial* sp = static_cast<HardwareSerial*>(ctx);
    sp->print(uid);
    sp->print(",");
    sp->println(name);
  }, SERIAL_PORT);
}

static void do_check_uid(const String& uidRaw) {
  String uid = uidRaw;
  uid.toUpperCase();
  bool ok = CardStore::exists(uid);
  String name = CardStore::getName(uid);
  println("CHECK " + uid + " ACCESS: " + String(ok ? "YES" : "NO") + (ok ? " NAME: " + name : ""));
}

static void do_arm_enroll(const String& name) {
  armEnroll(true, name);
  armDelete(false);
  println("ARMED_ENROLL name=\"" + name + "\"");
}

static void do_arm_delete() {
  armDelete(true);
  armEnroll(false);
  println("ARMED_DELETE");
}

static void do_unarm() {
  armEnroll(false);
  armDelete(false);
  println("UNARMED");
}

static void do_send_next() {
  request_send_next = true;
  println("SEND armed: next card will be reported to serial");
}

void serialOnCardScanned(const String& uid) {
  // Si hay una solicitud de envio para proxima tarjeta, consumila
  if (request_send_next) {
    request_send_next = false;
    bool ok = CardStore::exists(uid);
    String name = CardStore::getName(uid);
    String out = "CARD " + uid + " ACCESS: " + String(ok ? "YES" : "NO");
    if (ok) out += " NAME: " + name;
    println(out);
    return;
  }
  // Si no habia request_send_next, nada hace (las demas operaciones
  // ENROLL/DELETE las maneja el flujo principal que arma y consume el armado).
}

void serialControlLoop() {
  if (!SERIAL_PORT) return;
  while (SERIAL_PORT->available()) {
    char c = (char)SERIAL_PORT->read();
    if (c == '\r') continue;
    if (c == '\n') {
      String raw = lineBuffer;
      raw.trim();
      lineBuffer = "";
      if (raw.length() == 0) { /* ignore empty */ continue; }
      // parse command
      String cmd, rest;
      splitOnce(raw, cmd, rest);
      cmd.toUpperCase();
      if (cmd == "ENROLL") {
        if (rest.length() == 0) println("Usage: ENROLL <name>");
        else do_arm_enroll(rest);
      } else if (cmd == "DELETE") {
        do_arm_delete();
      } else if (cmd == "UNARM") {
        do_unarm();
      } else if (cmd == "SEND") {
        do_send_next();
      } else if (cmd == "CHECK") {
        if (rest.length() == 0) println("Usage: CHECK <UID>");
        else do_check_uid(rest);
      } else if (cmd == "LIST") {
        do_list();
      } else if (cmd == "HELP") {
        println("Commands: ENROLL <name>, DELETE, UNARM, SEND, CHECK <UID>, LIST");
      } else {
        println("Unknown command: " + cmd + "  (type HELP)");
      }
    } else {
      lineBuffer += c;
      // limit buffer length
      if (lineBuffer.length() > 200) lineBuffer = lineBuffer.substring(lineBuffer.length()-200);
    }
  }
}

void serialPrintln(const String& s) { println(s); }

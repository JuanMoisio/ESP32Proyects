#include "card_store.h"
#include <Preferences.h>

static Preferences prefs; // namespace "cards"
static const char* INDEX_KEY = "__index__";

namespace {
  String toKey(const String& uidHex) {
    String s(uidHex); s.toUpperCase(); return s;
  }
  // CSV helpers
  String loadIndex() { return prefs.getString(INDEX_KEY, ""); }
  void saveIndex(const String& csv) { prefs.putString(INDEX_KEY, csv); }

  bool indexHas(const String& uid) {
    String csv = loadIndex();
    csv += ",";
    String token = uid + ",";
    return csv.indexOf(token) >= 0 || csv.startsWith(uid + ",");
  }

  void indexAdd(const String& uid) {
    if (indexHas(uid)) return;
    String csv = loadIndex();
    if (csv.length() == 0) csv = uid;
    else csv += "," + uid;
    saveIndex(csv);
  }

  void indexRemove(const String& uid) {
    String csv = loadIndex();
    if (csv == uid) { saveIndex(""); return; }
    csv.replace("," + uid, "");
    if (csv.startsWith(uid + ",")) csv = csv.substring(uid.length() + 1);
    saveIndex(csv);
  }

  template<typename F>
  void forEachIndex(F cb) {
    String csv = loadIndex();
    int start = 0;
    while (true) {
      int comma = csv.indexOf(',', start);
      String uid = (comma < 0) ? csv.substring(start) : csv.substring(start, comma);
      uid.trim();
      if (uid.length() > 0) {
        String name = prefs.getString(uid.c_str(), "");
        if (name.length() > 0) cb(uid, name);
      }
      if (comma < 0) break;
      start = comma + 1;
    }
  }
}

namespace CardStore {
  void begin() { prefs.begin("cards", false); }

  bool exists(const String& uidHex) {
    String key = toKey(uidHex);
    return prefs.getString(key.c_str(), "").length() > 0;
  }

  bool add(const String& uidHex, const String& name) {
    String key = toKey(uidHex);
    if (exists(key)) return false;
    prefs.putString(key.c_str(), name);
    indexAdd(key);
    return true;
  }

  bool remove(const String& uidHex) {
    String key = toKey(uidHex);
    if (!exists(key)) return false;
    prefs.remove(key.c_str());
    indexRemove(key);
    return true;
  }

  String getName(const String& uidHex) {
    String key = toKey(uidHex);
    return prefs.getString(key.c_str(), "");
  }

  // Nuevo: callback con contexto
  void forEach(card_iter_cb cb, void* ctx) {
    forEachIndex([&](const String& uid, const String& name){
      cb(uid, name, ctx);
    });
  }
}

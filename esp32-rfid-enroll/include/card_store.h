#pragma once
#include <Arduino.h>

namespace CardStore {
  void begin();
  bool exists(const String& uidHex);
  bool add(const String& uidHex, const String& name);
  bool remove(const String& uidHex);
  String getName(const String& uidHex);

  // Nuevo: callback con contexto
  typedef void (*card_iter_cb)(const String& uid, const String& name, void* ctx);
  void forEach(card_iter_cb cb, void* ctx);
}

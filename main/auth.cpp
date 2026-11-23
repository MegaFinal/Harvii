#include "auth.h"
#include "esp_random.h"

#include <Preferences.h>
#include "config.h"

String gerarUUIDv4() {
  uint8_t bytes[16];
  esp_fill_random(bytes, sizeof(bytes));

  // Marca a versão (4) e a variante (RFC 4122)
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  char buf[37];
  snprintf(buf, sizeof(buf),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    bytes[0], bytes[1], bytes[2],  bytes[3],  bytes[4],  bytes[5],
    bytes[6], bytes[7], bytes[8],  bytes[9],  bytes[10], bytes[11],
    bytes[12], bytes[13], bytes[14], bytes[15]);

  return String(buf);
}

String auth_obter_hash() {
  Preferences prefs;
  prefs.begin(NVS_AUTH, false);
  String h = prefs.getString("hash", "");
  if (h == "") {
    h = gerarUUIDv4();
    prefs.putString("hash", h);
  }
  prefs.end();
  return h;
}
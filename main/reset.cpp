#include "reset.h"

#include <Preferences.h>
#include "config.h"
#include "display.h"

static volatile bool resetPendente = false;
static unsigned long apertadoDesde = 0;

/// Apaga WiFi e hash da flash e reinicia. No próximo boot o ESP32
/// abre o AP e gera um hash novo.
static void reset_fabrica() {
  Serial.println("Reset de fabrica");
  printOnDisplay("Resetando o Harvi", "Em instantes ele volta para a configuracao inicial");

  Preferences prefs;
  prefs.begin(NVS_WIFI, false); prefs.clear(); prefs.end();
  prefs.begin(NVS_AUTH, false); prefs.clear(); prefs.end();

  delay(200);
  ESP.restart();
}

void reset_iniciar() {
  pinMode(PIN_RESET, INPUT_PULLUP);
}

void reset_agendar() {
  resetPendente = true;
}

void reset_loop() {
  if (resetPendente) reset_fabrica();

  // Botão BOOT: LOW = apertado
  if (digitalRead(PIN_RESET) == LOW) {
    if (apertadoDesde == 0) {
      apertadoDesde = millis();
      Serial.println("[reset] BOOT apertado");
    }
    if (millis() - apertadoDesde > RESET_HOLD_MS) reset_fabrica();
  } else if (apertadoDesde != 0) {
    Serial.printf("[reset] BOOT solto apos %lu ms\n", millis() - apertadoDesde);
    apertadoDesde = 0;
  }
}
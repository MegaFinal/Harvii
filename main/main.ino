#include "config.h"
#include "display.h"
#include "wifi_manager.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);

  Serial.println("Build: " __DATE__ " " __TIME__);

  Serial.println("Iniciando display...");
  display_init();

  // Verifica se tem wifi -> não: inicia hotspot
  wifi_iniciar();

  servidor_iniciar();
}

void loop() {
  servidor_atender();
  wifi_loop();
  delay(10);
}
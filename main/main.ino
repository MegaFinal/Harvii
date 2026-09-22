#include "config.h"
#include "display.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "reset.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Build: " __DATE__ " " __TIME__);

  reset_iniciar();

  Serial.println("Iniciando display...");
  display_init();

  wifi_iniciar();
  servidor_iniciar();
}

void loop() {
  servidor_atender();
  wifi_loop();
  reset_loop();
  delay(10);
}
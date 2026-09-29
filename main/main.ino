#include "config.h"
#include "display.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "reset.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Build: " __DATE__ " " __TIME__);
  setCpuFrequencyMhz(160);   // padrão 240 MHz: consome menos e sobra desempenho para o Harvi

  printOnDisplay("Iniciando Harvis", "Conectando em instantes");

  reset_iniciar();

  Serial.println("Iniciando display...");
  display_init();

  wifi_iniciar();
  servidor_iniciar();

  delay(1000);
}

void loop() {
  servidor_atender();
  wifi_loop();
  reset_loop();
  delay(10);
}
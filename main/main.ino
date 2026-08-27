#include <WiFi.h>
#include <WebServer.h>
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_random.h"

//------------------------------- Display Definition -------------------------------
//definição dos pinos
#define EPD_CS 15
#define EPD_DC 21
#define EPD_RST 22
#define EPD_BUSY 23
#define EPD_SCLK 4 //SCL
#define EPD_MOSI 5 //DIM SDA
#define EPD_MISO -1

//Declaração única do obg display (junta o io + display)
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)); 

// ------------------------------- WiFi Definition -------------------------------
// Access Point credentials
const char* ssid = "ESP32";
const char* password = "12345678";

// New Wifi and password
String novoWifi = "";
String novaPassword = "";
 
// Create web server on port 80
WebServer server(80);

// ------------------------------- Setup -------------------------------
void setup() {
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
  Serial.begin(115200);

  // Inicializa display
  Serial.println("Iniciando display...");
  display.init(115200);
  display.setRotation(1);

  // Verifica se tem wifi -> não: inicia hostspot
  check_has_wifi();
 
  // Define routes (url)
  server.on("/save_wifi", handle_savewifi);
  server.on("/ask_code", handle_paringcode);
  server.on("/send_text", handle_print_text);
  server.on("/send_image", handle_show_image);
  server.begin();
}
 
// ------------------------------- Loop -------------------------------
void loop() {
  // Server atende as requisições
  server.handleClient();
  // Seed para criar o código
  randomSeed(analogRead(0));

  if (WiFi.status() == WL_CONNECTED) {
    
    server.handleClient();
    delay(10);
    
  } else {
    // micro pausa para não sobrecarregar o processador
    delay(10);
  }
}

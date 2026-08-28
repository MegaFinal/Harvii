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
 
 // wifi testar
const char* devWifiSSID = "";
const char* devWifiPassword = "";
// Create web server on port 80
WebServer server(80);

//conectar direto
void connectToWifiDirectly() {
  Serial.println("Conectando ao WiFi (modo dev)...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(devWifiSSID, devWifiPassword);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("Conectado! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Falha ao conectar ao WiFi. Verifique as credenciais em devWifiSSID/devWifiPassword.");
  }
}

// ------------------------------- Setup -------------------------------

void setup() {
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
  Serial.begin(115200);

  // Inicializa display
  Serial.println("Iniciando display...");
  display.init(115200);
  display.setRotation(1);

  // Avisa o servidor para não ignorar o nosso cabeçalho de segurança
  const char* headerKeys[] = {"X-Auth-Hash"};
  server.collectHeaders(headerKeys, 1);

  // Verifica se tem wifi -> não: inicia hostspot
  //check_has_wifi();

//conectar direto
  connectToWifiDirectly();
 
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

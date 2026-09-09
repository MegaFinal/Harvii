#include "web_server.h"

#include <WebServer.h>
#include <ArduinoJson.h>
#include "mbedtls/base64.h"

#include "config.h"
#include "display.h"
#include "auth.h"
#include "wifi_manager.h"

// Create web server on port 80
static WebServer server(80);

// SignIn Code
static String hash = gerarUUIDv4();

// Buffer para recebimento dos bitmaps
static uint8_t imgBuf[EPD_PLANE_BYTES];

// ------------------------------- Helpers -------------------------------

static String json_response(bool success, const char* message) {
  JsonDocument responseDoc;
  responseDoc["success"] = success;
  responseDoc["message"] = message;

  String responseString;
  serializeJson(responseDoc, responseString);

  return responseString;
}

/// Confere o hash enviado no header X-Auth-Hash. Responde 401 e devolve
/// false quando não bate.
static bool autorizado() {
  String hashRecebido = server.header("X-Auth-Hash");
  hashRecebido.trim(); // Remove os \r\n

  if (!server.hasHeader("X-Auth-Hash") || hashRecebido != hash) {
    server.send(401, "application/json", json_response(false, "Nao autorizado"));
    return false;
  }
  return true;
}

/// Lê o corpo do POST e faz o parse do JSON. Responde 400 e devolve false
/// quando o corpo está ausente ou malformado.
static bool ler_json(JsonDocument& doc) {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", json_response(false, "Body expected"));
    Serial.println("Erro 400");
    return false;
  }

  String jsonBody = server.arg("plain");
  Serial.println("\nReceived raw JSON:");
  Serial.println(jsonBody);

  DeserializationError error = deserializeJson(doc, jsonBody);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());

    server.send(400, "application/json", json_response(false, "Invalid JSON structure"));
    return false;
  }

  return true;
}

// ------------------------------- Handlers -------------------------------

static void handle_paringcode() {
  printOnDisplay("Pareando", ".........");

  server.send(200, "application/json", json_response(true, hash.c_str()));
  Serial.print("hash: ");
  Serial.println(hash);
  delay(500); //folego
}

static void handle_savewifi() {
  JsonDocument doc;
  if (!ler_json(doc)) return;

  if (!doc["ssid"].is<const char*>() || !doc["password"].is<const char*>()) {
    server.send(400, "application/json", json_response(false, "Campos ssid/password ausentes"));
    return;
  }

  String ssidRecebido  = doc["ssid"].as<String>();
  String senhaRecebida = doc["password"].as<String>();

  Serial.printf("Parsed ssid: %s\n", ssidRecebido.c_str());

  // Responde antes de conectar: conectar derruba o access point e o
  // celular nunca receberia esta resposta
  server.send(200, "application/json", json_response(true, "Data processed successfully"));
  Serial.println("Recebido pedido para novo WiFi pelo navegador!");
  delay(500); //folego

  wifi_conectar(ssidRecebido, senhaRecebida);
}

static void handle_print_text() {
  JsonDocument doc;
  if (!ler_json(doc)) return;

  if (doc["hash"].as<String>() != hash) {
    server.send(401, "application/json", json_response(false, "Nao autorizado"));
    return;
  }

  if (!doc["messageOne"].is<const char*>() || !doc["messageTwo"].is<const char*>()) {
    server.send(400, "application/json", json_response(false, "Campos messageOne/messageTwo ausentes"));
    return;
  }

  printOnDisplay(doc["messageOne"], doc["messageTwo"]);
  server.send(200, "application/json", json_response(true, "Data processed successfully"));
}

static void handle_show_image() {
  if (!autorizado()) return;

  // Confirma se vieram os dados da foto
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", json_response(false, "Corpo vazio"));
    return;
  }

  // Copia o corpo do post e reconstrói o bitmap
  String b64 = server.arg("plain");
  size_t outLen = 0; // Var que armazenará quantos bytes foram escritos
  int rc = mbedtls_base64_decode(imgBuf, sizeof(imgBuf), &outLen, (const unsigned char*)b64.c_str(), b64.length());

  // Verifica se deu erro na decodificação
  if (rc != 0 || outLen != sizeof(imgBuf)) {
    Serial.printf("base64 rc=%d outLen=%u b64Len=%u\n", rc, (unsigned)outLen, b64.length());
    server.send(400, "application/json", json_response(false, "Imagem invalida"));
    return;
  }

  // Responde antes de desenhar: o refresh do e-paper leva segundos
  server.send(200, "application/json", json_response(true, "Imagem desenhada"));
  // print_image(imgBuf, EPD_WIDTH, EPD_HEIGHT);
  print_image_4G(imgBuf);
}

// ------------------------------- API pública -------------------------------

void servidor_iniciar() {
  // Avisa o servidor para não ignorar o nosso cabeçalho de segurança
  const char* headerKeys[] = {"X-Auth-Hash"};
  server.collectHeaders(headerKeys, 1);

  server.on("/save_wifi",  handle_savewifi);
  server.on("/ask_code",   handle_paringcode);
  server.on("/send_text",  handle_print_text);
  server.on("/send_image", handle_show_image);
  server.begin();
}

void servidor_atender() {
  server.handleClient();
}
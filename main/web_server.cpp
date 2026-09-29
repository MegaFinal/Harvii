#include "web_server.h"

#include <WebServer.h>
#include <ArduinoJson.h>

#include "config.h"
#include "display.h"
#include "auth.h"
#include "wifi_manager.h"

#include "reset.h"

// Create web server on port 80
static WebServer server(80);

// SignIn Code
static String hash;

// A tela perdeu o gráfico e precisa que o app mande de novo.
// Começa ligado: logo depois do boot a tela não tem o gráfico.
static bool precisaAtualizar = true;

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
static bool hash_confere() {
  if (!server.hasHeader("X-Auth-Hash")) return false;
  String recebido = server.header("X-Auth-Hash");
  recebido.trim();
  return recebido == hash;
}

static bool autorizado() {
  if (!hash_confere()) {
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
  server.send(200, "application/json", json_response(true, hash.c_str()));
  Serial.print("hash: ");
  Serial.println(hash);

  // Responde primeiro: o refresh do e-paper leva ~2,5 s
  printOnDisplay("Celular pareado!", "Suas tarefas vao aparecer aqui em instantes");
  precisaAtualizar = true;   // a mensagem substituiu o gráfico
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

  // Só agenda: a conexão roda no loop(), depois que esta requisição
  // já foi respondida e encerrada
  wifi_agendar_conexao(ssidRecebido, senhaRecebida);

  // Retorna o nome do dispositivo para o app procurar. Ex: harvi-eb9c.local
  JsonDocument resp;
  resp["success"] = true;
  resp["message"] = "Data processed successfully";
  resp["name"]    = wifi_nome_dispositivo();
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
  Serial.println("Recebido pedido para novo WiFi pelo navegador!");
}

static void handle_status() {
  if (!autorizado()) return;

  JsonDocument resp;
  resp["success"]      = true;
  resp["message"]      = "ok";
  resp["needsRefresh"] = precisaAtualizar;
  String out;
  serializeJson(resp, out);
  server.send(200, "application/json", out);
}

static void handle_reset() {
  if (!autorizado()) return;
  server.send(200, "application/json", json_response(true, "Resetando"));
  reset_agendar();   // roda no loop(), depois que a resposta já saiu
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

  server.send(200, "application/json", json_response(true, "Data processed successfully"));
  printOnDisplay(doc["messageOne"], doc["messageTwo"]);
}

static uint8_t  bandBuf[EPD_BAND_BYTES];  // 9.600 bytes, o único custo de RAM
static uint32_t bandFill   = 0;
static uint32_t bytesTotal = 0;
static int16_t  bandY      = 0;
static bool     imgOk      = false;
static uint16_t imgStatus  = 200;
static const char* imgMsg  = "Imagem desenhada";

static void img_falhou(uint16_t status, const char* msg) {
  imgOk = false; imgStatus = status; imgMsg = msg;
  precisaAtualizar = true;   // a tela não recebeu a imagem: o app deve reenviar
}

static void flush_band() {
  if (bandFill == 0) return;
  int16_t lines = bandFill / EPD_ROW_BYTES;
  display_image_band(bandBuf, bandY, lines);
  bandY += lines;
  bandFill = 0;
}

static void handle_image_raw() {
  HTTPRaw& raw = server.raw();

  switch (raw.status) {

    case RAW_START:
      bandFill = 0; bytesTotal = 0; bandY = 0;
      imgOk = true; imgStatus = 200; imgMsg = "Imagem desenhada";

      // Os headers já foram lidos aqui, então dá pra autenticar antes
      // de aceitar um único byte de imagem.
      if (!hash_confere()) { img_falhou(401, "Nao autorizado"); return; }

      display_image_begin();
      break;

    case RAW_WRITE: {
      if (!imgOk) return;   // erro: continua consumindo o corpo, mas descarta

      const uint8_t* src = raw.buf;
      size_t restante = raw.currentSize;

      if (bytesTotal + restante > EPD_IMAGE_BYTES) {
        img_falhou(400, "Imagem maior que a tela");
        return;
      }

      while (restante > 0) {
        size_t espaco = EPD_BAND_BYTES - bandFill;
        size_t n = restante < espaco ? restante : espaco;

        memcpy(bandBuf + bandFill, src, n);
        bandFill += n; bytesTotal += n; src += n; restante -= n;

        if (bandFill == EPD_BAND_BYTES) flush_band();  // faixa cheia, desce pro painel
      }
      break;
    }

    case RAW_END:
      if (!imgOk) return;
      if (bytesTotal != EPD_IMAGE_BYTES) { img_falhou(400, "Imagem incompleta"); return; }
      flush_band();
      break;

    case RAW_ABORTED:
      img_falhou(400, "Transferencia interrompida");
      break;
  }
}

static void handle_show_image() {
  if (!imgOk) {
    server.send(imgStatus, "application/json", json_response(false, imgMsg));
    return;
  }
  server.send(200, "application/json", json_response(true, imgMsg));
  precisaAtualizar = false;
  display_image_end();   // responde antes: o refresh leva segundos
}

// ------------------------------- API pública -------------------------------

void servidor_iniciar() {
  hash = auth_obter_hash();
  
  // Avisa o servidor para não ignorar o nosso cabeçalho de segurança
  const char* headerKeys[] = {"X-Auth-Hash"};
  server.collectHeaders(headerKeys, 1);

  server.on("/save_wifi",  handle_savewifi);
  server.on("/ask_code",   handle_paringcode);
  server.on("/send_text",  handle_print_text);
  server.on("/send_image", HTTP_POST, handle_show_image, handle_image_raw);
  server.on("/reset",      HTTP_POST, handle_reset);
  server.on("/status", HTTP_GET, handle_status);
  server.begin();
}

void servidor_atender() {
  server.handleClient();
}

void servidor_pedir_atualizacao() {
  precisaAtualizar = true;
}
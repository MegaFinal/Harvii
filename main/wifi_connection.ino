
// Creates HTTP Server and let user connects

// SignIn Code
String hash = gerarUUIDv4();

#include "mbedtls/base64.h"

static uint8_t imgBuf[4736];

void handle_paringcode() {
  printOnDisplay("Pareando", ".........");

  // Send HTTP 200 Success response
  String responseString = json_response(true, hash.c_str());
  server.send(200, "application/json", responseString);
  Serial.println(responseString.c_str());

  Serial.println("hash: ");
  Serial.println(hash.c_str());
  delay(500); //folego
}

void handle_print_text() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body expected\"}");
    Serial.println("Erro 400");
    return;
  }

  String jsonBody = server.arg("plain");
  Serial.println("\nReceived raw JSON:");
  Serial.println(jsonBody);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonBody);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    
    server.send(400, "application/json", "{\"error\":\"Invalid JSON structure\"}");
    Serial.println("Erro em parsing");
    return;
  }

  if (doc.containsKey("hash") && doc["hash"] == hash.c_str() && doc.containsKey("messageOne") && doc.containsKey("messageTwo")) {
    printOnDisplay(doc["messageOne"], doc["messageTwo"]);
    String responseString = json_response(true, "Data processed successfully");
    server.send(200, "application/json", responseString);
  } else {
    String erroString = json_response(false, "Campos messageOne/messageTwo ausentes");
    server.send(400, "application/json", erroString);
  }
}

String json_response(bool success, const char* message) {
  JsonDocument responseDoc;
  responseDoc["success"] = success;
  responseDoc["message"] = message;

  String responseString;
  serializeJson(responseDoc, responseString);

  return responseString;
}

void handle_show_image() {
  String hashRecebido = server.header("X-Auth-Hash");
  hashRecebido.trim();
  hash.trim();

  if (!server.hasHeader("X-Auth-Hash") || hashRecebido != hash) {
    server.send(401, "application/json", json_response(false, "Nao autorizado"));
    return;
  }
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", json_response(false, "Corpo vazio"));
    return;
  }

  String b64 = server.arg("plain");
  size_t outLen = 0;
  int rc = mbedtls_base64_decode(imgBuf, sizeof(imgBuf), &outLen,
                                 (const unsigned char*)b64.c_str(), b64.length());

  if (rc != 0 || outLen != sizeof(imgBuf)) {
    Serial.printf("base64 rc=%d outLen=%u b64Len=%u\n", rc, (unsigned)outLen, b64.length());
    server.send(400, "application/json", json_response(false, "Payload invalido"));
    return;   // NÃO comente esse return
  }

  server.send(200, "application/json", json_response(true, "Imagem desenhada"));

  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(0, 0, imgBuf, 296, 128, GxEPD_BLACK);
  } while (display.nextPage());
}

void start_access_point() {
  // Manda pro display as instruções de conexão
  String linha1 = String("Wifi: ") + ssid;
  String linha2 = String("Senha: ") + password;

  // 2.c_str() converte Strings -> const char*
  printOnDisplay(linha1.c_str(), linha2.c_str());
  //delay(10000); //10 seg
 
  // Start Access Point
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  String ip = WiFi.softAPIP().toString();
  printOnDisplay("Entre no navegador:", ip.c_str());
  delay(5000); //5 seg
}

void handle_savewifi() {
  // Check if the request has body content
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"Body expected\"}");
    Serial.println("Erro 400");
    return;
  }

  // Get the raw JSON string from the client
  String jsonBody = server.arg("plain");
  Serial.println("\nReceived raw JSON:");
  Serial.println(jsonBody);

  // Allocate the JSON document (Use dynamic size matching your target payload)
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonBody);

  // Test if parsing succeeds
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    
    server.send(400, "application/json", "{\"error\":\"Invalid JSON structure\"}");
    Serial.println("Erro em parsing");
    return;
  }

  // Extract variables safely by checking existence or types
  if (doc.containsKey("ssid") && doc.containsKey("password")) {
    const char* ssid = doc["ssid"];
    const char* password = doc["password"]; 

    // Sava nas variáveis locais
    novoWifi = String(ssid);
    novaPassword = String(password);

    novoWifi.trim();
    novaPassword.trim();

    // Print parsed values to the Serial Monitor
    Serial.printf("Parsed Status: %s\n", ssid);
    Serial.printf("Parsed password: %s\n", password);

    String responseString = json_response(true, "Data processed successfully");

    // Send HTTP 200 Success response
    server.send(200, "application/json", responseString);
    Serial.println("Recebido pedido para novo WiFi pelo navegador!");
    delay(500); //folego

    connect_wifi(novoWifi.c_str(), novaPassword.c_str());
  } else {
    JsonDocument erroDoc;
    erroDoc["success"] = false;
    erroDoc["message"] = "Campos ssid/password ausentes";

    String erroString;
    serializeJson(erroDoc, erroString);

    server.send(400, "application/json", erroString);
  }
}

void connect_wifi(const char* nomeRede, const char* senhaRede) {
  Serial.print("Conectando a rede: ");
  Serial.println(nomeRede);

  printOnDisplay("Conectando ", "...");

  // Reseta o Access Point
  WiFi.softAPdisconnect(true);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(500);

  // tentativa de conexão
  WiFi.begin(nomeRede, senhaRede);

  // Tenta conectar no máximo 20 vezes
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  // Verifica se conectou ou se deu erro de tempo (timeout)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("Wi-Fi conectado com sucesso!");
    Serial.print("Endereco IP: ");
    Serial.println(WiFi.localIP());
    String ip = WiFi.localIP().toString();
    
    // Avisa na tela
    printOnDisplay("Online! Novo ip:", ip.c_str());
  } else {
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL:   Serial.println("SSID nao encontrado (5GHz? fora de alcance?)"); break;
      case WL_CONNECT_FAILED:  Serial.println("Senha incorreta"); break;
      case WL_DISCONNECTED:    Serial.println("Nao associou"); break;
      default: Serial.printf("status=%d\n", WiFi.status());
    }
    printOnDisplay("Erro ao conectar", "Reiniciando");
    delay(500);
    start_access_point();
  }
}

void check_has_wifi() {
  if (novoWifi != "" && novaPassword != "") {
    connect_wifi(novoWifi.c_str(), novaPassword.c_str());
  } else {
    start_access_point();
  }
}
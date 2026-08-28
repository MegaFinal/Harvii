#include "wifi_manager.h"

#include <WiFi.h>
#include "config.h"
#include "display.h"

// Credenciais da rede do usuário. Vivem só neste arquivo — o resto do
// projeto interage por wifi_conectar(), nunca lendo estas variáveis.
static String novoWifi = "";
static String novaPassword = "";

// Tenta conectar por no máximo 20 × 500 ms = 10 segundos
static const int MAX_TENTATIVAS = 20;

void wifi_iniciar_access_point() {
  // Manda pro display as instruções de conexão
  String linha1 = String("Wifi: ") + AP_SSID;
  String linha2 = String("Senha: ") + AP_PASSWORD;
  printOnDisplay(linha1.c_str(), linha2.c_str());

  // Start Access Point
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  String ip = WiFi.softAPIP().toString();
  printOnDisplay("Entre no navegador:", ip.c_str());
  delay(5000); //5 seg
}

void wifi_conectar(const String& ssid, const String& senha) {
  // Guarda para poder reconectar depois
  novoWifi = ssid;
  novaPassword = senha;
  novoWifi.trim();
  novaPassword.trim();

  Serial.print("Conectando a rede: ");
  Serial.println(novoWifi);
  printOnDisplay("Conectando ", "...");

  // Reseta o Access Point
  WiFi.softAPdisconnect(true);
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(500);

  // tentativa de conexão
  WiFi.begin(novoWifi.c_str(), novaPassword.c_str());

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < MAX_TENTATIVAS) {
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
    printOnDisplay("Online! Novo ip:", ip.c_str());
    return;
  }

  switch (WiFi.status()) {
    case WL_NO_SSID_AVAIL:   Serial.println("SSID nao encontrado (5GHz? fora de alcance?)"); break;
    case WL_CONNECT_FAILED:  Serial.println("Senha incorreta"); break;
    case WL_DISCONNECTED:    Serial.println("Nao associou"); break;
    default: Serial.printf("status=%d\n", WiFi.status());
  }

  printOnDisplay("Erro ao conectar", "Reiniciando");
  delay(500);
  wifi_iniciar_access_point();
}

void wifi_iniciar() {
  if (novoWifi != "" && novaPassword != "") {
    wifi_conectar(novoWifi, novaPassword);
  } else {
    wifi_iniciar_access_point();
  }
}
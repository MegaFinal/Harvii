#include "wifi_manager.h"

#include <WiFi.h>
#include "config.h"
#include "display.h"

// Credenciais da rede do usuário. Vivem só neste arquivo — o resto do
// projeto interage por wifi_conectar(), nunca lendo estas variáveis.
static String novoWifi = "";
static String novaPassword = "";

// Pedido de conexão vindo do servidor web, executado depois no loop()
static volatile bool pedidoConexao = false;
static String ssidPendente = "";
static String senhaPendente = "";

// Tenta conectar por no máximo 40 × 500 ms = 20 segundos
static const int MAX_TENTATIVAS = 40;

// Último motivo de desconexão informado pelo driver (0 = nenhum)
static volatile int ultimoMotivo = 0;

// ------------------------------- Diagnóstico -------------------------------

static void wifi_evento(WiFiEvent_t evento, WiFiEventInfo_t info) {
  if (evento == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    ultimoMotivo = info.wifi_sta_disconnected.reason;
    Serial.printf("\n[WiFi] desconectou, motivo=%d\n", ultimoMotivo);
  }
}

static void wifi_varrer_redes() {
  Serial.println("\nProcurando redes...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("  nenhuma rede encontrada");
  }
  for (int i = 0; i < n; i++) {
    Serial.printf("  %-24s  RSSI=%d  canal=%d  cripto=%d\n",
                  WiFi.SSID(i).c_str(), WiFi.RSSI(i),
                  WiFi.channel(i), WiFi.encryptionType(i));
  }
  WiFi.scanDelete();
}

// ------------------------------- Access point -------------------------------

static void mostrar_instrucoes_ap() {
  String linha1 = String("Wifi: ") + AP_SSID;
  String linha2 = String("Senha: ") + AP_PASSWORD;
  printOnDisplay(linha1.c_str(), linha2.c_str());
}

void wifi_iniciar_access_point() {
  mostrar_instrucoes_ap();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  String ip = WiFi.softAPIP().toString();
  printOnDisplay("Entre no navegador:", ip.c_str());
  delay(5000); // 5 seg
}

// ------------------------------- Conexão -------------------------------

void wifi_conectar(const String& ssid, const String& senha) {
  novoWifi = ssid;
  novaPassword = senha;
  novoWifi.trim();
  novaPassword.trim();

  Serial.print("Conectando a rede: [");
  Serial.print(novoWifi);
  Serial.println("]");
  printOnDisplay("Conectando ", "...");

  // Mantém o AP de pé e liga a estação, sem desligar o rádio
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect(false);
  delay(100);

  wifi_varrer_redes();

  ultimoMotivo = 0;
  WiFi.begin(novoWifi.c_str(), novaPassword.c_str());

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < MAX_TENTATIVAS) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado! IP: " + WiFi.localIP().toString());
    WiFi.softAPdisconnect(true);  // desliga só o AP, fica em STA
    printOnDisplay("Online! Novo ip:", WiFi.localIP().toString().c_str());
    return;
  }

  Serial.printf("\nFalhou. status=%d, ultimo motivo=%d\n",
                WiFi.status(), ultimoMotivo);

  // Para de tentar e volta só para AP. O AP nunca caiu, então não
  // precisa ser recriado: só redesenha as instruções no display.
  WiFi.disconnect(false);
  WiFi.mode(WIFI_AP);
  printOnDisplay("Erro ao conectar", "Tente de novo");
  delay(2000);
  mostrar_instrucoes_ap();
}

void wifi_iniciar() {
  WiFi.onEvent(wifi_evento);

  if (novoWifi != "" && novaPassword != "") {
    wifi_conectar(novoWifi, novaPassword);
  } else {
    wifi_iniciar_access_point();
  }
}

void wifi_agendar_conexao(const String& ssid, const String& senha) {
  ssidPendente = ssid;
  senhaPendente = senha;
  pedidoConexao = true;
}

void wifi_loop() {
  if (pedidoConexao) {
    pedidoConexao = false;
    wifi_conectar(ssidPendente, senhaPendente);
  }
}
#include "wifi_manager.h"

#include <WiFi.h>
#include "config.h"
#include "display.h"
#include <ESPmDNS.h>

#include <Preferences.h>

// Credenciais da rede do usuário. Vivem só neste arquivo — o resto do
// projeto interage por wifi_conectar(), nunca lendo estas variáveis.
static String novoWifi = "";
static String novaPassword = "";

// Pedido de conexão vindo do servidor web, executado depois no loop()
static volatile bool pedidoConexao = false;
static unsigned long pedidoEm = 0;
static String ssidPendente = "";
static String senhaPendente = "";

// Espera antes de conectar, para a resposta HTTP chegar ao celular
static const unsigned long ESPERA_ANTES_DE_CONECTAR_MS = 1500;

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

// ------------------------------- Persistência -------------------------------

static void salvar_credenciais() {
  Preferences prefs;
  prefs.begin(NVS_WIFI, false);
  prefs.putString("ssid", novoWifi);
  prefs.putString("senha", novaPassword);
  prefs.end();
}

static void carregar_credenciais() {
  Preferences prefs;
  prefs.begin(NVS_WIFI, true);
  novoWifi     = prefs.getString("ssid", "");
  novaPassword = prefs.getString("senha", "");
  prefs.end();
}

// ------------------------------- Access point -------------------------------

static void mostrar_instrucoes_ap() {
  String sub = String("Conecte o celular na rede ") + AP_SSID + ", senha " + AP_PASSWORD + ", e abra o app";
  printOnDisplay("Vamos configurar o Harvi!", sub.c_str());
}

void wifi_iniciar_access_point(bool mostrarInstrucoes) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  if (mostrarInstrucoes) mostrar_instrucoes_ap();
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
    printOnDisplay("Conectando ao WiFi", "Isso pode levar alguns segundos");

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
    WiFi.setSleep(false);   // sem modem sleep: o ESP32 não perde as perguntas do mDNS
    
    String nome = wifi_nome_dispositivo();
    MDNS.end();                          // caso já estivesse rodando
    if (MDNS.begin(nome.c_str())) {
      MDNS.addService("http", "tcp", 80);
      Serial.println("mDNS: " + nome + ".local");
    }
    delay(500);
    printOnDisplay("Harvi conectado!", "Volte o celular para o WiFi de casa e toque em Parear no app");
    salvar_credenciais();
    return;
  }

  Serial.printf("\nFalhou. status=%d, ultimo motivo=%d\n", WiFi.status(), ultimoMotivo);

  // Para de tentar e volta só para AP. Recria o AP porque, no boot,
  // ele ainda não tinha sido configurado com AP_SSID/AP_PASSWORD.
  WiFi.disconnect(false);
  printOnDisplay("Nao consegui conectar ao WiFi", "Para configurar de novo, conecte o celular na rede HARVI e abra o app");
  wifi_iniciar_access_point(false);   // sobe o AP sem apagar a mensagem de erro
}

void wifi_iniciar() {
  WiFi.onEvent(wifi_evento);
  carregar_credenciais();

  if (novoWifi != "") {
    wifi_conectar(novoWifi, novaPassword);
  } else {
    wifi_iniciar_access_point();
  }
}

void wifi_agendar_conexao(const String& ssid, const String& senha) {
  ssidPendente = ssid;
  senhaPendente = senha;
  pedidoEm = millis();
  pedidoConexao = true;
}

void wifi_loop() {
  if (pedidoConexao && millis() - pedidoEm > ESPERA_ANTES_DE_CONECTAR_MS) {
    pedidoConexao = false;
    wifi_conectar(ssidPendente, senhaPendente);
  }
}

String wifi_nome_dispositivo() {
  uint64_t mac = ESP.getEfuseMac();
  char buf[16];
  snprintf(buf, sizeof(buf), "harvi-%02x%02x",
           (uint8_t)(mac >> 32), (uint8_t)(mac >> 40));
  return String(buf);
}
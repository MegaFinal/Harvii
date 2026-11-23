#pragma once

// // ------------------------------- Pinos do display -------------------------------
// #define EPD_CS   15
// #define EPD_DC   21
// #define EPD_RST  22
// #define EPD_BUSY 23
// #define EPD_SCLK 4   // SCL
// #define EPD_MOSI 5   // DIN / SDA
// #define EPD_MISO -1  // o painel nao envia dados de volta

// ------------------------------- Pinos do display -------------------------------
#define EPD_CS   A17
#define EPD_DC   A16
#define EPD_RST  A15
#define EPD_BUSY A14
#define EPD_SCLK 18  // SCLK padrão do ESP32 (confirmar)
#define EPD_MOSI 23  // MOSI padrão do ESP32 (confirmar)
#define EPD_MISO -1  // o painel nao envia dados de volta

// ------------------------------- Painel -------------------------------
// Dimensoes em paisagem, ja considerando o setRotation(1).
// O painel e fisicamente 128 x 296 (retrato).
#define EPD_WIDTH  800
#define EPD_HEIGHT 480

// 2 bits por pixel, MSB primeiro. 3 = branco, 2 = cinza claro, 1 = cinza escuro, 0 = preto.

#define EPD_ROW_BYTES   (EPD_WIDTH / 4)                                 // 200 bytes por linha
#define EPD_IMAGE_BYTES ((uint32_t)EPD_ROW_BYTES * (uint32_t)EPD_HEIGHT) // 96.000, só pra validar
#define EPD_BAND_LINES  48                                              // divide 480 exato
#define EPD_BAND_BYTES  (EPD_ROW_BYTES * EPD_BAND_LINES)                // 9.600 bytes na RAM

// ------------------------------- Access Point -------------------------------
// Rede que o ESP32 cria quando ainda nao conhece nenhum WiFi.
#define AP_SSID     "HARVII"
#define AP_PASSWORD "12345678"

// ------------------------------- Persistência -------------------------------
#define NVS_WIFI "wifi"   // namespace das credenciais do WiFi
#define NVS_AUTH "auth"   // namespace do hash de pareamento

// ------------------------------- Reset de fábrica -------------------------------
#define PIN_RESET      0     // botão BOOT (GPIO0)
#define RESET_HOLD_MS  5000  // tempo segurando para resetar
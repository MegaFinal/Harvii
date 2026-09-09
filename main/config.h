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

// Bytes do buffer de imagem: o painel usa 2 bits por pixel (4 tons de cinza).
// 800 x 480 = 384.000 pixels x 2 bits / 8 bits por byte = 96.000 bytes.
#define EPD_PLANE_BYTES ((EPD_WIDTH * EPD_HEIGHT) / 4)

// ------------------------------- Access Point -------------------------------
// Rede que o ESP32 cria quando ainda nao conhece nenhum WiFi.
#define AP_SSID     "ESP32"
#define AP_PASSWORD "12345678"
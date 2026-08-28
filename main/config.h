#pragma once

// ------------------------------- Pinos do display -------------------------------
#define EPD_CS   15
#define EPD_DC   21
#define EPD_RST  22
#define EPD_BUSY 23
#define EPD_SCLK 4   // SCL
#define EPD_MOSI 5   // DIN / SDA
#define EPD_MISO -1  // o painel nao envia dados de volta

// ------------------------------- Painel -------------------------------
// Dimensoes em paisagem, ja considerando o setRotation(1).
// O painel e fisicamente 128 x 296 (retrato).
#define EPD_WIDTH  296
#define EPD_HEIGHT 128

// Bytes de um plano de cor: o painel usa 1 bit por pixel.
// 296 x 128 = 37.888 pixels / 8 bits por byte = 4.736 bytes.
#define EPD_PLANE_BYTES ((EPD_WIDTH * EPD_HEIGHT) / 8)

// ------------------------------- Access Point -------------------------------
// Rede que o ESP32 cria quando ainda nao conhece nenhum WiFi.
#define AP_SSID     "ESP32"
#define AP_PASSWORD "12345678"
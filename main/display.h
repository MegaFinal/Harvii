#pragma once

#include <Arduino.h>
#include <GxEPD2_4G_4G.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold24pt7b.h>   // título
#include <Fonts/FreeSans18pt7b.h>       // subtítulo
#include "config.h"

// Objeto global do painel — definido em display.cpp
// Tela: GDEY075T7 7.5" 800x480 preto e branco
// (o buffer 4G a 800x480 passa de 96000 bytes de RAM)
extern GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 8> display;

/// Inicializa o SPI e o painel. Chamar uma vez no setup().
void display_init();

/// Escreve um título (preto) e um subtítulo (cinza), centralizados na tela.
/// Textos longos quebram em várias linhas automaticamente.
void printOnDisplay(const char* text, const char* other_text);

void display_image_begin();
void display_image_band(const uint8_t* band, int16_t y, int16_t lines);
void display_image_end();
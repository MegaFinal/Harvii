#pragma once

#include <Arduino.h>
#include <GxEPD2_3C.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "config.h"

// Objeto global do painel — definido em display.cpp
extern GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display;

/// Inicializa o SPI e o painel. Chamar uma vez no setup().
void display_init();

/// Escreve duas linhas de texto: a primeira em preto, a segunda em vermelho.
void printOnDisplay(const char* text, const char* other_text);

/// Imprime um bitmap de 1 bit por pixel ocupando a tela inteira.
void print_image(const uint8_t* bitmap, int largura, int altura);
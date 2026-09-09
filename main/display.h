#pragma once

#include <Arduino.h>
#include <GxEPD2_4G_4G.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "config.h"

// Objeto global do painel — definido em display.cpp
// Tela: GDEY075T7 7.5" 800x480 preto e branco
// (o buffer 4G a 800x480 passa de 96000 bytes de RAM)
extern GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2> display;

/// Inicializa o SPI e o painel. Chamar uma vez no setup().
void display_init();

/// Escreve duas linhas de texto: a primeira em preto, a segunda em vermelho.
void printOnDisplay(const char* text, const char* other_text);

/// Imprime um bitmap de 1 bit por pixel ocupando a tela inteira.
void print_image(const uint8_t* bitmap, int largura, int altura);

/// Desenha a imagem de 4 tons recebida (formato 2 bits, o "2" do drawImage_4G).
void print_image_4G(const uint8_t* bitmap);
#include "display.h"
#include <SPI.h>

// Declaração única do objeto display (junta o io + display)
GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 8> display(
  GxEPD2_750_GDEY075T7(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

void display_init() {
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
  display.init(115200);
  display.setRotation(0);  // paisagem: 800 × 480
}

// Print no display com duas linhas
// Usa o padrao paginado (firstPage/nextPage): numa tela 800x480 o buffer
// cheio e grande, e o modo paginado evita estourar a RAM do ESP32.
void printOnDisplay(const char* text, const char* other_text) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 50);
    display.print(text);
    display.setTextColor(GxEPD_DARKGREY);
    display.setCursor(10, 80);
    display.print(other_text);
  } while (display.nextPage());
}

// Imprimir imagem
void display_image_begin() {
  // Nada aqui no caso normal. A primeira chamada de writeImage_4G dispara o
  // _Init_4G() da biblioteca, que carrega as LUTs de 4 tons e já preenche os
  // dois planos da controladora com branco.
  //
  // Se aparecer fantasma da imagem anterior, descomente — custa um refresh
  // branco inteiro (~5 s) com o celular esperando:
  Serial.println("Iniciando impressão imagem");
  display.clearScreen();
}

void display_image_band(const uint8_t* band, int16_t y, int16_t lines) {
  display.epd2.writeImage_4G(band, 2, 0, y, EPD_WIDTH, lines, false, false, false);
}

void display_image_end() {
  display.epd2.refresh(false);
}
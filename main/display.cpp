#include "display.h"
#include <SPI.h>

// Declaração única do objeto display (junta o io + display)
GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 2> display(
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
void print_image(const uint8_t* bitmap, int largura, int altura) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(0, 0, bitmap, largura, altura, GxEPD_BLACK);
  } while (display.nextPage());
}

// Desenha a imagem 4G recebida do app.
// O "2" e o formato de bits (confirmado pelo exemplo oficial pra 800x480).
// As flags (true,false,true) seguem o exemplo: invert, mirror_y, pgm.
void print_image_4G(const uint8_t* bitmap) {
  display.clearScreen();          // limpa (evita ghosting da GDEY075T7)
  display.epd2.drawImage_4G(bitmap, 2, 0, 0, 800, 480, false, false, false);
}
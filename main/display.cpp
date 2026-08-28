#include "display.h"
#include <SPI.h>

// Declaração única do objeto display (junta o io + display)
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(
  GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

void display_init() {
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
  display.init(115200);
  display.setRotation(1);  // paisagem: 296 × 128
}

// Print no display com preto e vermelho
void printOnDisplay(const char* text, const char* other_text) {
  display.fillScreen(GxEPD_WHITE);
  display.setFont(&FreeMonoBold12pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setCursor(10, 50);
  display.print(text);
  display.setCursor(10, 70);
  display.setTextColor(GxEPD_RED);
  display.print(other_text);
  display.display();
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
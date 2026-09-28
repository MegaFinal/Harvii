#include "display.h"
#include <SPI.h>

static const GFXfont* FONTE_TITULO    = &FreeSansBold24pt7b;
static const GFXfont* FONTE_SUBTITULO = &FreeSans18pt7b;
static const int16_t  MARGEM          = 40;  // espaço livre nas laterais
static const int16_t  ESPACO_BLOCOS   = 24;  // entre título e subtítulo
static const int      MAX_LINHAS      = 3;   // por bloco

// Declaração única do objeto display (junta o io + display)
GxEPD2_4G_4G<GxEPD2_750_GDEY075T7, GxEPD2_750_GDEY075T7::HEIGHT / 8> display(
  GxEPD2_750_GDEY075T7(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

void display_init() {
  SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
  display.init(115200);
  display.setRotation(0);  // paisagem: 800 × 480
}

// ------------------------------- Texto -------------------------------

/// Quebra o texto em linhas que cabem na largura da tela, sem cortar palavras.
static int quebrar_linhas(const char* texto, const GFXfont* fonte, String linhas[]) {
  display.setFont(fonte);
  int n = 0;
  String atual = "";
  String palavra = "";
  String t = String(texto) + " ";   // o espaço final fecha a última palavra

  for (size_t i = 0; i < t.length(); i++) {
    char c = t[i];
    if (c != ' ') { palavra += c; continue; }
    if (palavra.length() == 0) continue;

    String tentativa = atual.length() ? atual + " " + palavra : palavra;
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(tentativa, 0, 0, &x1, &y1, &w, &h);

    if (w > EPD_WIDTH - 2 * MARGEM && atual.length()) {
      if (n < MAX_LINHAS) linhas[n++] = atual;
      atual = palavra;
    } else {
      atual = tentativa;
    }
    palavra = "";
  }
  if (atual.length() && n < MAX_LINHAS) linhas[n++] = atual;
  return n;
}

/// Desenha as linhas centralizadas na horizontal, a partir da linha de base `y`.
/// Devolve a linha de base seguinte.
static int16_t desenhar_linhas(String linhas[], int n, const GFXfont* fonte,
                               uint16_t cor, int16_t y) {
  display.setFont(fonte);
  display.setTextColor(cor);
  for (int i = 0; i < n; i++) {
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(linhas[i], 0, 0, &x1, &y1, &w, &h);
    display.setCursor((EPD_WIDTH - w) / 2 - x1, y);
    display.print(linhas[i]);
    y += fonte->yAdvance;
  }
  return y;
}

// Usa o padrao paginado (firstPage/nextPage): numa tela 800x480 o buffer
// cheio e grande, e o modo paginado evita estourar a RAM do ESP32.
void printOnDisplay(const char* text, const char* other_text) {
  String titulo[MAX_LINHAS];
  String subtitulo[MAX_LINHAS];
  int nTitulo = quebrar_linhas(text, FONTE_TITULO, titulo);
  int nSub    = quebrar_linhas(other_text, FONTE_SUBTITULO, subtitulo);

  // Altura do bloco inteiro, para centralizar na vertical
  int16_t altura = nTitulo * FONTE_TITULO->yAdvance;
  if (nSub > 0) altura += ESPACO_BLOCOS + nSub * FONTE_SUBTITULO->yAdvance;

  // Cerca de 3/4 da altura da fonte fica acima da linha de base
  int16_t yTitulo = (EPD_HEIGHT - altura) / 2 + FONTE_TITULO->yAdvance * 3 / 4;

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    int16_t y = desenhar_linhas(titulo, nTitulo, FONTE_TITULO, GxEPD_BLACK, yTitulo);

    if (nSub > 0) {
      int16_t fimTitulo = y - FONTE_TITULO->yAdvance * 3 / 4;
      int16_t ySub = fimTitulo + ESPACO_BLOCOS + FONTE_SUBTITULO->yAdvance * 3 / 4;
      desenhar_linhas(subtitulo, nSub, FONTE_SUBTITULO, GxEPD_DARKGREY, ySub);
    }
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
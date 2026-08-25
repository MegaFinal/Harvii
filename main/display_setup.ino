
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

// Printa texto preto no display
void update_display(const String& text, int x, int y) {
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(x, y);
    display.print(text);
  } while (display.nextPage());
}

String gerarUUIDv4() {
  uint8_t bytes[16];
  esp_fill_random(bytes, sizeof(bytes));

  // Marca a versão (4) e a variante (RFC 4122)
  bytes[6] = (bytes[6] & 0x0F) | 0x40;
  bytes[8] = (bytes[8] & 0x3F) | 0x80;

  char buf[37];
  snprintf(buf, sizeof(buf),
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    bytes[0], bytes[1], bytes[2],  bytes[3],  bytes[4],  bytes[5],
    bytes[6], bytes[7], bytes[8],  bytes[9],  bytes[10], bytes[11],
    bytes[12], bytes[13], bytes[14], bytes[15]);

  return String(buf);
}
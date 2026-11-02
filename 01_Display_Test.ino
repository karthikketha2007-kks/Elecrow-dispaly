/*
 * 01_Display_Test - hardware initialization check (Test 1)
 *
 * Verifies: ESP32-S3 + PSRAM, RGB panel + framebuffer, backlight, board
 * revision detection, PCA9557 (V3.0) and GT911 presence. Prints a full
 * diagnostic report on Serial (115200) and draws a status screen.
 *
 * Arduino IDE: Board "ESP32S3 Dev Module", PSRAM "OPI PSRAM",
 *              Flash Size "4MB", Partition "Huge APP (3MB No OTA/1MB SPIFFS)".
 */
#include <ElecrowDisplay.h>

ElecrowDisplay display;

static void statusLine(int y, const char* label, bool ok, const char* detail) {
  display.setFont(&fonts::FreeSans12pt7b);
  display.setTextColor(WHITE, BLACK);
  display.drawText(label, 40, y);
  display.setTextColor(ok ? GREEN : RED, BLACK);
  display.drawText(ok ? "OK" : "FAIL", 300, y);
  display.setTextColor(LIGHTGRAY, BLACK);
  display.drawText(detail, 400, y);
}

void setup() {
  Serial.begin(115200);
  delay(300);  // give the USB-serial bridge time to attach (setup only)
  Serial.println("\n=== 01_Display_Test ===");

  const uint32_t t0 = millis();
  const bool ok = display.begin();
  const uint32_t t_init = millis() - t0;

  display.printDiagnostics(Serial);
  if (!ok) {
    Serial.printf("Display init FAILED: %s\n", display.errorString());
    return;  // backlight stays off; nothing else to do
  }

  display.clear(BLACK);
  display.fillRectangle(0, 0, display.width(), 60, NAVY);
  display.setFont(&fonts::FreeSansBold18pt7b);
  display.setTextColor(WHITE, NAVY);
  display.drawTextCentered("CrowPanel 7.0 - Hardware Init Test", display.width() / 2, 30);

  char buf[64];
  const ElecrowMemoryInfo m = ElecrowDiagnostics::memory();
  snprintf(buf, sizeof(buf), "%u KB total, %u KB free", (unsigned)(m.psram_total / 1024), (unsigned)(m.psram_free / 1024));
  statusLine(100, "PSRAM", m.psram_found, buf);
  snprintf(buf, sizeof(buf), "%dx%d, %.1f Hz, %u ms", (int)display.width(), (int)display.height(),
           display.rgb().refreshRate(), (unsigned)t_init);
  statusLine(150, "RGB panel", display.rgb().ready(), buf);
  snprintf(buf, sizeof(buf), "%u bytes @ %p", (unsigned)display.rgb().framebufferBytes(), (void*)display.rgb().framebuffer());
  statusLine(200, "Framebuffer", display.rgb().framebuffer() != nullptr, buf);
  statusLine(250, "Board revision", display.boardRevision() != ElecrowBoardRevision::Auto, display.boardRevisionString());
  const bool v3 = display.boardRevision() == ElecrowBoardRevision::V3;
  statusLine(300, "PCA9557 (V3.0)", v3 ? display.expander().present() : true, v3 ? "0x18 touch reset done" : "not fitted on V1/V2");
  snprintf(buf, sizeof(buf), "GT911 \"%s\" @ 0x%02X", display.touch().productId(), display.touch().address());
  statusLine(350, "Touch", display.touchAvailable(), display.touchAvailable() ? buf : "not found");
  statusLine(400, "Backlight", display.getBrightness() > 0, "GPIO2");

  display.setFont(&fonts::FreeSans9pt7b);
  display.setTextColor(GRAY, BLACK);
  display.drawTextCentered("Touch the screen: a dot is drawn where you touch.", display.width() / 2, 455);
}

void loop() {
  display.update();
  int16_t x, y;
  if (display.getTouch(x, y)) display.fillCircle(x, y, 4, YELLOW);
}

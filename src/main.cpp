// Minimal bring-up: M5GFX only + startup beep on GPIO2
#include <Arduino.h>
#include "lgfx_dial.hpp"

// Power hold: keep device awake after WAKE/RTC boot
static constexpr uint8_t POWER_HOLD = 46;  // G46 must be driven HIGH to maintain power

// Encoder pins (M5Dial)
[[maybe_unused]] static constexpr uint8_t ENC_CLK = 41;  // A
[[maybe_unused]] static constexpr uint8_t ENC_DT  = 40;  // B
[[maybe_unused]] static constexpr uint8_t ENC_SW  = 42;  // Button

// Buzzer (piezo via transistor)
static constexpr uint8_t BUZZER = 2;
static constexpr uint8_t LEDC_CHANNEL = 0;

// Display instance
static LGFX_DIAL display;

void setup() {
  // IMPORTANT: assert power hold as early as possible
  pinMode(POWER_HOLD, OUTPUT);
  digitalWrite(POWER_HOLD, HIGH);

  display.init();
  display.setBrightness(255);
  display.setRotation(0);
  display.fillScreen(TFT_BLACK);
  display.setTextColor(TFT_WHITE, TFT_BLACK);

  // Startup beep
  ledcAttachPin(BUZZER, LEDC_CHANNEL);
  ledcWriteTone(LEDC_CHANNEL, 2000);  // 2 kHz
  delay(120);
  ledcWriteTone(LEDC_CHANNEL, 0);

  const char *msg = "Hello, world";
  display.setTextSize(2);
  int16_t w = display.textWidth(msg);
  int16_t h = display.fontHeight();
  int16_t x = (display.width() - w) / 2;
  int16_t y = (display.height() - h) / 2;
  display.setCursor(x, y);
  display.print(msg);
}

void loop() {
  // Idle
}

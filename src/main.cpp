// M5Dial-based bring-up using official libs
#include <Arduino.h>
#include <M5Unified.h>
#include <M5Dial.h>

void setup()
{
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && millis() - start < 1500)
  {
    delay(10);
  }
  Serial.println();
  Serial.println("[BOOT] M5Dial (M5 libs) starting...");

  auto cfg = M5.config();
  // Use the two booleans as in forum sample: enable display & peripherals
  M5Dial.begin(cfg, true, true);

  // Make taps less fussy: allow more movement before flick, longer before hold
  M5Dial.Touch.setHoldThresh(700);   // ms before hold
  M5Dial.Touch.setFlickThresh(18);   // pixels to consider flick/drag

  // Basic screen test
  M5Dial.Display.setBrightness(200);
  M5Dial.Display.fillScreen(TFT_BLACK);
  M5Dial.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Dial.Display.setTextSize(2);
  M5Dial.Display.setTextDatum(textdatum_t::middle_center);
  M5Dial.Display.drawString("Hello, Dial", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
  Serial.println("[LCD] Drew hello via M5Dial.Display");

  // Beep on startup (2 kHz for 200 ms)
  M5Dial.Speaker.setVolume(180);
  M5Dial.Speaker.tone(2000, 200);
  Serial.println("[BUZ] Beep 2kHz for 200ms");

  // Prime encoder baseline for UI loop
  M5Dial.Encoder.readAndReset();
}

void loop()
{
  // Process inputs
  M5.update();
  M5Dial.update();

  static uint32_t last_ui = 0;
  static int brightness_pct = 80;   // 0..100
  static int32_t enc_total = 0;     // logical steps count (after division)
  static int pending_clicks = 0;
  static uint32_t next_click_time = 0;
  static int enc_accum = 0;         // accumulate raw encoder ticks
  constexpr int ENC_DIV = 10;       // 10 raw ticks -> 1 logical step
  constexpr int BRIGHT_MAX = 100;   // brightness range 0..100
  constexpr int BRIGHT_STEP = 1;    // percent per logical step

  // Touch visualization state
  static bool cross_active = false;
  static int16_t cross_x = 0, cross_y = 0;
  static uint32_t press_start_ms = 0;
  static int16_t press_x0 = 0, press_y0 = 0;
  static bool ripple_active = false;
  static int16_t ripple_x = 0, ripple_y = 0, ripple_r = 8;
  static uint32_t ripple_erase_at = 0;
  static bool invert_latched = false; // toggled by hardware button

  // Encoder delta: accumulate raw ticks; convert to logical steps with division
  int32_t d = M5Dial.Encoder.readAndReset();
  if (d != 0)
  {
    enc_accum += d;
  }
  int logical = 0;
  while (enc_accum >= ENC_DIV) { enc_accum -= ENC_DIV; ++logical; }
  while (enc_accum <= -ENC_DIV) { enc_accum += ENC_DIV; --logical; }
  if (logical != 0)
  {
    enc_total += logical;
    brightness_pct += logical * BRIGHT_STEP;
    if (brightness_pct < 0) brightness_pct = 0;
    if (brightness_pct > BRIGHT_MAX) brightness_pct = BRIGHT_MAX;
    int mapped = (brightness_pct * 255) / BRIGHT_MAX;
    M5Dial.Display.setBrightness(mapped);

    // Queue click sounds per logical step
    int add = logical > 0 ? logical : -logical;
    pending_clicks += add;
  }

  // Touch handling: draw crosshair, detect tap vs hold/drag, show ripple for tap
  if (M5Dial.Touch.getCount() > 0)
  {
    const auto &t = M5Dial.Touch.getDetail(0);
    if (t.wasPressed())
    {
      press_start_ms = millis();
      press_x0 = t.x; press_y0 = t.y;
      // Draw crosshair
      if (cross_active) {
        M5Dial.Display.drawLine(cross_x - 6, cross_y, cross_x + 6, cross_y, TFT_BLACK);
        M5Dial.Display.drawLine(cross_x, cross_y - 6, cross_x, cross_y + 6, TFT_BLACK);
      }
      cross_x = t.x; cross_y = t.y; cross_active = true;
      M5Dial.Display.drawLine(cross_x - 6, cross_y, cross_x + 6, cross_y, TFT_YELLOW);
      M5Dial.Display.drawLine(cross_x, cross_y - 6, cross_x, cross_y + 6, TFT_YELLOW);
    }
    if (t.isPressed())
    {
      // Update crosshair on movement
      if ((abs(t.x - cross_x) > 3) || (abs(t.y - cross_y) > 3))
      {
        if (cross_active) {
          M5Dial.Display.drawLine(cross_x - 6, cross_y, cross_x + 6, cross_y, TFT_BLACK);
          M5Dial.Display.drawLine(cross_x, cross_y - 6, cross_x, cross_y + 6, TFT_BLACK);
        }
        cross_x = t.x; cross_y = t.y; cross_active = true;
        M5Dial.Display.drawLine(cross_x - 6, cross_y, cross_x + 6, cross_y, TFT_YELLOW);
        M5Dial.Display.drawLine(cross_x, cross_y - 6, cross_x, cross_y + 6, TFT_YELLOW);
      }
    }
    if (t.wasReleased())
    {
      uint32_t dur = millis() - press_start_ms;
      int dx = t.x - press_x0; int dy = t.y - press_y0;
      int manhattan = abs(dx) + abs(dy);
      // Consider it a TAP if short and little movement
      if (dur <= 300 && manhattan <= 20)
      {
        // Tap ripple and click
        ripple_x = t.x; ripple_y = t.y; ripple_r = 10;
        M5Dial.Display.fillCircle(ripple_x, ripple_y, ripple_r, TFT_YELLOW);
        ripple_active = true;
        ripple_erase_at = millis() + 120;
        M5Dial.Speaker.tone(1800, 60);
      }
      else if (dur > 600)
      {
        // Long press: toggle global invert
        invert_latched = !invert_latched;
        M5Dial.Display.invertDisplay(invert_latched);
        M5Dial.Speaker.tone(900, 120);
      }

      // Clear crosshair on release
      if (cross_active) {
        M5Dial.Display.drawLine(cross_x - 6, cross_y, cross_x + 6, cross_y, TFT_BLACK);
        M5Dial.Display.drawLine(cross_x, cross_y - 6, cross_x, cross_y + 6, TFT_BLACK);
        cross_active = false;
      }
    }
  }

  // Ripple erase (non-blocking)
  if (ripple_active && millis() >= ripple_erase_at)
  {
    M5Dial.Display.fillCircle(ripple_x, ripple_y, ripple_r, TFT_BLACK);
    ripple_active = false;
  }

  // Hardware button (BtnA): toggle global invert and beep
  if (M5Dial.BtnA.wasClicked())
  {
    invert_latched = !invert_latched;
    M5Dial.Display.invertDisplay(invert_latched);
    M5Dial.Speaker.tone(invert_latched ? 1200 : 800, 100);
  }

  // Update screen header with encoder + brightness roughly at 10Hz or on change
  if (millis() - last_ui > 100 || logical != 0)
  {
    last_ui = millis();
    M5Dial.Display.fillRect(0, 0, M5Dial.Display.width(), 40, TFT_BLACK);
    M5Dial.Display.setTextDatum(textdatum_t::top_left);
    M5Dial.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Dial.Display.setTextSize(2);
    char buf[64];
    snprintf(buf, sizeof(buf), "Enc: %ld  Bright: %d%%  Btn:%s", (long)enc_total, brightness_pct, invert_latched ? "INV" : "NORM");
    M5Dial.Display.drawString(buf, 8, 8);
  }

  // Service queued encoder click sounds without blocking UI
  const uint16_t CLICK_FREQ = 1800;     // Hz
  const uint16_t CLICK_ON_MS = 12;      // tone duration
  const uint16_t CLICK_GAP_MS = 18;     // gap after each click
  uint32_t now = millis();
  if (pending_clicks > 0 && now >= next_click_time)
  {
    M5Dial.Speaker.tone(CLICK_FREQ, CLICK_ON_MS);
    next_click_time = now + CLICK_ON_MS + CLICK_GAP_MS;
    pending_clicks--;
  }

  // Heartbeat to serial every second
  static uint32_t last_hb = 0;
  if (millis() - last_hb > 1000)
  {
    last_hb = millis();
    Serial.println("[HB] alive");
  }
}

#pragma once

#include <M5GFX.h>
#include <lgfx/v1/platforms/esp32/Bus_SPI.hpp>
#include <lgfx/v1/panel/Panel_GC9A01.hpp>
#include <lgfx/v1/platforms/esp32/Light_PWM.hpp>
#include <driver/spi_master.h>

// LovyanGFX configuration for M5Stack Dial (ESP32-S3 + GC9A01)

class LGFX_DIAL : public lgfx::v1::LGFX_Device {
 public:
  LGFX_DIAL() {
    { // SPI bus
      auto cfg = _bus.config();
      cfg.spi_host    = SPI2_HOST;  // FSPI
      cfg.spi_mode    = 0;
      cfg.freq_write  = 40000000;   // 40 MHz
      cfg.freq_read   = 16000000;
      cfg.spi_3wire   = true;       // write-only
      cfg.use_lock    = true;
      cfg.dma_channel = 1;

      cfg.pin_sclk = 36;  // SCK
      cfg.pin_mosi = 35;  // MOSI
      cfg.pin_miso = -1;  // not used
      cfg.pin_dc   = 7;   // DC

      _bus.config(cfg);
      _panel.setBus(&_bus);
    }

    { // Panel
      auto pcfg = _panel.config();
      pcfg.pin_cs         = 6;   // CS
      pcfg.pin_rst        = 5;   // RST
      pcfg.pin_busy       = -1;
      pcfg.panel_width    = 240;
      pcfg.panel_height   = 240;
      pcfg.offset_x       = 0;
      pcfg.offset_y       = 0;
      pcfg.offset_rotation= 0;
      pcfg.dummy_read_pixel = 8;
      pcfg.dummy_read_bits  = 1;
      pcfg.readable       = false;
      pcfg.invert         = true;
      pcfg.rgb_order      = false;
      pcfg.dlen_16bit     = false;
      pcfg.bus_shared     = false;
      _panel.config(pcfg);
    }

    { // Backlight
      auto lcfg = _light.config();
      lcfg.pin_bl = 38;     // BL
      lcfg.invert = false;
      lcfg.freq   = 44100;
      lcfg.pwm_channel = 7;
      _light.config(lcfg);
      _panel.setLight(&_light);
    }

    setPanel(&_panel);
  }

 private:
  lgfx::v1::Bus_SPI _bus;
  lgfx::v1::Panel_GC9A01 _panel;
  lgfx::v1::Light_PWM _light;
};

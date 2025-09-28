# MStack Dial Quick Demo

Here's a quick interactive playground for the M5Stack Dial round display which uses the S3 stamp. Rotate the dial to change brightness, tap for a ping, drag a crosshair showing X,Y, press BtnA to change colours, long press for a full screen 'starbust', and long‑press the screen to invert colours.

![M5Stack Dial demo](docs/assets/m5stack-demo.gif)

I made this because the manufacturer libraries and docs seem either too much or too little.

The outer ring is a 'poker chip' style I 3D printed (available on request).

## Purpose

This repo is meant as a hands‑on demo of the core I/O and rendering paths on the Dial/StampS3. It shows, with minimal code and no LVGL:

- Touch
  - Tap (with movement threshold) → visual ping + debug log
  - Drag (live X,Y, crosshair overlay)
  - Long press (screen) → invert colours
- Sound
  - Directional click tones on rotary up/down
  - Two‑tone sounds
  - Effect tones (starburst)
- Rotary encoder
  - Accumulator → logical detents (default 10% per step)
  - Event logging and click feedback
- Screen brightness
  - Brightness mapped from 0–100% → 0–255 via `Display.setBrightness`
- Rendering patterns
  - Watch‑face ticks from the physical edge inward
  - Theme‑driven colors (primary/text/accent/ripple)
  - Small‑area snapshot/restore (`readRect`/`pushImage`) overlays to avoid full redraws
- Buttons
  - BtnA press: cycle theme; hold: starburst effect
- Debug & Config
  - Event‑based logs (PRESS/DRAG/RELEASE, ROT, BTN, effects)
  - All tunables in `Config` at the top of `src/main.cpp`

## Features

- Brightness dial with watch‑face ticks (0–100%) and big percent readout
- 6 colour themes
- Crosshair you can drag anywhere; live X/Y coordinates
- Tap “ping” (outline 'shockwave')
- BtnA:
  - Press: cycle colours
  - Hold: starburst effect (full‑screen, two‑tone sound)
- Long press on screen: invert display (two‑tone be‑boop)
- Event‑based Serial debug logs (PRESS / DRAG / RELEASE, ROT, BTN, effects)

## Libraries and architecture

This project is intentionally lightweight and does not use LVGL or other widget frameworks.

- M5Unified — common HAL/shim for input, display, and audio across M5 devices
- M5GFX — fast graphics backend used by M5Unified’s `Display`
- M5Dial — glue for the StampS3 Dial hardware that exposes `Display`, `Touch`, `Encoder`, `Speaker`

Why no LVGL?
- The UI is simple and highly animated. Direct M5GFX calls have lower overhead and avoid additional buffers/complexity. Code stays small and easy to tweak.

How it’s wired
- Display: `M5Dial.Display` (from M5Unified/M5GFX) for all drawing (lines, circles, text).
- Touch: `M5Dial.Touch` with a tiny state machine to detect tap/drag/long‑press reliably.
- Encoder: `M5Dial.Encoder.readAndReset()` → small accumulator → 10% logical steps, with up/down click tones.
- Speaker: `M5Dial.Speaker.tone(freq, ms)`; a simple timer schedules two‑tone sounds.
- Overlays: small `readRect`/`pushImage` snapshots for crosshair/ping to avoid full‑frame redraws and flicker.

PlatformIO deps (from `platformio.ini`):

```
lib_deps =
  m5stack/M5Dial
  m5stack/M5Unified
  m5stack/M5GFX
```

## Build

- Dev build (debug on):
  - `pio run -e m5dial`
- Release build (debug off):
  - `pio run -e release`
- Upload: `pio run -e m5dial -t upload`
- Monitor: `pio device monitor -b 115200`

## Event logs to serial

- `[ROT] +10% -> br=70%`
- `[TOUCH] PRESS x=.. y=..`
- `[TOUCH] DRAG start d2=..`
- `[TOUCH] RELEASE dur=.. TAP -> ping`
- `[TOUCH] RELEASE dur=.. invert (no-drag)`
- `[TOUCH] RELEASE dur=.. drag refresh`
- `[BTN] A press -> theme N`
- `[BTN] A hold -> starburst`
- `[EFFECT] Starburst start/end`
- `[PING] end`

---

Have fun.

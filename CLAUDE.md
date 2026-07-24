# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Firmware for a DIY 4-button USB HID macro keypad ("Stream Deck") on a Seeed
XIAO ESP32S3. It enumerates as a USB keyboard and sends Cmd/GUI + function-key
chords that are bound to OBS Studio hotkeys. There is no companion app or
backend — the entire project is the firmware in [src/main.cpp](src/main.cpp).

## Commands

This is a [PlatformIO](https://platformio.org/) project (`platformio.ini`,
env `seeed_xiao_esp32s3`, Arduino framework). There is no separate test suite.

```bash
pio run                    # build
pio run --target upload    # flash to the board
pio device monitor          # serial monitor, 115200 baud
```

## Architecture

Everything lives in a single file, [src/main.cpp](src/main.cpp), structured
around three parallel arrays indexed by button number (0–3):

- `BUTTON_PINS` — GPIO pin per button (GPIO1–GPIO4, `INPUT_PULLUP`, active-low)
- `BUTTON_KEYS` — the HID keycode sent for that button (paired with
  `KEY_LEFT_GUI`)
- `BUTTON_IS_TOGGLE` — whether the button is a start/stop toggle (Record,
  Stream) vs. a one-shot action (Scene switches)

When adding/remapping a button, all three arrays (plus `BUTTON_LABELS`) must
stay in sync by index.

Key points to know before changing behavior:

- **Debouncing** is done per-button in the main loop via a timestamp
  (`lastChangeTime`) compared against `DEBOUNCE_MS`, not with a library.
- **Toggle state** (`toggleOn`) is a local guess used only for serial
  logging — it is not read back from OBS, so it can drift out of sync if OBS
  is toggled some other way (its own UI, a different hotkey, a crash). Don't
  use `toggleOn` to gate behavior as if it were ground truth.
- **Key sends** are fire-and-forget: press GUI+key, `delay(10)`, release all.
  Record/Stream intentionally share one keycode between OBS's separate
  Start/Stop hotkey slots — OBS only reacts to whichever is valid for the
  current state, so one physical button toggles both.
- OBS-side hotkey bindings (Settings > Hotkeys) must match `BUTTON_KEYS`
  manually; this repo has no way to verify or configure that from the
  firmware side.

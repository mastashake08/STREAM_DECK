# Stream Deck

A DIY USB HID macro keypad built on the Seeed XIAO ESP32S3. It presents itself
to the host computer as a keyboard and sends Cmd/GUI + function-key
combinations for controlling OBS Studio hotkeys.

## Hardware

- **Board:** Seeed Studio XIAO ESP32S3
- **Buttons:** 4 momentary push buttons wired to GPIO1–GPIO4, each to GND
  (uses internal pull-ups, active-low)

| Button | GPIO | Key sent  | Behavior           |
|--------|------|-----------|---------------------|
| 1      | 1    | Cmd+F10   | Record (start/stop toggle) |
| 2      | 2    | Cmd+F9    | Stream (start/stop toggle) |
| 3      | 3    | Cmd+F8    | Scene 1 (one-shot)  |
| 4      | 4    | Cmd+F7    | Scene 2 (one-shot)  |

Bind each of these key combinations in OBS under **Settings > Hotkeys**. The
Record and Stream buttons share one key between their Start/Stop hotkey
slots — OBS only reacts to whichever is valid for the current state, so a
single key press toggles it.

## Firmware behavior

- Each button is debounced in software (30 ms).
- On press, the firmware holds `KEY_LEFT_GUI` + the mapped function key for
  ~10 ms via USB HID, then releases all keys.
- For toggle buttons (Record/Stream) the firmware tracks an assumed on/off
  state locally for logging purposes only. This is not read back from OBS,
  so it can drift out of sync if OBS is toggled another way (its UI, a
  different hotkey, a crash, etc.).
- Status and button events are logged over USB serial at 115200 baud.

## Building and flashing

This is a [PlatformIO](https://platformio.org/) project targeting the
`seeed_xiao_esp32s3` board with the Arduino framework.

```bash
# Build
pio run

# Flash
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

Or open the project folder in VS Code with the PlatformIO extension
installed and use its build/upload/monitor commands.

## Project layout

```
src/main.cpp       Firmware: button reads, debounce, USB HID key sends
platformio.ini     Board/platform/framework config and build flags
include/, lib/      Empty PlatformIO scaffold directories
test/               Empty PlatformIO scaffold directory
```

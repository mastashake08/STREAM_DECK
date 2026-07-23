#include <Arduino.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

USBHIDKeyboard Keyboard;

const uint8_t BUTTON_PINS[] = {1, 2, 3, 4}; // GPIO1-GPIO4
const uint8_t NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);
const unsigned long DEBOUNCE_MS = 30;

// Bind each of these in OBS: Settings > Hotkeys.
// Record/Stream keys are shared between the Start and Stop hotkey slots -
// OBS only acts on whichever one is valid for the current state, so one
// key press toggles it.
const uint8_t BUTTON_KEYS[] = {KEY_F10, KEY_F9, KEY_F8, KEY_F7};
const char *BUTTON_LABELS[] = {"Record", "Stream", "Scene 1", "Scene 2"};
// Record/Stream are start/stop toggles; Scene 1/2 are one-shot actions.
const bool BUTTON_IS_TOGGLE[] = {true, true, false, false};

bool lastReading[NUM_BUTTONS];
bool debouncedState[NUM_BUTTONS];
unsigned long lastChangeTime[NUM_BUTTONS];
// Tracks assumed on/off state for toggle buttons. This is a local guess,
// not read back from OBS, so it can drift out of sync if OBS is toggled
// some other way (its own UI, a different hotkey, a crash, etc).
bool toggleOn[NUM_BUTTONS];

void setup() {
  Serial.begin(115200);
  unsigned long serialWaitStart = millis();
  while (!Serial && (millis() - serialWaitStart) < 3000) {
    delay(10);
  }

  Serial.println("Stream Deck booting...");

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    lastReading[i] = digitalRead(BUTTON_PINS[i]);
    debouncedState[i] = lastReading[i];
    lastChangeTime[i] = 0;
    toggleOn[i] = false;
    Serial.printf("Button %d -> GPIO%d, initial read = %s\n",
                  i + 1, BUTTON_PINS[i], lastReading[i] ? "HIGH (idle)" : "LOW (pressed?)");
  }

  Keyboard.begin();
  USB.begin();
  Serial.println("USB HID keyboard ready");
}

void loop() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    bool reading = digitalRead(BUTTON_PINS[i]);

    if (reading != lastReading[i]) {
      lastChangeTime[i] = now;
    }

    if ((now - lastChangeTime[i]) > DEBOUNCE_MS && reading != debouncedState[i]) {
      debouncedState[i] = reading;
      Serial.printf("Button %d %s (GPIO%d)\n", i + 1,
                    debouncedState[i] == LOW ? "PRESSED" : "released",
                    BUTTON_PINS[i]);
      if (debouncedState[i] == LOW) {
        Keyboard.write(BUTTON_KEYS[i]);

        if (BUTTON_IS_TOGGLE[i]) {
          toggleOn[i] = !toggleOn[i];
          Serial.printf("Sent key for '%s' (%s)\n", BUTTON_LABELS[i],
                        toggleOn[i] ? "started" : "stopped");
        } else {
          Serial.printf("Sent key for '%s'\n", BUTTON_LABELS[i]);
        }
      }
    }

    lastReading[i] = reading;
  }
}

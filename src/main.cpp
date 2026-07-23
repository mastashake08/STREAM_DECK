#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <USB.h>
#include <USBHIDKeyboard.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
USBHIDKeyboard Keyboard;

const uint8_t BUTTON_PINS[] = {1, 2, 3, 4}; // GPIO1-GPIO4
const uint8_t NUM_BUTTONS = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);
const unsigned long DEBOUNCE_MS = 30;

// Bind each of these in OBS: Settings > Hotkeys.
// Record/Stream keys are shared between the Start and Stop hotkey slots -
// OBS only acts on whichever one is valid for the current state, so one
// key press toggles it.
const uint8_t BUTTON_KEYS[] = {KEY_F13, KEY_F14, KEY_F15, KEY_F16};
const char *BUTTON_LABELS[] = {"Record", "Stream", "Scene Up", "Scene Down"};

bool lastReading[NUM_BUTTONS];
bool debouncedState[NUM_BUTTONS];
unsigned long lastChangeTime[NUM_BUTTONS];

void showMessage(const String &line1, const String &line2 = "") {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 20);
  display.println(line1);
  if (line2.length() > 0) {
    display.setTextSize(1);
    display.println(line2);
  }
  display.display();
}

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
    Serial.printf("Button %d -> GPIO%d, initial read = %s\n",
                  i + 1, BUTTON_PINS[i], lastReading[i] ? "HIGH (idle)" : "LOW (pressed?)");
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 init FAILED - check wiring/address");
    for (;;) delay(1000);
  }
  Serial.println("SSD1306 init OK");

  Keyboard.begin();
  USB.begin();
  Serial.println("USB HID keyboard ready");

  showMessage("Stream Deck", "Ready");
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
        Serial.printf("Sent key for '%s'\n", BUTTON_LABELS[i]);
        showMessage(BUTTON_LABELS[i], "key sent");
      }
    }

    lastReading[i] = reading;
  }
}

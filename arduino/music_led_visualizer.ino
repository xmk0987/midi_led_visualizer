#include <FastLED.h>

constexpr uint8_t DATA_PIN = 6;
constexpr uint8_t NUM_LEDS = 78;

constexpr uint8_t FIRST_NOTE_NUMBER = 21;
constexpr uint8_t LAST_NOTE_NUMBER = 108;

constexpr uint8_t FIRST_LED_INDEX = 75;
constexpr uint8_t LAST_LED_INDEX = 2;

constexpr unsigned long LED_REFRESH_INTERVAL_MS = 5;

unsigned long lastLedRefresh = 0;

CRGB leds[NUM_LEDS];

bool activeNotes[128] = {false};
bool ledsChanged = false;

uint8_t activeNotesPerLed[NUM_LEDS] = {0};

uint8_t brightness = 50;
CRGB color = CRGB(0,0,0);

void updateLedBrightness(int brightness) {
  FastLED.setBrightness(brightness);
  FastLED.show();
}

void handleConfig(String message) {
  int firstComma = message.indexOf(',');
  int secondComma = message.indexOf(',', firstComma + 1);
  int thirdComma = message.indexOf(',', secondComma + 1);
  int fourthComma = message.indexOf(',', thirdComma + 1);

  if (
    firstComma == -1 ||
    secondComma == -1 ||
    thirdComma == -1 ||
    fourthComma == -1
  ) {
    Serial.println("ERROR,Invalid basic config format");
    return;
  }

  uint8_t red = message.substring(firstComma + 1, secondComma).toInt();
  uint8_t green = message.substring(secondComma + 1, thirdComma).toInt();
  uint8_t blue = message.substring(thirdComma + 1, fourthComma).toInt();
  brightness = message.substring(fourthComma + 1).toInt();

  color = CRGB(red, green, blue);
  updateLedBrightness(brightness);
  Serial.println("CONFIG_OK");
}

int getLedFromNote(int note) {
  int ledIndex = map(
    note,
    FIRST_NOTE_NUMBER,
    LAST_NOTE_NUMBER,
    FIRST_LED_INDEX,
    LAST_LED_INDEX
  );

  return ledIndex - 1;
}

void turnOnLed(uint8_t index) {
  leds[index] = color;
  ledsChanged = true;
}

void turnOffLed(uint8_t index) {
  leds[index] = CRGB::Black;
  ledsChanged = true;
}

bool isValidLedIndex(int ledIndex) {
  if (ledIndex < 0 || ledIndex >= NUM_LEDS) {
    Serial.println("ERROR,LED index out of range");
    return false;
  }

  return true;
}

void logMidiEvent(const char* event, int noteNumber, int velocity = -1) {
  Serial.print(event);
  Serial.print(",");
  Serial.print(noteNumber);

  if (velocity >= 0) {
    Serial.print(",");
    Serial.print(velocity);
  }

  Serial.println();
}

void handleKeyboardPress(String message) {
  int firstComma = message.indexOf(',');
  int secondComma = message.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) {
    Serial.println("ERROR,Invalid keyboard press format");
    return;
  }

  int noteNumber = message.substring(firstComma + 1, secondComma).toInt();

  if (noteNumber < 0 || noteNumber > 127) {
    Serial.println("ERROR,Invalid MIDI note");
    return;
  }

  logMidiEvent("PRESS", noteNumber);

  if (activeNotes[noteNumber]) {
    return;
  }

  int ledIndex = getLedFromNote(noteNumber);

  if (!isValidLedIndex(ledIndex)) {
    return;
  }

  activeNotes[noteNumber] = true;
  activeNotesPerLed[ledIndex]++;

  turnOnLed(ledIndex);
}

void handleKeyboardRelease(String message) {
  int firstComma = message.indexOf(',');

  if (firstComma == -1) {
    Serial.println("ERROR,Invalid keyboard release format");
    return;
  }

  int noteNumber = message.substring(firstComma + 1).toInt();

  if (noteNumber < 0 || noteNumber > 127) {
    Serial.println("ERROR,Invalid MIDI note");
    return;
  }

  logMidiEvent("RELEASE", noteNumber);

  if (!activeNotes[noteNumber]) {
    return;
  }

  int ledIndex = getLedFromNote(noteNumber);

  if (!isValidLedIndex(ledIndex)) {
    return;
  }

  activeNotes[noteNumber] = false;

  if (activeNotesPerLed[ledIndex] > 0) {
    activeNotesPerLed[ledIndex]--;
  }

  if (activeNotesPerLed[ledIndex] == 0) {
    turnOffLed(ledIndex);
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  FastLED.clear();
  FastLED.show();

  Serial.begin(115200);
}

void loop() {
  while (Serial.available()) {
    String message = Serial.readStringUntil('\n');
    message.trim();

    if (message.startsWith("CONFIG,")) {
      handleConfig(message);
    } else if (message.startsWith("PRESS,")) {
      handleKeyboardPress(message);
    } else if (message.startsWith("RELEASE,")) {
      handleKeyboardRelease(message);
    }
  }

  unsigned long now = millis();

  if (
    ledsChanged &&
    now - lastLedRefresh >= LED_REFRESH_INTERVAL_MS
  ) {
    FastLED.show();
    ledsChanged = false;
    lastLedRefresh = now;
  }
}
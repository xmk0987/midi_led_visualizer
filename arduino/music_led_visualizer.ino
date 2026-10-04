#include <FastLED.h>

constexpr uint8_t DATA_PIN = 6;
constexpr uint8_t NUM_LEDS = 78;

constexpr uint8_t MIDI_NOTE_COUNT = 128;

constexpr uint8_t MY_KEYBOARD_FIRST_MIDI_NOTE = 21;
constexpr uint8_t MY_KEYBOARD_LAST_MIDI_NOTE = 108;

constexpr uint8_t FIRST_LED_INDEX = 75;
constexpr uint8_t LAST_LED_INDEX = 2;

constexpr int LED_STRIP_OFFSET = -1;


CRGB leds[NUM_LEDS];

bool ledsChanged = false;

bool isPedalOn = false;
bool activeNotes[MIDI_NOTE_COUNT] = {false};
bool sustainedNotes[MIDI_NOTE_COUNT] = {false};

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
    MY_KEYBOARD_FIRST_MIDI_NOTE,
    MY_KEYBOARD_LAST_MIDI_NOTE,
    FIRST_LED_INDEX,
    LAST_LED_INDEX
  );

  return ledIndex + LED_STRIP_OFFSET;
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

  //logMidiEvent("PRESS", noteNumber);

  if (activeNotes[noteNumber]) {
    return;
  }

  int ledIndex = getLedFromNote(noteNumber);

  if (!isValidLedIndex(ledIndex)) {
    return;
  }

  if (sustainedNotes[noteNumber]) {
    sustainedNotes[noteNumber] = false;
    activeNotes[noteNumber] = true;
    turnOnLed(ledIndex);
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

  //logMidiEvent("RELEASE", noteNumber);

  if (!activeNotes[noteNumber]) {
    return;
  }

  int ledIndex = getLedFromNote(noteNumber);

  if (!isValidLedIndex(ledIndex)) {
    return;
  }

  activeNotes[noteNumber] = false;

  if (isPedalOn) {
    sustainedNotes[noteNumber] = true;
    return;
  }

  if (activeNotesPerLed[ledIndex] > 0) {
    activeNotesPerLed[ledIndex]--;
  }

  if (activeNotesPerLed[ledIndex] == 0) {
    turnOffLed(ledIndex);
  }
}

void handlePedalOn(String message) {
  isPedalOn = true;
}

void handlePedalOff(String message) {
  isPedalOn = false;

  for (int note = 0; note < MIDI_NOTE_COUNT; note++) {
    if (!sustainedNotes[note]) {
      continue;
    }

    sustainedNotes[note] = false;

    if (activeNotes[note]) {
      continue;
    }

    int ledIndex = getLedFromNote(note);

    if (!isValidLedIndex(ledIndex)) {
      continue;
    }

    if (activeNotesPerLed[ledIndex] > 0) {
      activeNotesPerLed[ledIndex]--;
    }

    if (activeNotesPerLed[ledIndex] == 0) {
      turnOffLed(ledIndex);
    }
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);

  FastLED.clear();
  FastLED.show();

  Serial.begin(115200);
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  String message = Serial.readStringUntil('\n');
  message.trim();

  bool shouldAcknowledge = false;

  if (message.startsWith("CONFIG,")) {
    handleConfig(message);
    return;
  }

  if (message.startsWith("PRESS,")) {
    handleKeyboardPress(message);
    shouldAcknowledge = true;

  } else if (message.startsWith("RELEASE,")) {
    handleKeyboardRelease(message);
    shouldAcknowledge = true;

  } else if (message.startsWith("PEDAL_ON")) {
    handlePedalOn(message);
    shouldAcknowledge = true;

  } else if (message.startsWith("PEDAL_OFF")) {
    handlePedalOff(message);
    shouldAcknowledge = true;
  }

  if (ledsChanged) {
    FastLED.show();
    ledsChanged = false;
  }

  if (shouldAcknowledge) {
    Serial.println("OK");
  }
}
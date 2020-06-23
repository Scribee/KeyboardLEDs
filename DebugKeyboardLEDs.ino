#include <MIDI.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

MIDI_CREATE_DEFAULT_INSTANCE();
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Number of LEDs over the piano keys
#define NUM_KEYBOARD_LEDS 73 // With a 60 LED/meter strip and a full size keyboard (122cm), there are 73

// Arduino pin controlling the LED strip
#define DATA_PIN 9

// Index of the LED that lines up with the first note on the piano
#define FIRST_LED 30

// MIDI pitch of an A0, the lowest (first) note on a piano
#define FIRST_KEY_PITCH 21

// Number of keys on the keyboard
#define NUM_KEYS 88 // A full size keyboard has 88 keys

// Define the array of leds
CRGB leds[NUM_KEYBOARD_LEDS + FIRST_LED];
// Define the array of colors
CRGB colors[NUM_KEYBOARD_LEDS];

String data;
byte row = 0;
int keysDown = 0;

void onNoteOn(byte channel, byte pitch, byte velocity) {
  keysDown++;

  int index = map(pitch - FIRST_KEY_PITCH, 0, NUM_KEYS - 1, FIRST_LED, NUM_KEYBOARD_LEDS + FIRST_LED - 1);
  leds[index] = colors[index - FIRST_LED];
  FastLED.show();

  data = String("P: ") + pitch + ", V: " + velocity;

  lcd.setCursor(0, row);
  lcd.print("                    ");
  lcd.setCursor(0, row);
  lcd.print(data);

  if (row == 2) {
    row = 0;
  }
  else {
    row++;
  }

  lcd.setCursor(18, 3);
  lcd.print(keysDown);
}

void onNoteOff(byte channel, byte pitch, byte velocity) {
  keysDown--;

  int index = map(pitch - FIRST_KEY_PITCH, 0, NUM_KEYS - 1, FIRST_LED, NUM_KEYBOARD_LEDS + FIRST_LED - 1);
  leds[index] = CRGB::Black;
  FastLED.show();

  lcd.setCursor(18, 3);
  lcd.print(keysDown);
}

void setup() {
  delay(3000);

  MIDI.setHandleNoteOn(onNoteOn); // Specify function to call on reception of a NoteOn command
  MIDI.setHandleNoteOff(onNoteOff); // Do the same for NoteOffs
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initiate MIDI communications, listen to all channels

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_KEYBOARD_LEDS + FIRST_LED).setCorrection(TypicalSMD5050); // Set up the LED strip

  FastLED.setBrightness(50);
  FastLED.setTemperature(Candle);

  fill_rainbow(colors, NUM_KEYBOARD_LEDS, 42, 4);

  lcd.begin(20, 4);
}

void loop() {
  MIDI.read();
}

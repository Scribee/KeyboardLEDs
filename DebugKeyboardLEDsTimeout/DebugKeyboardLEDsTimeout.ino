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

/*
 Amount of time in ms to wait after the last MIDI message has been received before updating the LED strip.
 The minimum time in which messages can be processed with my setup is 33ms.
 My glissandos usually produce noteOn messages with 70-90ms between them, while fast grace notes or trills can produce messages with even shorter intervals.
 My regular playing does not ever seem to cause MIDI messages to be sent more frequently than every 100ms.
 
 The purpose of this is to try to separate chords (multiple notes played at basically the same time) from individual notes spread out over time.
 This way the expensive FastLED.show() doesn't have to be run between processing each note in a chord that should be processed at once.
 Without limiting this, the arduino can sometimes 'miss' subsequent MIDI messages from notes played in very quick succession while busy updating the LEDs for the first received message. 
*/
#define LED_UPDATE_DELAY 43

// Define array of leds to control the led strip
CRGB leds[NUM_KEYBOARD_LEDS + FIRST_LED];
// Define an array of colors to use for the led strip
CRGB colors[NUM_KEYBOARD_LEDS];

uint8_t row = 0;
uint8_t keysDown = 0;
bool ledsModified = false;
unsigned long lastMessageTime = 0;

void setup() {
  delay(3000);

  // Set up MIDI handler
  MIDI.setHandleNoteOn(onNoteOn); // Specify function to call on reception of a NoteOn command
  MIDI.setHandleNoteOff(onNoteOff); // Do the same for NoteOffs
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initiate MIDI communications, listen to all channels

  // Set up the LED strip
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_KEYBOARD_LEDS + FIRST_LED);
  FastLED.setCorrection(TypicalSMD5050); 
  FastLED.setBrightness(50);

  fill_rainbow(colors, NUM_KEYBOARD_LEDS, 42, 4);

  // Set up lcd for debugging
  lcd.begin(20, 4);
}

void loop() {
  // Read MIDI data from piano
  MIDI.read();

  // If leds have been changed and it has been longer than LED_UPDATE_DELAY since the last MIDI message was received, update the led strip.
  if (ledsModified && millis() - lastMessageTime > LED_UPDATE_DELAY) {
    FastLED.show();
    ledsModified = false;
    lcd.setCursor(0, 3);
    lcd.print("Updated ");
  }
}

void onNoteOn(byte channel, byte pitch, byte velocity) {
  keysDown++;

  uint8_t index = map(pitch - FIRST_KEY_PITCH, 0, NUM_KEYS - 1, FIRST_LED, NUM_KEYBOARD_LEDS + FIRST_LED - 1);
  leds[index] = colors[index - FIRST_LED];
  ledsModified = true;

  unsigned long messageTime = millis();
  unsigned long noteOnDiff = messageTime - lastMessageTime;
  lastMessageTime = messageTime;
  
  String data = String("P: ") + pitch + ", T: " + noteOnDiff;

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

  lcd.setCursor(0, 3);
  lcd.print("Modified");
  lcd.setCursor(18, 3);
  lcd.print(keysDown);
}

void onNoteOff(byte channel, byte pitch, byte velocity) {
  keysDown--;

  uint8_t index = map(pitch - FIRST_KEY_PITCH, 0, NUM_KEYS - 1, FIRST_LED, NUM_KEYBOARD_LEDS + FIRST_LED - 1);
  leds[index] = CRGB::Black;
  ledsModified = true;

  lastMessageTime = millis();
  
  lcd.setCursor(0, 3);
  lcd.print("Modified");  
  lcd.setCursor(18, 3);
  lcd.print(keysDown);
}

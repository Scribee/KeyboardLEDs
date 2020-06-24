#include <MIDI.h>
#include <FastLED.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// Arduino pin controlling the LED strip
#define DATA_PIN 9

// Number of LEDs over the piano keys
#define NUM_KEYBOARD_LEDS 64 // With a 60 LED/meter strip and a full size keyboard (122cm), there are 73

// Index of the LED that lines up with the first note on the piano
#define FIRST_LED 30

// MIDI pitch of A0, the lowest (first) note on a full size keyboard
#define FIRST_KEY_PITCH 21

// Number of keys on the keyboard
#define NUM_KEYS 88 // A full size keyboard has 88 keys

/*
 Amount of time in ms to wait after the last MIDI message has been received before updating the LED strip.
 The minimum time in which messages can be processed with my setup is ~25ms.
 My glissandos usually produce noteOn messages with 70-90ms between them, while fast grace notes or trills can produce messages with even shorter intervals.
 My regular playing does not ever seem to cause MIDI messages to be sent more frequently than every 100ms.
 
 The purpose of this is to try to separate chords (multiple notes played at basically the same time) from individual notes spread out over time.
 This way the expensive FastLED.show() doesn't have to be run between processing each note in a chord that should be processed at once.
 Without limiting this, the arduino can sometimes 'miss' subsequent MIDI messages from notes played in very quick succession while busy updating the LEDs for the first received message. 
*/
#define LED_UPDATE_DELAY 29

CRGB leds[NUM_KEYBOARD_LEDS + FIRST_LED]; // Array to store the color of each LED in the strip
CRGB colors[NUM_KEYBOARD_LEDS]; // Array of colors to be used in leds*

bool ledsModified = false;
unsigned long lastMessageTime = 0;

void setup() {
  delay(3000);

  // Set up MIDI handler
  MIDI.setHandleNoteOn(onNoteOn); // Specify function to call on reception of a NoteOn command
  MIDI.setHandleNoteOff(onNoteOff); // Do the same for NoteOffs
  MIDI.begin(MIDI_CHANNEL_OMNI); // Initiate MIDI communication, listen to all channels

  // Set up the LED strip
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_KEYBOARD_LEDS + FIRST_LED); // Set up strip with the array to hold the color of each LED and the total number
  FastLED.setCorrection(TypicalSMD5050); // Set color correction to remove greenish tint
  FastLED.setBrightness(50); // Up to 255

  // Fill the colors array with a rainbow
  fill_rainbow(colors, NUM_KEYBOARD_LEDS, 42, 9);
}

void loop() {
  MIDI.read(); // Read MIDI data from piano

  // If leds have been changed and it has been longer than LED_UPDATE_DELAY since the last MIDI message was received, update the led strip
  if (ledsModified && millis() - lastMessageTime > LED_UPDATE_DELAY) {
    FastLED.show(); // 'Push' the new leds* to the LED strip, updating which LEDs are on
    ledsModified = false;
  }
}

/*
 * Handler for NoteOn MIDI messages.
 */
void onNoteOn(byte channel, byte pitch, byte velocity) {
  setLED(pitch - FIRST_KEY_PITCH, true);
}

/*
 * Handler for NoteOff MIDI messages.
 */
void onNoteOff(byte channel, byte pitch, byte velocity) {
  setLED(pitch - FIRST_KEY_PITCH, false);
}

/*
 * Using the index of the pressed or released key, the index of the closest LED is calculated and given a color or set to black.
 */
void setLED(byte keyIndex, bool turningOn) {
  byte index = map(keyIndex, 0, NUM_KEYS - 1, FIRST_LED, NUM_KEYBOARD_LEDS + FIRST_LED - 1); // Calculate index of LED that lines up with key
  
  if (turningOn) {
    leds[index] = colors[index - FIRST_LED]; // 'Turn on' that LED by giving it a color from the rainbow array
  }
  else {
    leds[index] = CRGB::Black; // 'Turn off' the LED by setting its color to black
  }

  ledsModified = true; // Remember that the LEDs have been changed since the last update
  lastMessageTime = millis(); // Remember the time this keypress was processed
}

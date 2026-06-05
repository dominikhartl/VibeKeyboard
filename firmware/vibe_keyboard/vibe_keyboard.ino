/*
 * VibeKeyboard — 3-button macro pad for vibe coding
 * -------------------------------------------------
 * Target board : Arduino Pro Micro (ATmega32U4, 5V / 16MHz) — or any ATmega32U4
 *                board such as the Arduino Leonardo. Native USB-HID required.
 *
 * Buttons (Cherry MX, wired switch -> pin and switch -> GND, read active-low):
 *   - Voice : push-to-talk. Holds VOICE_KEY while pressed, releases on release.
 *   - Accept: taps ACCEPT_KEY once per press.
 *   - Reject: taps REJECT_KEY once per press.
 *
 * LEDs (one per button, pin -> 220R -> LED+ -> LED- -> GND):
 *   - On boot, STARTUP_PATTERN plays once.
 *   - Afterwards each LED is OFF until its button is pressed (mirrors the live
 *     pressed state), so the Voice LED stays lit for the whole hold.
 *
 * No external libraries beyond the built-in "Keyboard". No diodes, no key matrix,
 * no external button resistors (internal pull-ups are used).
 *
 * See docs/CONFIGURATION.md for how to remap keys and edit the boot pattern.
 *
 * License: MIT
 */

#include <Keyboard.h>

// ===========================================================================
//  CONFIGURATION  — edit this block to customize the keyboard
// ===========================================================================

// ---- Switch pins (each switch connects this pin to GND) ----
const uint8_t PIN_ACCEPT = 2;
const uint8_t PIN_REJECT = 3;
const uint8_t PIN_VOICE  = 4;

// ---- LED pins (one status LED per button) ----
const uint8_t PIN_ACCEPT_LED = 5;
const uint8_t PIN_REJECT_LED = 6;
const uint8_t PIN_VOICE_LED  = 7;

// LEDs wired pin -> resistor -> LED+ -> LED- -> GND are "active high" (HIGH = on).
// Set to false if you instead wired them +5V -> resistor -> LED+ -> LED- -> pin.
const bool LED_ACTIVE_HIGH = true;

// ---- Key mappings (edit to match your tools) ----
// Defaults target Claude Code in a terminal:
//   Accept = Enter (confirm the highlighted action)
//   Reject = Esc   (cancel the current action/prompt)
//   Voice  = F13   (unused on macOS; bind your dictation app's push-to-talk to F13)
#define ACCEPT_KEY KEY_RETURN
#define REJECT_KEY KEY_ESC
#define VOICE_KEY  KEY_F13
// Alternative if KEY_F13 is unavailable on your core, or your voice app wants a
// held modifier instead of a function key:
//   #define VOICE_KEY KEY_RIGHT_ALT
// (KEY_F13..KEY_F24 require a reasonably recent Arduino AVR core / Keyboard lib.)

// ---- Behavior ----
// false = push-to-talk: hold the button to hold VOICE_KEY (recommended).
// true  = toggle: tap the button to press+hold VOICE_KEY, tap again to release.
const bool VOICE_TOGGLE = false;

// Software debounce window in milliseconds.
const unsigned long DEBOUNCE_MS = 8;

// ---- Customizable boot pattern ----
// Plays ONCE at power-up. Each frame sets the three LEDs and a duration (ms).
// Edit freely: add/remove frames, change timings. A final {0,0,0,0} ends dark.
struct LedFrame { bool a; bool r; bool v; uint16_t ms; };
const LedFrame STARTUP_PATTERN[] = {
  {1, 0, 0, 120},   // chase: accept
  {0, 1, 0, 120},   // chase: reject
  {0, 0, 1, 120},   // chase: voice
  {1, 1, 1, 150},   // double flash
  {0, 0, 0, 150},
  {1, 1, 1, 150},
  {0, 0, 0, 0},     // end dark
};
const uint8_t STARTUP_FRAMES = sizeof(STARTUP_PATTERN) / sizeof(STARTUP_PATTERN[0]);

// ===========================================================================
//  IMPLEMENTATION  — no need to edit below this line for normal customization
// ===========================================================================

// A single button channel: its switch pin, its LED pin, the HID key it sends,
// whether it is the push-to-talk (held) button, and its debounce bookkeeping.
struct Button {
  uint8_t       switchPin;
  uint8_t       ledPin;
  uint8_t       key;
  bool          isHold;          // true = press/release (PTT); false = tap on press
  bool          stableState;     // debounced "is pressed" state
  bool          lastReading;     // last raw reading
  bool          keyHeld;         // for hold/toggle: is VOICE_KEY currently pressed?
  unsigned long lastChangeMs;    // when lastReading last changed
};

Button buttons[] = {
  { PIN_ACCEPT, PIN_ACCEPT_LED, ACCEPT_KEY, false, false, false, false, 0 },
  { PIN_REJECT, PIN_REJECT_LED, REJECT_KEY, false, false, false, false, 0 },
  { PIN_VOICE,  PIN_VOICE_LED,  VOICE_KEY,  true,  false, false, false, 0 },
};
const uint8_t NUM_BUTTONS = sizeof(buttons) / sizeof(buttons[0]);

// Drive an LED honoring LED_ACTIVE_HIGH.
void setLed(uint8_t pin, bool on) {
  digitalWrite(pin, (on == LED_ACTIVE_HIGH) ? HIGH : LOW);
}

// Play the configurable boot animation once.
void playStartupPattern() {
  for (uint8_t i = 0; i < STARTUP_FRAMES; i++) {
    setLed(PIN_ACCEPT_LED, STARTUP_PATTERN[i].a);
    setLed(PIN_REJECT_LED, STARTUP_PATTERN[i].r);
    setLed(PIN_VOICE_LED,  STARTUP_PATTERN[i].v);
    if (STARTUP_PATTERN[i].ms) delay(STARTUP_PATTERN[i].ms);
  }
  setLed(PIN_ACCEPT_LED, false);
  setLed(PIN_REJECT_LED, false);
  setLed(PIN_VOICE_LED,  false);
}

void setup() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].switchPin, INPUT_PULLUP);  // switch closed -> reads LOW
    pinMode(buttons[i].ledPin, OUTPUT);
    setLed(buttons[i].ledPin, false);
  }
  Keyboard.begin();
  playStartupPattern();
}

void loop() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    Button &b = buttons[i];

    // Active-low: pressed when the pin reads LOW.
    bool reading = (digitalRead(b.switchPin) == LOW);

    // Debounce: only accept a new state after it has been stable for DEBOUNCE_MS.
    if (reading != b.lastReading) {
      b.lastReading = reading;
      b.lastChangeMs = now;
    }

    if ((now - b.lastChangeMs) >= DEBOUNCE_MS && reading != b.stableState) {
      b.stableState = reading;  // debounced edge

      if (b.isHold) {
        // Voice button.
        if (VOICE_TOGGLE) {
          // Toggle mode: flip on each press edge only.
          if (b.stableState) {
            if (b.keyHeld) { Keyboard.release(b.key); b.keyHeld = false; }
            else           { Keyboard.press(b.key);   b.keyHeld = true;  }
          }
        } else {
          // Push-to-talk: hold while pressed.
          if (b.stableState) { Keyboard.press(b.key);   b.keyHeld = true;  }
          else               { Keyboard.release(b.key); b.keyHeld = false; }
        }
      } else {
        // Accept / Reject: tap once on the press edge.
        if (b.stableState) Keyboard.write(b.key);
      }
    }

    // LED feedback: off until pressed.
    // - Hold/toggle button: follow whether its key is currently held.
    // - Tap buttons: follow the live pressed state (blinks during the tap).
    bool ledOn = b.isHold ? b.keyHeld : b.stableState;
    setLed(b.ledPin, ledOn);
  }
}

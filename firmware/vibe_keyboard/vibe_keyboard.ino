/*
 * VibeKeyboard — 3-button macro pad for vibe coding
 * -------------------------------------------------
 * Board : Arduino Pro Micro (ATmega32U4, 5V / 16MHz) — native USB-HID.
 *
 * Buttons (Cherry MX, switch -> pin and switch -> GND, read active-low):
 *   - Voice : push-to-talk. Holds VOICE_KEY (F13) while pressed.
 *   - Accept: taps ACCEPT_KEY (Enter).
 *   - Reject: taps REJECT_KEY (Esc).
 *   - CHORD Voice+Accept (pressed together): types CHORD_TEXT + Enter
 *     (default "/voice") to start Claude Code's voice dictation.
 *
 * LEDs (one per button, on PWM pins) run a continuous dimmable "wave"; pressing
 * a button lights its LED full.
 *
 * Needs only the built-in "Keyboard" library. License: MIT.
 */

#include <Keyboard.h>
#include <math.h>

// ===========================================================================
//  CONFIGURATION
// ===========================================================================

// ---- Switch pins (digital input; the analog header keeps special pins free) ----
const uint8_t PIN_ACCEPT = A0;
const uint8_t PIN_REJECT = A1;
const uint8_t PIN_VOICE  = A2;

// ---- LED pins — MUST be PWM (3,5,6,9,10). Ordered LEFT -> RIGHT = wave direction ----
const uint8_t PIN_VOICE_LED  = 5;   // D5  leftmost
const uint8_t PIN_ACCEPT_LED = 6;   // D6  middle
const uint8_t PIN_REJECT_LED = 9;   // D9  rightmost
const bool    LED_ACTIVE_HIGH = true;   // false if LEDs wired to +5V instead of GND

// Per-LED brightness trim (1.0 = unchanged; >1 brightens, clamped at full).
const float LED_GAIN_VOICE  = 1.0f;
const float LED_GAIN_ACCEPT = 1.5f;     // Accept LED a bit brighter
const float LED_GAIN_REJECT = 1.0f;

// ---- Key mappings ----
#define ACCEPT_KEY KEY_RETURN   // Claude Code: confirm
#define REJECT_KEY KEY_ESC      // Claude Code: cancel
#define VOICE_KEY  KEY_F13      // hold-to-talk; bind your voice tool's PTT to F13
const bool          VOICE_TOGGLE = false;  // false = push-to-talk; true = tap on/off
const unsigned long DEBOUNCE_MS  = 8;

// ---- Chord: two buttons pressed together type a command ----
// Voice + Accept -> "/voice" + Enter, to start Claude Code dictation.
const bool          CHORD_ENABLE    = true;
const uint8_t       CHORD_BTN_A     = 0;        // index in buttons[] : Voice
const uint8_t       CHORD_BTN_B     = 1;        // index in buttons[] : Accept
const char* const   CHORD_TEXT      = "/voice"; // typed, then Enter
const unsigned long CHORD_WINDOW_MS = 50;       // how close together the two presses must land
                                                // (also the resolve delay for single taps)

// ---- Ambient wave animation ----
const bool          WAVE_ENABLE    = true;
const unsigned long WAVE_PERIOD_MS = 1800;   // one sweep, lower = faster
const bool          WAVE_BOUNCE    = true;   // true = back-and-forth; false = one-way loop
const bool          WAVE_REVERSE   = false;  // flip the starting direction
const float         WAVE_WIDTH     = 1.1f;   // peak softness (bigger = softer)
const uint8_t       WAVE_MIN       = 4;      // dim floor (0-255)
const uint8_t       WAVE_MAX       = 200;    // peak brightness (0-255)
const uint8_t       PRESS_BRIGHT   = 255;    // brightness while pressed
const bool          GAMMA_CORRECT  = true;   // perceptual (smoother) fade

// ===========================================================================
//  IMPLEMENTATION
// ===========================================================================

struct Button {
  uint8_t       sw, led, key;
  bool          isHold;        // true = press/release (PTT); false = tap
  float         gain;
  // runtime state (zero-initialized):
  bool          rawLast, down, pending, held, consumed;
  unsigned long tRaw, tPress;
};

// Listed LEFT -> RIGHT so the wave flows across the physical layout in this order.
Button buttons[] = {
  { PIN_VOICE,  PIN_VOICE_LED,  VOICE_KEY,  true,  LED_GAIN_VOICE  },  // [0] left
  { PIN_ACCEPT, PIN_ACCEPT_LED, ACCEPT_KEY, false, LED_GAIN_ACCEPT },  // [1] mid
  { PIN_REJECT, PIN_REJECT_LED, REJECT_KEY, false, LED_GAIN_REJECT },  // [2] right
};
const uint8_t N = sizeof(buttons) / sizeof(buttons[0]);
bool chordEngaged = false;

uint8_t gammaCorrect(uint8_t b) {
  return GAMMA_CORRECT ? (uint8_t)((uint16_t)b * b / 255) : b;
}

void writeLed(uint8_t pin, uint8_t level, float gain = 1.0f) {
  uint16_t v = (uint16_t)(gammaCorrect(level) * gain);
  if (v > 255) v = 255;
  analogWrite(pin, LED_ACTIVE_HIGH ? (uint8_t)v : (uint8_t)(255 - v));
}

uint8_t waveLevel(uint8_t i, unsigned long now) {
  float peak;
  if (WAVE_BOUNCE) {
    // Turn around exactly AT the edge LEDs (no overshoot -> no double blink).
    float p = (now % (2UL * WAVE_PERIOD_MS)) / (float)WAVE_PERIOD_MS;  // 0..2
    float t = (p < 1.0f) ? p : (2.0f - p);                            // 0..1..0
    peak = t * (N - 1);
  } else {
    float t = (now % WAVE_PERIOD_MS) / (float)WAVE_PERIOD_MS;          // 0..1
    peak = -WAVE_WIDTH + t * ((N - 1) + 2.0f * WAVE_WIDTH);
  }
  float pos = WAVE_REVERSE ? (N - 1 - i) : i;
  float d   = fabsf(pos - peak);
  float g   = (d < WAVE_WIDTH) ? 0.5f * (1.0f + cosf(PI * d / WAVE_WIDTH)) : 0.0f;
  return WAVE_MIN + (uint8_t)((WAVE_MAX - WAVE_MIN) * g);
}

// Commit a button's press as a single (non-chord) action.
void commitPress(uint8_t i) {
  Button &b = buttons[i];
  b.pending = false;
  if (b.isHold) {
    if (VOICE_TOGGLE) {
      if (b.held) { Keyboard.release(b.key); b.held = false; }
      else        { Keyboard.press(b.key);   b.held = true;  }
    } else        { Keyboard.press(b.key);   b.held = true;  }   // start PTT hold
  } else {
    Keyboard.write(b.key);                                       // tap
  }
}

// Apply a button's release action.
void handleRelease(uint8_t i) {
  Button &b = buttons[i];
  if (b.isHold && !VOICE_TOGGLE && b.held) { Keyboard.release(b.key); b.held = false; }
  // toggle-hold: key stays toggled on release; taps: nothing to do
}

void setup() {
  for (uint8_t i = 0; i < N; i++) {
    pinMode(buttons[i].sw, INPUT_PULLUP);
    pinMode(buttons[i].led, OUTPUT);
  }
  Keyboard.begin();
  // Boot self-test: fade all LEDs up then down so you can confirm the wiring.
  for (int v = 0; v <= 255; v += 5) { for (uint8_t i = 0; i < N; i++) writeLed(buttons[i].led, v); delay(4); }
  for (int v = 255; v >= 0; v -= 5) { for (uint8_t i = 0; i < N; i++) writeLed(buttons[i].led, v); delay(4); }
}

void loop() {
  unsigned long now = millis();
  unsigned long window = CHORD_ENABLE ? CHORD_WINDOW_MS : 0;

  // 1) Debounce each switch; turn debounced edges into press/release handling.
  for (uint8_t i = 0; i < N; i++) {
    Button &b = buttons[i];
    bool raw = (digitalRead(b.sw) == LOW);
    if (raw != b.rawLast) { b.rawLast = raw; b.tRaw = now; }
    if ((now - b.tRaw) >= DEBOUNCE_MS && raw != b.down) {
      b.down = raw;
      if (b.down) {                          // press edge -> wait to resolve chord vs single
        b.pending = true; b.tPress = now; b.consumed = false;
      } else {                               // release edge
        if (b.consumed)      { b.consumed = false; }            // was part of a chord
        else if (b.pending)  { commitPress(i); handleRelease(i); } // released within the window
        else                 { handleRelease(i); }             // normal release of a held press
      }
    }
  }

  // 2) Chord: both members pressed together (still pending) -> fire once.
  if (CHORD_ENABLE && !chordEngaged &&
      buttons[CHORD_BTN_A].down && buttons[CHORD_BTN_B].down &&
      buttons[CHORD_BTN_A].pending && buttons[CHORD_BTN_B].pending) {
    buttons[CHORD_BTN_A].pending = false; buttons[CHORD_BTN_A].consumed = true;
    buttons[CHORD_BTN_B].pending = false; buttons[CHORD_BTN_B].consumed = true;
    chordEngaged = true;
    Keyboard.print(CHORD_TEXT);
    Keyboard.write(KEY_RETURN);
  }

  // 3) Resolve any pending single press whose window has elapsed.
  for (uint8_t i = 0; i < N; i++)
    if (buttons[i].pending && (now - buttons[i].tPress) >= window) commitPress(i);

  // 4) Re-arm the chord once both members are released.
  if (chordEngaged && !buttons[CHORD_BTN_A].down && !buttons[CHORD_BTN_B].down)
    chordEngaged = false;

  // 5) LEDs: full while pressed, otherwise the ambient wave.
  for (uint8_t i = 0; i < N; i++) {
    uint8_t level = buttons[i].down ? PRESS_BRIGHT : (WAVE_ENABLE ? waveLevel(i, now) : 0);
    writeLed(buttons[i].led, level, buttons[i].gain);
  }
}

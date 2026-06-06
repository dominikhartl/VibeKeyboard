/*
 * VibeKeyboard — 3-button macro pad for vibe coding
 * -------------------------------------------------
 * Board : Arduino Pro Micro (ATmega32U4, 5V / 16MHz) — native USB-HID.
 *
 * Buttons (Cherry MX, switch -> pin and switch -> GND, read active-low):
 *   - Voice : push-to-talk. Holds VOICE_KEY while pressed.
 *   - Accept: taps ACCEPT_KEY once per press.
 *   - Reject: taps REJECT_KEY once per press.
 *
 * LEDs (one per button, on PWM pins so they DIM):
 *   - A continuous "wave" animation flows across the three LEDs (and therefore
 *     across the backlit logo) using hardware PWM.
 *   - Pressing a button drives its LED to full for feedback, then the wave resumes.
 *
 * Pin choice is future-proofed (see docs/WIRING.md): switches sit on the analog
 * header (digital input only) so the I2C, SPI, UART and spare PWM pins stay free.
 *
 * Needs only the built-in "Keyboard" library. License: MIT.
 */

#include <Keyboard.h>
#include <math.h>

// ===========================================================================
//  CONFIGURATION
// ===========================================================================

// ---- Switch pins (any digital pin; the analog header keeps special pins free) ----
const uint8_t PIN_ACCEPT = A0;
const uint8_t PIN_REJECT = A1;
const uint8_t PIN_VOICE  = A2;

// ---- LED pins — MUST be PWM-capable for dimming: 3, 5, 6, 9, 10 on the Pro Micro ----
// Ordered to match the physical layout LEFT -> RIGHT (Voice, Accept, Reject), which is
// also the direction the wave flows.
const uint8_t PIN_VOICE_LED  = 5;   // D5  (PWM Timer3) — leftmost
const uint8_t PIN_ACCEPT_LED = 6;   // D6  (PWM Timer4) — middle
const uint8_t PIN_REJECT_LED = 9;   // D9  (PWM Timer1) — rightmost
const bool    LED_ACTIVE_HIGH = true;   // false if LEDs are wired to +5V instead of GND

// ---- Key mappings (edit to match your tools) ----
#define ACCEPT_KEY KEY_RETURN   // Claude Code: confirm
#define REJECT_KEY KEY_ESC      // Claude Code: cancel
#define VOICE_KEY  KEY_F13      // hold-to-talk; bind your voice app's PTT to F13

// ---- Behavior ----
const bool          VOICE_TOGGLE = false;  // false = push-to-talk; true = tap toggle
const unsigned long DEBOUNCE_MS  = 8;

// ---- Ambient wave animation (LEDs dim via PWM) ----
const bool          WAVE_ENABLE    = true;   // continuous wave across the logo
const unsigned long WAVE_PERIOD_MS = 1800;   // time for one sweep, lower = faster
const bool          WAVE_REVERSE   = false;  // flip the flow direction
const float         WAVE_WIDTH     = 1.1f;   // peak softness in LED units (bigger = softer)
const uint8_t       WAVE_MIN       = 4;      // dim floor brightness (0-255)
const uint8_t       WAVE_MAX       = 200;    // peak brightness (0-255)
const uint8_t       PRESS_BRIGHT   = 255;    // brightness while a button is pressed
const bool          GAMMA_CORRECT  = true;   // perceptual fade (smoother to the eye)

// ===========================================================================
//  IMPLEMENTATION
// ===========================================================================

struct Button {
  uint8_t       sw, led, key;
  bool          isHold;       // true = press/release (PTT); false = tap
  bool          stable, lastRead, keyHeld;
  unsigned long lastChange;
};

// Listed LEFT -> RIGHT so the wave flows across the physical layout in this order.
Button buttons[] = {
  { PIN_VOICE,  PIN_VOICE_LED,  VOICE_KEY,  true,  false, false, false, 0 },   // leftmost
  { PIN_ACCEPT, PIN_ACCEPT_LED, ACCEPT_KEY, false, false, false, false, 0 },   // middle
  { PIN_REJECT, PIN_REJECT_LED, REJECT_KEY, false, false, false, false, 0 },   // rightmost
};
const uint8_t N = sizeof(buttons) / sizeof(buttons[0]);

uint8_t gammaCorrect(uint8_t b) {
  return GAMMA_CORRECT ? (uint8_t)((uint16_t)b * b / 255) : b;   // ~gamma 2.0
}

void writeLed(uint8_t pin, uint8_t level) {
  level = gammaCorrect(level);
  analogWrite(pin, LED_ACTIVE_HIGH ? level : 255 - level);
}

// Brightness of LED i for a wave whose peak travels across the LEDs over time.
uint8_t waveLevel(uint8_t i, unsigned long now) {
  float phase = (now % WAVE_PERIOD_MS) / (float)WAVE_PERIOD_MS;     // 0..1
  float span  = (N - 1) + 2.0f * WAVE_WIDTH;
  float peak  = -WAVE_WIDTH + phase * span;                        // sweeps in then out
  float pos   = WAVE_REVERSE ? (N - 1 - i) : i;
  float d     = fabsf(pos - peak);
  float g     = (d < WAVE_WIDTH) ? 0.5f * (1.0f + cosf(PI * d / WAVE_WIDTH)) : 0.0f;
  return WAVE_MIN + (uint8_t)((WAVE_MAX - WAVE_MIN) * g);
}

void setup() {
  for (uint8_t i = 0; i < N; i++) {
    pinMode(buttons[i].sw, INPUT_PULLUP);   // switch closed -> reads LOW
    pinMode(buttons[i].led, OUTPUT);
  }
  Keyboard.begin();

  // Boot self-test: fade all LEDs up then down so you can confirm the wiring.
  for (int v = 0; v <= 255; v += 5) { for (uint8_t i = 0; i < N; i++) writeLed(buttons[i].led, v); delay(4); }
  for (int v = 255; v >= 0; v -= 5) { for (uint8_t i = 0; i < N; i++) writeLed(buttons[i].led, v); delay(4); }
}

void loop() {
  unsigned long now = millis();

  for (uint8_t i = 0; i < N; i++) {
    Button &b = buttons[i];

    // --- debounce ---
    bool reading = (digitalRead(b.sw) == LOW);
    if (reading != b.lastRead) { b.lastRead = reading; b.lastChange = now; }
    if ((now - b.lastChange) >= DEBOUNCE_MS && reading != b.stable) {
      b.stable = reading;                       // debounced edge
      if (b.isHold) {
        if (VOICE_TOGGLE) {
          if (b.stable) {
            if (b.keyHeld) { Keyboard.release(b.key); b.keyHeld = false; }
            else           { Keyboard.press(b.key);   b.keyHeld = true;  }
          }
        } else {
          if (b.stable) { Keyboard.press(b.key);   b.keyHeld = true;  }
          else          { Keyboard.release(b.key); b.keyHeld = false; }
        }
      } else if (b.stable) {
        Keyboard.write(b.key);                  // tap
      }
    }

    // --- LED: full while pressed, otherwise the ambient wave ---
    bool active = b.isHold ? b.keyHeld : b.stable;
    uint8_t level = active ? PRESS_BRIGHT : (WAVE_ENABLE ? waveLevel(i, now) : 0);
    writeLed(b.led, level);
  }
}

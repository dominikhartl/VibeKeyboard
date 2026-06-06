# Configuration & Remapping

Everything tunable is in the **CONFIGURATION block** at the top of
[`firmware/vibe_keyboard/vibe_keyboard.ino`](../firmware/vibe_keyboard/vibe_keyboard.ino).
Edit, then re-upload ([FLASHING.md](FLASHING.md)).

## Pins

```c
const uint8_t PIN_ACCEPT = A0;     // switches: any digital pin
const uint8_t PIN_REJECT = A1;
const uint8_t PIN_VOICE  = A2;

const uint8_t PIN_VOICE_LED  = 5;  // D5 left  — LEDs MUST be PWM (3,5,6,9,10) to dim
const uint8_t PIN_ACCEPT_LED = 6;  // D6 mid
const uint8_t PIN_REJECT_LED = 9;  // D9 right
const bool    LED_ACTIVE_HIGH = true;   // false if LEDs wired to +5V instead of GND
```

If you move an LED, keep it on a **PWM pin (3, 5, 6, 9, 10)** or it can only switch on/off
(the wave will step instead of fade). See [WIRING.md](WIRING.md) for why these pins.

### Per-LED brightness trim
LEDs of different colors (or driven through different resistors) can look uneven. Trim each
one independently — `1.0` = unchanged, higher = brighter (clamped at full):

```c
const float LED_GAIN_VOICE  = 1.0f;
const float LED_GAIN_ACCEPT = 1.5f;   // Accept LED a bit brighter
const float LED_GAIN_REJECT = 1.0f;
```

Raise a value to brighten that LED, lower it (e.g. `0.7`) to dim one that's too strong.

## Key mappings

```c
#define ACCEPT_KEY KEY_RETURN
#define REJECT_KEY KEY_ESC
#define VOICE_KEY  KEY_F13
const bool VOICE_TOGGLE = false;   // false = push-to-talk hold; true = tap on/off
```

Remap for other tools (e.g. Copilot: Accept `KEY_TAB`, Reject `KEY_ESC`). Send a plain
character with a literal, e.g. `#define ACCEPT_KEY 'y'`.

### Common `KEY_*` constants
| Key | Constant |
|-----|----------|
| Enter | `KEY_RETURN` |
| Escape | `KEY_ESC` |
| Tab | `KEY_TAB` |
| Arrows | `KEY_UP_ARROW`, `KEY_DOWN_ARROW`, `KEY_LEFT_ARROW`, `KEY_RIGHT_ARROW` |
| Modifiers | `KEY_LEFT_CTRL`, `KEY_LEFT_SHIFT`, `KEY_LEFT_ALT`, `KEY_LEFT_GUI` (⌘/Win) |
| Function keys | `KEY_F1`…`KEY_F12`, `KEY_F13`…`KEY_F24` |
| Letter/number | the character, e.g. `'a'`, `'7'` |

> `KEY_F13`–`KEY_F24` need a recent Arduino AVR core. If yours lacks `KEY_F13`, update the
> AVR boards package or pick another `VOICE_KEY` (e.g. `KEY_RIGHT_ALT`).

### Voice / dictation hotkey
Set your voice app's push-to-talk hotkey to the **same** key as `VOICE_KEY` (default F13):
SuperWhisper (Recording key → F13), Wispr Flow (Shortcut → F13, hold mode), or macOS
Dictation (toggle-style → pair with `VOICE_TOGGLE = true`).

## Chord (two buttons at once)

Pressing **Voice + Accept together** types a command — by default `/voice` + Enter, to start
Claude Code's voice dictation. A short window distinguishes the chord from two separate taps,
so the individual keys (F13 / Enter) are suppressed when you press both.

```c
const bool          CHORD_ENABLE    = true;
const uint8_t       CHORD_BTN_A     = 0;        // Voice  (index in buttons[])
const uint8_t       CHORD_BTN_B     = 1;        // Accept
const char* const   CHORD_TEXT      = "/voice"; // typed, then Enter
const unsigned long CHORD_WINDOW_MS = 50;       // how close together the presses must land
```

- Change `CHORD_TEXT` to type any command/snippet (e.g. `"/clear"`).
- Do the chord on an **empty prompt** so `/voice` is read as a command, not appended to text.
- `CHORD_WINDOW_MS` also sets the resolve delay for normal taps (50 ms ≈ imperceptible);
  raise it if your two presses don't always register together, lower it for snappier taps.

### Using it with Claude Code `/voice`
Claude Code has built-in voice dictation (`/voice`; needs a Claude.ai login). To make the
**Voice button** drive it, bind its push-to-talk key to **F13** in `~/.claude/keybindings.json`:

```json
{ "context": "Chat", "bindings": { "f13": "voice:pushToTalk" } }
```

Then: chord (Voice+Accept) starts `/voice`, and holding **Voice** dictates. Note F13 must be
delivered by your terminal — if your terminal doesn't forward F13, use the default **Space**
push-to-talk instead, or a third-party dictation app (which captures F13 globally).

## The wave animation

The LEDs run a continuous brightness **wave** (via PWM), so the backlit logo flows from one
side to the other. Pressing a button drives its LED to full, then the wave resumes.

```c
const bool          WAVE_ENABLE    = true;   // set false for plain off-until-pressed
const unsigned long WAVE_PERIOD_MS = 1800;   // one sweep; LOWER = faster
const bool          WAVE_BOUNCE    = true;   // true = back-and-forth; false = one-way loop
const bool          WAVE_REVERSE   = false;  // flip the starting direction
const float         WAVE_WIDTH     = 1.1f;   // peak softness (bigger = broader, softer)
const uint8_t       WAVE_MIN       = 4;      // dim floor (0-255)
const uint8_t       WAVE_MAX       = 200;    // peak brightness (0-255)
const uint8_t       PRESS_BRIGHT   = 255;    // brightness while pressed
const bool          GAMMA_CORRECT  = true;   // perceptual (smoother) fade
```

Tuning cheatsheet:
- **Faster / slower:** lower / raise `WAVE_PERIOD_MS` (e.g. 900 = brisk, 3000 = lazy).
- **Motion:** `WAVE_BOUNCE = true` sweeps back and forth; `false` loops one way. Flip
  `WAVE_REVERSE` to change which side it starts from. (Physical flow follows the order the
  LEDs are wired — arrange them left-to-right, or swap the `PIN_*_LED` lines, to match.)
- **Softer wave:** raise `WAVE_WIDTH` (e.g. 1.6). **Sharper, comet-like:** lower it (e.g. 0.7).
- **Overall brightness:** raise/lower `WAVE_MAX`; raise `WAVE_MIN` for a glowing baseline.
- **No animation:** set `WAVE_ENABLE = false` — LEDs then just light full while their button
  is pressed (off otherwise).

The boot **self-test** (all LEDs fade up then down once) is in `setup()`; it confirms wiring
and isn't part of the wave.

# Configuration & Remapping

Everything you'd want to change lives in the **CONFIGURATION block** at the top of
[`firmware/vibe_keyboard/vibe_keyboard.ino`](../firmware/vibe_keyboard/vibe_keyboard.ino).
Edit, then re-upload ([FLASHING.md](FLASHING.md)). No logic changes needed.

## The config constants

```c
// ---- Switch pins ----
const uint8_t PIN_ACCEPT = 2;
const uint8_t PIN_REJECT = 3;
const uint8_t PIN_VOICE  = 4;

// ---- LED pins (one per button) ----
const uint8_t PIN_ACCEPT_LED = 5;
const uint8_t PIN_REJECT_LED = 6;
const uint8_t PIN_VOICE_LED  = 7;
const bool    LED_ACTIVE_HIGH = true;

// ---- Key mappings ----
#define ACCEPT_KEY KEY_RETURN
#define REJECT_KEY KEY_ESC
#define VOICE_KEY  KEY_F13

// ---- Behavior ----
const bool          VOICE_TOGGLE = false;   // false = push-to-talk, true = toggle
const unsigned long DEBOUNCE_MS  = 8;
```

| Constant | What it controls |
|----------|------------------|
| `PIN_*` | Which Pro Micro pins the switches/LEDs use. Change if you wired differently. |
| `LED_ACTIVE_HIGH` | `true` for pin→resistor→LED→GND wiring; `false` for +5V→resistor→LED→pin. |
| `ACCEPT_KEY` / `REJECT_KEY` | The single keystroke each taps. |
| `VOICE_KEY` | The key held (or toggled) for dictation. |
| `VOICE_TOGGLE` | `false` = hold-to-talk; `true` = tap-on / tap-off. |
| `DEBOUNCE_MS` | Switch debounce window. Raise to ~15 if you get double-fires. |

## Remapping Accept / Reject for other tools

The defaults (`Enter` / `Esc`) suit Claude Code in a terminal. For other workflows, change
`ACCEPT_KEY` / `REJECT_KEY`. A few examples:

| Tool | Accept | Reject |
|------|--------|--------|
| Claude Code (terminal) — default | `KEY_RETURN` | `KEY_ESC` |
| GitHub Copilot (inline suggestion) | `KEY_TAB` | `KEY_ESC` |
| Generic "yes / no" prompt | `'y'` | `'n'` |

To send a **letter or digit**, just use the character literal, e.g. `#define ACCEPT_KEY 'y'`.

### Sending a key combo (modifiers)
A `#define` holds one key. For a combo like **⌘↵** (Cmd+Enter, e.g. "accept" in some
editors), edit the press in `loop()` for that button to chord a modifier, for example:

```c
// inside the Accept branch, instead of Keyboard.write(b.key):
Keyboard.press(KEY_LEFT_GUI);   // Cmd  (use KEY_LEFT_CTRL for Ctrl on Win/Linux)
Keyboard.write(KEY_RETURN);
Keyboard.release(KEY_LEFT_GUI);
```

## Common `KEY_*` constants

| Key | Constant |
|-----|----------|
| Enter / Return | `KEY_RETURN` |
| Escape | `KEY_ESC` |
| Tab | `KEY_TAB` |
| Backspace | `KEY_BACKSPACE` |
| Space | `' '` |
| Arrow keys | `KEY_UP_ARROW`, `KEY_DOWN_ARROW`, `KEY_LEFT_ARROW`, `KEY_RIGHT_ARROW` |
| Modifiers | `KEY_LEFT_CTRL`, `KEY_LEFT_SHIFT`, `KEY_LEFT_ALT`, `KEY_LEFT_GUI` (⌘/Win), and `KEY_RIGHT_*` |
| Function keys | `KEY_F1` … `KEY_F12`, and `KEY_F13` … `KEY_F24` |
| Any letter/number | the character itself, e.g. `'a'`, `'7'` |

> **Note on F13–F24:** these need a reasonably recent Arduino AVR core / `Keyboard` library.
> If your core doesn't define `KEY_F13`, update the AVR boards package, or pick a different
> `VOICE_KEY` (see below).

## The Voice button

### Push-to-talk vs. toggle
- `VOICE_TOGGLE = false` (default): **hold** Voice to hold `VOICE_KEY` down; release to let go.
- `VOICE_TOGGLE = true`: **tap** Voice to start (key stays held), tap again to stop.

The Voice LED tracks the held state either way.

### Picking `VOICE_KEY`
`KEY_F13` is the default because **F13 does nothing on its own** (macOS and Windows leave it
unbound), so it's a safe, dedicated hotkey for your dictation software. Alternatives:

- A held modifier some apps prefer: `#define VOICE_KEY KEY_RIGHT_ALT`
- Any other unused key/combo your voice app lets you bind.

### Setting up your dictation app
Whatever you pick for `VOICE_KEY`, set the **same** key as your voice app's push-to-talk
hotkey:

| App | Where to set the hotkey |
|-----|-------------------------|
| **SuperWhisper** | Settings → Recording / Modes → set the activation key to **F13** |
| **Wispr Flow** | Settings → Shortcut → bind **F13** (use hold-to-talk mode) |
| **macOS Dictation** | System Settings → Keyboard → Dictation → set the shortcut; note macOS Dictation is toggle-style, so pair it with `VOICE_TOGGLE = true` |
| **Talon / others** | Bind a "speech" or "push-to-talk" command to **F13** |

After binding, hold the Voice button and speak — release to stop.

## Customizing the boot LED pattern

The light show on power-up is a plain table you can rewrite:

```c
struct LedFrame { bool a; bool r; bool v; uint16_t ms; };
const LedFrame STARTUP_PATTERN[] = {
  {1, 0, 0, 120},   // accept on, 120 ms
  {0, 1, 0, 120},   // reject on
  {0, 0, 1, 120},   // voice on
  {1, 1, 1, 150},   // all on
  {0, 0, 0, 150},   // all off
  {1, 1, 1, 150},
  {0, 0, 0, 0},     // end dark
};
```

Each frame is `{ acceptLED, rejectLED, voiceLED, durationMs }` — `1` = on, `0` = off. Add,
remove, or reorder frames freely; the firmware sizes the table automatically. Examples:

```c
// Triple blink, all together:
const LedFrame STARTUP_PATTERN[] = {
  {1,1,1,150},{0,0,0,150},{1,1,1,150},{0,0,0,150},{1,1,1,150},{0,0,0,0}
};

// Slow back-and-forth sweep:
const LedFrame STARTUP_PATTERN[] = {
  {1,0,0,150},{0,1,0,150},{0,0,1,150},{0,1,0,150},{1,0,0,150},{0,0,0,0}
};
```

> **Fades?** The pattern uses simple on/off so it works on any pin. `D5` and `D6` are
> PWM-capable, so if you want brightness fades you can drive those two with `analogWrite()`
> in a custom pattern routine. `D7` is digital-only.

After any change: re-upload and watch it on the next power-up.

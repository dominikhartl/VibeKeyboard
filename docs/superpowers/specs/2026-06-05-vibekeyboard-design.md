# VibeKeyboard — Design Spec

**Date:** 2026-06-05
**Status:** Approved

## Summary

A three-button USB macro pad for AI-assisted ("vibe") coding, built on an Arduino Pro
Micro (ATmega32U4) with standard Cherry MX switches. The board enumerates as a standard
USB-HID keyboard, so it works on any OS with no driver. Each button has a dedicated status
LED. Goal: a complete, reproducible open-hardware project — firmware plus full build,
wiring, flashing, and configuration documentation — that anyone can rebuild.

## Buttons

| Button | Default action | Rationale |
|---|---|---|
| **Voice Control** | Push-to-talk: hold `F13`, release to stop | F13 is unbound on macOS; bind your dictation app's PTT hotkey to it |
| **Accept** | Tap `Enter` | Confirms the highlighted/default action in Claude Code |
| **Reject** | Tap `Esc` | Cancels the current action/prompt in Claude Code |

All mappings are editable constants at the top of the sketch — remap for Cursor, Copilot,
or any tool without touching the logic.

## LEDs

- **One LED per button** (3 total), each on its own GPIO via a 220 Ω resistor to the GND bus.
- **Boot:** a customizable `STARTUP_PATTERN` frame table plays once (default: chase + double
  flash), giving a visible "it powered up and ran" signal.
- **Normal use:** off until pressed. Each LED mirrors its button's live pressed state, so the
  Voice LED stays lit for the whole push-to-talk hold and Accept/Reject blink on tap.

## Hardware

- **MCU:** Arduino Pro Micro (ATmega32U4, 5 V / 16 MHz), native USB-HID.
- **Switches:** 3 × Cherry MX (any variant), read active-low with internal pull-ups —
  **no diodes, no key matrix, no external button resistors**.
- **Pins:** switches on `D2/D3/D4`; LEDs on `D5/D6/D7`; common `GND`.
- **Passives:** 3 × 220 Ω (LEDs only).

## Firmware

- Single Arduino sketch, only the built-in `Keyboard` library (zero third-party deps).
- Inline ~8 ms software debounce per button.
- Accept/Reject fire on the debounced press edge; Voice uses `Keyboard.press`/`release`
  for true push-to-talk. A `VOICE_TOGGLE` flag offers tap-to-toggle as an alternative.
- A uniform `Button` struct + `applyLeds()` helper (honoring `LED_ACTIVE_HIGH`) keep the
  three channels symmetric and the loop readable.

## Repository / docs

- `firmware/vibe_keyboard/vibe_keyboard.ino` — the sketch.
- `hardware/BOM.md` — bill of materials + tools.
- `docs/ASSEMBLY.md`, `WIRING.md`, `FLASHING.md`, `CONFIGURATION.md` — build, wire, flash, customize.
- `docs/images/wiring-diagram.svg` — schematic-style wiring diagram.
- `case/` — git-ignored drop folder for the user's own logo + STL (only its README is tracked).
- `README.md`, `LICENSE` (MIT), `.gitignore`.

## Out of scope

- No 3D case design generated here (the `case/` folder is a local, git-ignored drop-spot).
- No key matrix / diodes (unnecessary for 3 independent switches).
- No host-side software (the board is a plain HID keyboard).

## Verification

1. Headless compile via `arduino-cli` against the AVR core.
2. `.gitignore` keeps `case/` binaries out while tracking `case/README.md`.
3. Docs render and cross-links resolve on GitHub.
4. Documented on-device smoke test: boot pattern plays; Accept→newline, Reject→Esc,
   Voice→held F13 (LED tracks), then bind the voice app PTT to F13.

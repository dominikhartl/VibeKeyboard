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
| **Voice Control** | Push-to-talk: hold `Space` (Claude Code's built-in PTT), release to stop | Alternative: `KEY_F13` + a global dictation app (terminals swallow F13, so native /voice needs Space) |
| **Accept** | Tap `Enter` | Confirms the highlighted/default action in Claude Code |
| **Reject** | Tap `Esc` | Cancels the current action/prompt in Claude Code |

All mappings are editable constants at the top of the sketch — remap for Cursor, Copilot,
or any tool without touching the logic.

## LEDs

- **One dimmable LED per button** (3 total), each on a **PWM** pin via a 220 Ω resistor to the GND bus.
- **Boot:** a self-test fades all LEDs up then down once (confirms wiring).
- **Normal use:** a continuous **wave** animation flows across the three LEDs (and the backlit
  logo). Pressing a button drives its LED to full for feedback, then the wave resumes. The
  Voice LED stays full for the whole push-to-talk hold. Tunable (speed, width, direction,
  brightness) or disabled via `WAVE_ENABLE`.

## Hardware

- **MCU:** Arduino Pro Micro (ATmega32U4, 5 V / 16 MHz), native USB-HID.
- **Switches:** 3 × Cherry MX (any variant), read active-low with internal pull-ups —
  **no diodes, no key matrix, no external button resistors**.
- **Pins:** switches on `A0/A1/A2` (digital input); LEDs on `D5/D6/D9` (**PWM**, for dimming);
  common `GND`. Chosen to keep I²C/SPI/UART/spare-PWM free for future add-ons.
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
   Voice→held Space (LED tracks); tap Voice+Accept to run /voice, then hold Voice to dictate.

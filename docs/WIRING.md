# Wiring

The electronics are deliberately minimal. Three independent switches and three LEDs wired
straight to GPIO pins — no key matrix, no diodes, and no resistors on the buttons.

![Wiring diagram](images/wiring-diagram.svg)

## Pin map

| Function | Pro Micro pin | Connection |
|----------|---------------|------------|
| Accept switch | `D2` | one switch leg → `D2`, other leg → GND |
| Reject switch | `D3` | one switch leg → `D3`, other leg → GND |
| Voice switch | `D4` | one switch leg → `D4`, other leg → GND |
| Accept LED | `D5` | `D5` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Reject LED | `D6` | `D6` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Voice LED | `D7` | `D7` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Ground | `GND` | shared return for every switch leg and every LED cathode |

These pin numbers match the constants at the top of
[`firmware/vibe_keyboard/vibe_keyboard.ino`](../firmware/vibe_keyboard/vibe_keyboard.ino).
Change them there if you wire to different pins.

## Why it's this simple

### Switches: active-low with internal pull-ups
Each switch connects its pin to **GND**. In firmware the pin is set to `INPUT_PULLUP`, which
turns on a resistor *inside* the ATmega32U4 that gently holds the pin **HIGH** when the
switch is open. Pressing the switch shorts the pin to **GND**, so it reads **LOW**.

- Pin reads **HIGH** → button released.
- Pin reads **LOW** → button pressed.

That's why **no external pull-up/pull-down resistors are needed** on the buttons.

### No diodes, no matrix
Diodes and row/column matrices exist to tell apart multiple simultaneous presses on a
*shared-wire* layout. With only three switches, each gets its **own pin**, so there is
nothing to disambiguate. Skip the diodes.

### LEDs: one resistor each
An LED needs a series resistor so it doesn't draw too much current. With a 5 V pin and a
typical LED (~2 V forward voltage):

```
I = (5 V − 2 V) / 220 Ω ≈ 13 mA
```

That's a safe, bright level well under the Pro Micro's ~20 mA-per-pin limit. Want them
dimmer? Use a larger resistor (330 Ω, 470 Ω…). The LED is **polarized**: the longer leg is
the anode (+, toward the resistor/pin); the flat side / shorter leg is the cathode (−,
toward GND). Backwards = it simply won't light.

### LED polarity in firmware
The sketch assumes LEDs are wired **pin → resistor → LED → GND** (`LED_ACTIVE_HIGH = true`,
HIGH turns the LED on). If you instead wire them **+5V → resistor → LED → pin**, set
`LED_ACTIVE_HIGH = false`.

## Recommended build order for the ground wire
Run a single **GND bus**: solder a wire from the Pro Micro `GND` pin, then daisy-chain it to
one leg of each switch and to each LED's cathode. One wire back to the board, many taps off
it. See [ASSEMBLY.md](ASSEMBLY.md) for the step-by-step.

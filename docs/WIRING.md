# Wiring

Minimal and future-proofed: three switches read with internal pull-ups, three LEDs on
**PWM pins** so they can dim for the wave animation. No key matrix, no diodes, no resistors
on the buttons — just one resistor per LED.

![Wiring diagram](images/wiring-diagram.svg)

## Pin map

| Function | Pro Micro pin | Connection |
|----------|---------------|------------|
| Accept switch | `A0` | one switch leg → `A0`, other leg → GND |
| Reject switch | `A1` | one switch leg → `A1`, other leg → GND |
| Voice switch | `A2` | one switch leg → `A2`, other leg → GND |
| Voice LED *(leftmost)* | `D5` (PWM) | `D5` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Accept LED *(middle)* | `D6` (PWM) | `D6` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Reject LED *(rightmost)* | `D9` (PWM) | `D9` → 220 Ω → LED **anode (+)** → LED **cathode (−)** → GND |
| Ground | `GND` | shared return for every switch leg and every LED cathode |

The LED pins are ordered **left → right (Voice · Accept · Reject)** — the direction the wave
flows. (Switch pins are independent of LED order; each button still sends its own key.)

These match the constants at the top of
[`firmware/vibe_keyboard/vibe_keyboard.ino`](../firmware/vibe_keyboard/vibe_keyboard.ino).

## Why these pins (future-proofing)

The job only needs 3 digital inputs and 3 dimmable outputs, so the pins were chosen to
**keep every special-function pin free** for later upgrades:

- **Switches on `A0`–`A2`.** The analog header pins work fine as plain digital inputs with
  pull-ups, and using them here frees the more capable digital pins.
- **LEDs on `D5`, `D6`, `D9`.** Dimming needs **hardware PWM**, which on the Pro Micro only
  exists on pins **3, 5, 6, 9, 10**. These three are on three independent timers (Timer3,
  Timer4, Timer1), so they dim independently and don't disturb `millis()` (Timer0).

### Left free for upgrades
| Pins | Bus / capability | Example upgrade |
|------|------------------|-----------------|
| `D2` (SDA), `D3` (SCL) | **I²C** | OLED status screen, sensors, I/O expander |
| `D0` (RX), `D1` (TX) | **UART** | serial debug, ESP/BT module |
| `D14`/`D15`/`D16` | **SPI** | SPI display, SD card |
| `D10` | **spare PWM** | 4th LED channel, addressable-RGB data, buzzer |
| `D4`, `D7`, `D8`, `A3` | GPIO / analog / `D7`=INT6 | rotary encoder, more keys, a pot |

So you can add a screen (I²C), an encoder (`D7` interrupt + a neighbor), RGB underglow
(`D10`), or more keys without re-flowing the existing wiring.

## Why it's this simple

### Switches: active-low with internal pull-ups
Each switch connects its pin to **GND**. The firmware sets the pin to `INPUT_PULLUP`, which
holds it **HIGH** when open; pressing shorts it to **GND** so it reads **LOW**. No external
pull-up/pull-down resistors needed, and **no diodes** — with only three switches on their
own pins there is nothing to disambiguate, so no matrix.

### LEDs: one resistor each, on PWM
A 220 Ω series resistor sets a safe ~13 mA at 5 V (use a bigger value for dimmer LEDs). The
pins are PWM, so `analogWrite()` gives 256 brightness levels for the wave. The LED is
**polarized**: long leg = anode (+, toward the resistor/pin), flat side/short leg = cathode
(−, toward GND). Backwards = it won't light.

The sketch assumes **pin → resistor → LED → GND** (`LED_ACTIVE_HIGH = true`). If you wire
**+5V → resistor → LED → pin** instead, set `LED_ACTIVE_HIGH = false`.

## Ground bus
Run a single **GND bus**: a wire from the Pro Micro `GND`, daisy-chained to one leg of each
switch and to each LED's cathode. See [ASSEMBLY.md](ASSEMBLY.md) for the build order.

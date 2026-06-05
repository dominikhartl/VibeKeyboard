# Assembly / Soldering Guide

Time: ~30–45 minutes. Difficulty: beginner-friendly (7 solder joints per switch/LED group,
all point-to-point). Read [WIRING.md](WIRING.md) first so the pin map is fresh.

> **Safety:** A soldering iron is 300–400 °C. Work in a ventilated space, don't touch the
> tip, and let joints cool before handling. Wear eye protection when trimming leads.

## What you need
See [`hardware/BOM.md`](../hardware/BOM.md). In short: the Pro Micro, 3 switches, 3 LEDs,
3 × 220 Ω resistors, hookup wire, solder, and your tools.

## Step 0 — Test the Pro Micro *before* soldering
Plug the bare Pro Micro into USB. The on-board power LED should light, and your computer
should detect a new USB device. Even better, [flash the firmware now](FLASHING.md) while the
board is bare and easy to handle — that confirms the board and your toolchain work before
you commit any solder.

## Step 1 — Plan the physical layout
Decide where the three switches sit (a row is typical: Voice · Accept · Reject). If you have
a switch plate, perfboard, or printed case, mount the switches in it now so they stay put.
Hand-wiring on the bench works too — just be gentle with the joints afterward.

Note each Cherry MX switch has **two electrical pins** (plus possibly plastic mounting
posts). Either electrical pin can be the "signal" or the "ground" side — they're symmetric.

## Step 2 — Build the common GND bus
This single wire is the shared ground for everything.

1. Cut a wire long enough to reach all three switches and all three LED positions.
2. Solder one end to the Pro Micro **`GND`** pin.
3. Daisy-chain it: solder a tap to **one leg of each switch**.
4. Keep the same bus going to where each **LED cathode (−)** will connect (Step 4).

Tip: strip small windows in a single solid-core wire rather than cutting it into pieces —
makes a tidy continuous bus.

## Step 3 — Wire the switch signals
For each switch, solder a wire from its **other** leg (the one not on the GND bus) to its pin:

| Switch | → Pro Micro pin |
|--------|-----------------|
| Accept | `D2` |
| Reject | `D3` |
| Voice  | `D4` |

## Step 4 — Wire the LEDs (mind polarity!)
Each LED is **polarized**. Long leg = **anode (+)**, short leg / flat edge = **cathode (−)**.

For each LED:
1. Solder one end of a **220 Ω resistor** to the Pro Micro pin (`D5` Accept, `D6` Reject,
   `D7` Voice). Resistors are not polarized — either end is fine.
2. Solder the resistor's other end to the LED **anode (+)** (long leg).
3. Solder the LED **cathode (−)** (short leg) to the **GND bus**.

| LED | Pin → 220 Ω → anode(+) | cathode(−) → |
|-----|------------------------|--------------|
| Accept | `D5` | GND bus |
| Reject | `D6` | GND bus |
| Voice  | `D7` | GND bus |

## Step 5 — Inspect
- Look for solder bridges between adjacent pins, and for cold/dull joints (reheat them).
- *Optional, recommended:* with the board **unplugged**, set a multimeter to continuity and
  confirm: each switch leg ↔ its pin, each GND tap ↔ `GND`, and **no** short between any
  signal pin and GND while the switches are released.

## Step 6 — Flash & test
1. Flash the firmware if you haven't: see [FLASHING.md](FLASHING.md).
2. On power-up the LEDs play the **boot pattern** (chase, then a double flash). Seeing it is
   your visual confirmation the board powered up and the firmware is running.
3. Open any text editor and test:
   - **Accept** → types a newline; its LED blinks during the press.
   - **Reject** → sends `Esc`; its LED blinks during the press.
   - **Voice** → hold it: the Voice LED stays lit the whole time and `F13` is held down;
     release and it stops.
4. Finally, bind your dictation app's push-to-talk hotkey to **F13** (see
   [CONFIGURATION.md](CONFIGURATION.md)) and confirm holding Voice starts/stops dictation.

## Troubleshooting
| Symptom | Likely cause |
|---------|--------------|
| An LED never lights | LED reversed (swap legs), or `LED_ACTIVE_HIGH` wrong in firmware |
| A button does nothing | Signal wire on wrong pin, or cold joint; recheck Step 3 |
| A button fires constantly | Signal pin shorted to GND; look for a solder bridge |
| Upload fails / no port | Use the double-tap-reset trick — see [FLASHING.md](FLASHING.md) |
| Nothing at all on boot | Re-test the bare board (Step 0); check the USB cable is data-capable |

# Assembly / Soldering Guide

Build time ~40 min, beginner-friendly. Every connection is point-to-point — no PCB. Skim
[WIRING.md](WIRING.md) first so the pin map is fresh.

> **Safety:** the iron tip is ~350 °C. Work in a ventilated space, tin the tip, never touch
> the tip or fresh joints, and wear eye protection when trimming leads.

## Parts & tools
Full list: [`hardware/BOM.md`](../hardware/BOM.md). Core: Pro Micro, 3 × Cherry MX switches,
3 × LEDs, 3 × 220 Ω resistors, hookup wire, solder. Helpful: helping-hands, flush cutters,
a multimeter for continuity.

## Connection summary (what you're about to build)
```
   A0 ── Accept switch ──┐
   A1 ── Reject switch ──┤
   A2 ── Voice  switch ──┤
                         ├── GND  (one shared ground bus)
   D5 ─220Ω─▷|─ VceLED ──┤   (left)
   D6 ─220Ω─▷|─ AccLED ──┤   (mid)
   D9 ─220Ω─▷|─ RejLED ──┘   (right)
        (▷| = LED, flat/short leg = cathode toward GND)
```
Each switch: one leg to its signal pin, the other to the GND bus.
Each LED: signal pin → 220 Ω → LED **+** (long leg); LED **−** (short/flat) → GND bus.

---

## Step 0 — Test the bare Pro Micro first
Plug it into USB. The power LED should light and the PC should detect a USB device. Best of
all, **[flash the firmware now](FLASHING.md)** while the board is bare — on boot it fades all
three LED pins up/down (the self-test), so you can verify everything *before* soldering, and
the board is far easier to handle now than later.

## Step 1 — Arrange the switches
Decide the row order (e.g. **Voice · Accept · Reject**). If you have a plate, perfboard, or
printed case, mount the switches so they can't move. Each Cherry MX switch has **two
electrical pins** (plus possibly plastic posts) — either electrical pin can be signal or
ground; they're symmetric.

## Step 2 — Lay the common GND bus
This single wire is ground for everything.
1. Cut a wire long enough to reach all three switches and all three LED spots.
2. Solder one end to a Pro Micro **`GND`** pad (there are several; pick a convenient one).
3. Strip small windows along the wire and solder a tap to **one leg of each switch**.
4. Continue the same bus toward where each **LED cathode (−)** will land (Step 4).

Tip: a single continuous solid-core wire with stripped windows makes the tidiest bus.

## Step 3 — Switch signal wires
Solder a wire from each switch's **other** leg to its pin:

| Switch | → pin |
|--------|-------|
| Accept | `A0` |
| Reject | `A1` |
| Voice  | `A2` |

## Step 4 — LEDs (watch polarity!)
LED legs: **long = anode (+)**, **short / flat side = cathode (−)**.

For each LED:
1. Solder one end of a **220 Ω** resistor to the LED pin (`D5` Voice, `D6` Accept, `D9`
   Reject — left to right). Resistors aren't polarized.
2. Solder the resistor's free end to the LED **anode (+)**.
3. Solder the LED **cathode (−)** to the **GND bus**.

| LED | pin → 220 Ω → anode(+) | cathode(−) |
|-----|------------------------|------------|
| Voice *(left)* | `D5` | GND bus |
| Accept *(mid)* | `D6` | GND bus |
| Reject *(right)* | `D9` | GND bus |

> The LEDs **must** be on `D5/D6/D9` (PWM pins) for the wave to dim smoothly. Any other pin
> can only switch on/off.

## Step 5 — Inspect
- Hunt for solder bridges between adjacent pads and reflow any dull/cold joints.
- *Recommended:* with the board **unplugged**, use a multimeter on continuity to confirm
  each switch leg ↔ its pin, each ground tap ↔ `GND`, and **no** short between any signal
  pin and GND while switches are released.

## Step 6 — Flash & test
1. [Flash the firmware](FLASHING.md) if you haven't.
2. On power-up the three LEDs **fade up then down** (the self-test) and then settle into the
   continuous **wave**. Seeing the wave flow is your "it works" signal.
3. In any text editor:
   - **Accept** → newline; its LED jumps to full during the press.
   - **Reject** → `Esc`; its LED jumps to full during the press.
   - **Voice** → hold it: the Voice LED stays full and `F13` is held; release to stop.
4. Bind your dictation app's push-to-talk hotkey to **F13** ([CONFIGURATION.md](CONFIGURATION.md))
   and confirm holding Voice starts/stops dictation.

## Troubleshooting
| Symptom | Likely cause |
|---------|--------------|
| An LED never lights | LED reversed (swap legs), or `LED_ACTIVE_HIGH` wrong |
| Wave steps instead of fading on one LED | that LED isn't on a PWM pin — use `D5/D6/D9` |
| A button does nothing | signal wire on the wrong pin, or a cold joint (recheck Step 3) |
| A button fires non-stop | signal pin shorted to GND — look for a bridge |
| Upload fails / no port | double-tap-reset trick — see [FLASHING.md](FLASHING.md) |
| Nothing on boot | re-test the bare board (Step 0); check the USB cable carries data |

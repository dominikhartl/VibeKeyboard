# Bill of Materials

Everything you need to build one VibeKeyboard.

## Components

| Qty | Item | Notes / suggestions |
|-----|------|---------------------|
| 1 | **Arduino Pro Micro** (ATmega32U4, 5 V / 16 MHz) | Genuine SparkFun or a clone. Must be the 32U4 version — it has native USB so it can act as a keyboard. The Arduino Leonardo works too. |
| 3 | **Cherry MX switches** | Any variant (Red = linear, Brown = tactile, Blue = clicky). Gateron/Kailh MX-clones are fine. Each has 2 electrical pins. |
| 3 | **Keycaps** (MX-compatible) | Optional but nice. Consider labels: Voice / Accept / Reject. |
| 3 | **LEDs**, 3 mm or 5 mm | One status LED per button. Pick whatever color you like. |
| 3 | **Resistors, 220 Ω** | One per LED, for current limiting (~13 mA @ 5 V). 150–470 Ω all work; higher = dimmer. |
| — | **Hookup wire**, 24–30 AWG | Solid core is easiest for point-to-point. ~1 m total. |
| 1 | **USB cable** for the Pro Micro | Micro-USB or USB-C depending on your board. This is also the cable you use day-to-day. |
| — | **Solder** (60/40 or lead-free) + flux | — |

## Optional

| Qty | Item | Notes |
|-----|------|-------|
| 1 | Switch plate / perfboard / 3D-printed case | Holds the switches steady. Hand-wiring works without one. Design files go in the local `case/` folder. |
| 3 | Diodes (1N4148) | **Not needed** for this build — only listed because matrix keyboards use them. Skip. |

## Tools

| Tool | Why |
|------|-----|
| Soldering iron (~30–60 W, fine tip) | Joining wires to the Pro Micro and switches |
| Wire strippers | Stripping hookup wire |
| Flush cutters | Trimming leads |
| Helping hands / vise | Holding parts while soldering |
| Multimeter *(optional)* | Continuity checks to catch shorts/cold joints |
| Tweezers *(optional)* | Handling small parts |

## Cost

Roughly **$10–20** depending on whether the Pro Micro is genuine and what switches you
already have on hand.

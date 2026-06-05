# Flashing the Firmware

The firmware is a single Arduino sketch with no third-party libraries. You can flash it with
the **Arduino IDE** (easiest, recommended) or the **arduino-cli** (command line). It works
on macOS, Windows, and Linux.

The sketch: [`firmware/vibe_keyboard/vibe_keyboard.ino`](../firmware/vibe_keyboard/vibe_keyboard.ino)

---

## Option A — Arduino IDE (recommended)

### 1. Install the Arduino IDE
Download from <https://www.arduino.cc/en/software> and install.

### 2. Add board support for the Pro Micro
The Pro Micro isn't in the IDE by default. Two options:

- **Best (SparkFun definition):**
  1. **Arduino IDE → Settings** → *Additional Boards Manager URLs* → add:
     `https://raw.githubusercontent.com/sparkfun/Arduino_Boards/master/IDE_Board_Manager/package_sparkfun_index.json`
  2. **Tools → Board → Boards Manager** → search **"SparkFun AVR"** → Install.
  3. **Tools → Board → SparkFun AVR Boards → SparkFun Pro Micro**.
  4. **Tools → Processor → ATmega32U4 (5V, 16 MHz)** (match your board's printed voltage!).
- **Quick (no extra install):** many Pro Micro clones flash fine as **Tools → Board →
  Arduino AVR Boards → Arduino Leonardo** (same ATmega32U4 chip).

### 3. Open the sketch
**File → Open** → select `firmware/vibe_keyboard/vibe_keyboard.ino`. (The `.ino` must stay
inside a folder of the same name, `vibe_keyboard/` — that's how Arduino sketches work.)

The `Keyboard` library is bundled with the Arduino AVR core, so there's nothing else to
install. If the IDE ever flags it, install **"Keyboard" by Arduino** via **Tools → Manage
Libraries**.

### 4. Select the port
**Tools → Port** → pick the one that appears when the Pro Micro is plugged in (often
`/dev/cu.usbmodemXXXX` on macOS, `COMx` on Windows).

### 5. Upload
Click **Upload** (→ arrow). The IDE compiles and flashes. On success the LEDs play the boot
pattern.

---

## Option B — arduino-cli

```bash
# install (macOS example)
brew install arduino-cli

# one-time setup
arduino-cli core update-index
arduino-cli core install arduino:avr        # the Leonardo / ATmega32U4 target
arduino-cli lib install Keyboard            # the built-in Keyboard HID library

# compile (no hardware needed — good as a sanity check)
arduino-cli compile --fqbn arduino:avr:leonardo firmware/vibe_keyboard

# find the port, then upload
arduino-cli board list
arduino-cli upload -p /dev/cu.usbmodemXXXX --fqbn arduino:avr:leonardo firmware/vibe_keyboard
```

---

## The Pro Micro reset trick (important!)

Pro Micros run as a USB keyboard, which can make the upload port hard to catch. If an upload
**hangs at "Uploading…" or errors out**:

1. Click **Upload** in the IDE.
2. The moment the status bar says *Uploading*, **quickly short the `RST` pin to `GND` twice**
   (double-tap reset). On boards with a reset button, just press it twice quickly.
3. This forces the bootloader to stay awake for ~8 seconds so the upload can grab the port.

You usually only need this the first time, or if you ever upload a broken sketch.

> **Recovery:** if you flash bad firmware and the board misbehaves, the double-tap-reset
> bootloader window always lets you re-flash a good sketch. The board is hard to truly brick.

---

## Verify it worked
- The three LEDs run the **boot pattern** on power-up.
- In a text editor: **Accept** types a newline, **Reject** sends `Esc`, **Voice** holds `F13`
  while pressed (its LED stays lit).

Next: [configure your keys and voice app](CONFIGURATION.md).

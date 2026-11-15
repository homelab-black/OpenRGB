# Akko MU02 ISO

Native USB HID support for the Akko MU02 ISO keyboard (`342D:E4CC`) with VIA
protocol 12. The VIA application and Akko's web application are not runtime
dependencies. No firmware replacement is required.

## Controls

- 46 hardware effects, including **Custom** static per-key lighting.
- 83 physical key LEDs arranged in a 6-by-15 matrix; holes and the rotary knob
  are omitted. Key names describe physical ISO positions, not the OS layout.
- Effect speed and mode-specific hue/saturation where the effect supports them.
- Global brightness, except in All Off. This affects every key regardless of
  which LED is selected. Individual-key brightness is not supported.
- Explicit **Save To Device** in Custom applies the full per-key palette. This
  vendor command may also persist colours; it is never sent automatically.

In **Custom**, select a key or LED-view selection and choose a colour. Then press
**Save To Device**, particularly when activating previously unlit keys. Colour
readback alone does not establish that the complete palette is displayed.
**Save Profile** stores a software profile, not the palette commit; after loading
such a profile, press **Save To Device** to apply its full palette.

There is no Direct/streaming RGB mode. Per-key colours use hue and saturation;
black turns a key off, while nonblack colours use the global brightness. The
colour picker's RGB value is not an independent brightness control. In global
colour modes, black is ignored with a warning; use All Off or global brightness.

## Firmware limitations and safety

- Brightness is quantized by the firmware: requesting 255 was observed to return
  253. OpenRGB retains the actual readback and does not repeatedly rewrite it.
  The keyboard's Fn+Up shortcut reaches the physical maximum of 255. Intermediate
  levels have not been calibrated.
- Detection sends only GET requests: protocol, four global values and 90 matrix
  colour slots. Every transaction checks report length and echoed arguments.
- Only VIA protocol 12 and effects 0 through 45 are accepted.
- Brightness, speed and colour caches avoid rewriting untouched values or undoing
  physical-shortcut adjustments during unrelated updates.
- The Custom save callback refreshes the mode, flushes pending colours, and
  refuses to commit if a key's write/readback is unverified. One commit is sent
  per explicit save, without automatic retries.
- No standard VIA EEPROM save, reset, keymap or firmware commands are sent.
  Power-cycle persistence of the vendor palette command remains untested.
- Do not run VIA, Akko's web driver, another OpenRGB instance controlling this
  device, or a diagnostic HID reader concurrently. MU02 replies can be delivered
  to multiple open handles and leave a client's request/reply queue out of sync.

Profiles created by an earlier experimental single-LED driver need recreating
for the 83-key layout.

## Protocol

Source: the MU02 ISO definition and the JavaScript at
<https://akko.hsgaming.cn/>, inspected in November 2026, followed by hardware tests.
The vendor device table explicitly includes `13357_58572.json` and its ISO variant.

The HID interface uses usage page `FF60`, usage `61`, report ID zero and 32-byte
payloads. Host writes include the leading report-ID byte (33 bytes total).
Commands below are hexadecimal; replies echo the command and arguments.

| Operation | Command | Arguments | Reply data |
| --- | --- | --- | --- |
| Protocol version | `01` | none | `00 0C` |
| Get global value | `08` | `03 id` | starts at byte 3 |
| Set global value | `07` | `03 id value...` | echo |
| Get key colour | `08` | `00 03 00 row col` | byte 6 saturation, byte 7 hue |
| Set key colour | `07` | `00 03 00 row col saturation hue` | echo |
| Apply/save palette | `07` | `00 02 00` | echo |

Global IDs: 1 brightness, 2 effect, 3 speed, 4 colour (hue then saturation).
Effect 45 is Custom; the older VIA definition misleadingly calls it Close All.
A per-key hue/saturation pair of zero means off; white uses hue 1, saturation 0.

A reversible Esc test established live colour updates for an already-lit key.
A subsequent explicitly authorized palette commit activated the whole keyboard,
while all 90 colour slots and global settings retained identical readback. This
is why Custom exposes manual save rather than relying on per-key SETs alone.

## Tests

The standalone fake-HID transport test does not access hardware:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -fno-omit-frame-pointer -g -Idependencies/hidapi-win/include \
    -IControllers/QMKController -IControllers/QMKController/AkkoMU02Controller \
    tests/akko-mu02.cpp \
    Controllers/QMKController/AkkoMU02Controller/AkkoMU02Controller.cpp \
    -o /tmp/akko-mu02-test
/tmp/akko-mu02-test
```

After a full Linux qmake build in `build/`, the adapter fixture links the actual
OpenRGB framework to the same fake HID implementation:

```sh
python3 tests/check-akko-mu02-adapter.py
```

Tests cover framing, echoed arguments, failures, partial updates, brightness
rounding, physical-shortcut preservation, all 83 layout entries, per-key
black/white, framework Custom selection and explicit-only palette commits.
Actual-device testing was performed on Debian 13 amd64. Windows/macOS hardware
operation, every individual physical key coordinate, and all effect appearances
have not been verified.

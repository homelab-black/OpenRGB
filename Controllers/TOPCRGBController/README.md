# TOPC RGB Controller

OpenRGB controller driver for the TOPC motherboard RGB / ARGB controller
(USB `VID_8888` / `PID_7A95`).

## Overview

This driver controls the single addressable 3-pin ARGB header found on
certain TOPC motherboards.  The firmware is driven **exactly like the vendor
reference implementation** (the original `backend/` Python code): the same
colour buffer is mirrored to **four HID feature reports** (`0x10`..`0x13`),
each followed by an apply report.

The device exposes only **direct per-LED control**: there is no on-device
effect engine and no dedicated brightness command.  Every dynamic effect is
therefore rendered **host side** by a background thread in the controller and
pushed to the device as raw per-LED RGB frames.

The number of connected LEDs is **user configurable** in the OpenRGB UI
(select the zone, then use the "Edit Zone" button) between **1 and 60** LEDs,
and defaults to `TOPC_RGB_LED_COUNT` (**60**) on first creation.  The protocol
always fills 60 LED slots per report (matching the reference code); if fewer
LEDs are configured the remaining slots are padded with the last colour and
simply drive nothing.

## Device identification

| Property          | Value                                              |
|-------------------|----------------------------------------------------|
| Vendor ID         | `0x8888`                                           |
| Product ID        | `0x7A95`                                           |
| HID usage page    | `0xFF01` (vendor-defined)                          |
| HID usage         | `0x0001`                                           |
| Zones             | 1 (single addressable header)                      |
| LEDs              | user configurable, 1..60 (default 60)              |
| Color format      | 8-bit RGB, 3 bytes per LED (no alpha/brightness)  |

The usage page/usage pair comes from the Windows `HardwareID`
`HID\VID_8888&PID_7A95&UP:FF01_U:0001`.  Filtering on it guarantees only a
single controller is created even if the device exposes additional HID top
level collections.

## Protocol

The device is driven through HID **feature reports**, each 256 bytes long.

### Color report (mirrored to four report IDs)

```
byte 0        report ID          0x10 .. 0x13
byte 1        0x00
byte 2        0x00
byte 3..182   60 * (R, G, B)
remaining     padding zeros
```

### Apply report

After each colour report an apply report must be sent so the device latches
the new colours:

```
byte 0        0x01
byte 1        0x00
byte 2        0x88
byte 3        0xFF
remaining     padding zeros
```

The same colour buffer is written once for every report ID (`0x10`..`0x13`),
applying after each, exactly like the reference code.  If fewer colours than
60 are supplied, the last colour is repeated to fill the report; colours
beyond 60 are ignored.

## Modes

All modes are rendered **host side** by a background thread
(`EffectThreadFunction`) at ~30 fps.  Each frame is written straight to the
hardware through the driver.  This mirrors what `backend/effects/*.py` does.

| Mode         | Colours        | Parameters                  | Notes                             |
|--------------|----------------|-----------------------------|-----------------------------------|
| Direct       | per-LED        | -                           | UI drives each LED directly       |
| Static       | mode specific  | brightness                  | Single fixed colour               |
| Rainbow Wave | none           | speed, brightness, direction| Moving rainbow (flowing)          |
| Breathing    | mode specific  | speed, brightness, random   | Fade in/out                       |
| Meteor       | mode specific  | speed, brightness, random   | Comet / moving tail               |
| Blink        | mode specific  | speed, brightness, random   | Alternate on/off                  |

Selecting **Direct** stops the renderer so the UI's per-LED colours are sent
verbatim; selecting any other mode enables the renderer.

## Source layout

```
Controllers/TOPCRGBController/
├── TOPCRGBController.h          Low level HID driver
├── TOPCRGBController.cpp        Protocol implementation
├── RGBController_TOPCRGB.h      RGBController wrapper
├── RGBController_TOPCRGB.cpp    Zone / mode / software effect renderer
└── TOPCRGBControllerDetect.cpp  Detector and registration
```

### Driver layer (`TOPCRGBController`)

* `SetLEDs(colors, num_colors)` - builds the 256 byte colour report and sends
  it for every report ID (`0x10`..`0x13`), applying after each.  Thread safe
  (guarded by a mutex).

### Controller layer (`RGBController_TOPCRGB`)

* Creates one `ZONE_TYPE_LINEAR` zone that is **user resizable** in the UI
  (select the zone and use the "Edit Zone" button;
  `ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE` / `NAME` / `TYPE`), between 1 and 60
  LEDs.  The default size on first creation is `TOPC_RGB_LED_COUNT` (60).
* `DeviceConfigureZone()` rebuilds the LED/color arrays whenever the user
  changes the zone size; the persisted count is preserved via
  `ZONE_FLAG_MANUALLY_CONFIGURED_SIZE`.
* Declares the modes listed above.
* Owns a background thread that renders the non-Direct modes in software and
  pushes each frame through `SetLEDs()`.

### Driver constants (`TOPCRGBController.h`)

| Macro                     | Meaning                                              |
|---------------------------|------------------------------------------------------|
| `TOPC_RGB_LED_COUNT`      | Default LEDs exposed to OpenRGB (60)                 |
| `TOPC_RGB_LEDS_PER_REPORT`| LED slots filled in every report (60, from reference)|
| `TOPC_RGB_REPORT_ID_BASE` | First colour report ID (`0x10`)                      |
| `TOPC_RGB_REPORT_ID_COUNT`| Number of colour reports (`4`)                       |
| `TOPC_RGB_REPORT_SIZE`    | HID feature report size (256)                        |
| `TOPC_RGB_DEBUG_LOG`      | Set to 1 for verbose per-report debug logging        |

## Design references

* `Controllers/GoveeController/` - keepalive thread + software-rendered
  effects on a direct-only strip.
* `Controllers/MountainKeyboardController/` - controller-owned effect/update
  thread pattern.
* `Controllers/ElgatoLightStripController/` and `qt/hsv.h` - HSV handling used
  by the Rainbow effect.
* `Controllers/HYTEKeyboardController/` - `REGISTER_HID_DETECTOR_PU`
  usage/usage-page filtered detector.


## Building OpenRGB with this controller

No build file changes are needed: `OpenRGB.pro` collects controller sources
automatically via `$$files("Controllers/*.cpp"/"*.h", true)`.

### Windows (native, recommended for this device)

1. Install [Git](https://git-scm.com/download).
2. Install the **Microsoft Visual Studio Build Tools** (2019 or 2022).
3. Clone the Qt package repository and run the installer:

   ```bat
   git clone https://gitlab.com/OpenRGBDevelopers/OpenRGB-Qt-Packages
   cd OpenRGB-Qt-Packages
   install.bat
   ```

   (`install-chocolatey.bat` if you use Chocolatey.)  This installs Qt under
   `C:\Qt\<version>\msvc<version>_64\` plus `jom`.  Provided versions include
   `5.15.0` (MSVC 2019) and `6.8.3` (MSVC 2022).

4. From the OpenRGB source root run:

   ```bat
   scripts\build-windows.bat 5.15.0 2019 64
   ```

   or for Qt 6:

   ```bat
   scripts\build-windows.bat 6.8.3 2022 64
   ```

   The script sets `PATH`, runs `vcvarsall`, `qmake OpenRGB.pro
   CONFIG+=release`, `jom` and finally `windeployqt`.

5. The result is placed in a folder named
   `OpenRGB Windows 64-bit`.  Run `OpenRGB.exe` from there.

You can also open `OpenRGB.pro` directly in Qt Creator and build/debug from
there.

### Linux (for reference)

```bash
sudo apt install git build-essential qtcreator qtbase5-dev qtchooser qt5-qmake \
    qtbase5-dev-tools libusb-1.0-0-dev libhidapi-dev pkgconf libmbedtls-dev \
    qttools5-dev-tools
mkdir build && cd build
qmake ../OpenRGB.pro
make -j$(nproc)
./openrgb
```

Remember to install the udev rules (see `Documentation/UdevRules.md`) so the
device can be opened without root.

## Verifying the device on Windows

List the relevant HID interfaces / usage pages:

```powershell
Get-PnpDevice -PresentOnly -Class HIDClass | Where-Object {
    $_.InstanceId -match 'VID_8888&PID_7A95'
} | Select-Object -ExpandProperty HardwareID
```

Inspect all properties:

```powershell
Get-PnpDevice -PresentOnly | Where-Object {
    $_.InstanceId -match 'VID_8888&PID_7A95'
} | Format-List *
```


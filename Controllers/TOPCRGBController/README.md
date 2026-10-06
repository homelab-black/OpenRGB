# TOPC RGB Controller

OpenRGB controller driver for the TOPC motherboard RGB / ARGB controller
(USB `VID_8888` / `PID_7A95`).

## Overview

This driver controls the single addressable 3-pin ARGB header found on
certain TOPC motherboards.  The header is intended to drive a user supplied
strip or fan chain, so the number of connected LEDs is **manually
configurable** in the OpenRGB UI (per-zone sizing, 0-40 LEDs).

The device only exposes **direct per-LED control**.  There is no on-device
effect engine and no dedicated brightness command, so every dynamic effect is
rendered host side by OpenRGB and pushed to the device as raw per-LED RGB
values.

## Device identification

| Property          | Value                                              |
|-------------------|----------------------------------------------------|
| Vendor ID         | `0x8888`                                           |
| Product ID        | `0x7A95`                                           |
| HID usage page    | `0xFF01` (vendor-defined)                          |
| HID usage         | `0x0001`                                           |
| Channels          | 1                                                  |
| Max LEDs / channel| 40                                                 |
| Color format      | 8-bit RGB, 3 bytes per LED (no alpha/brightness)  |

The usage page/usage pair comes from the Windows `HardwareID`
`HID\VID_8888&PID_7A95&UP:FF01_U:0001`.  Filtering on it guarantees only a
single controller is created even if the device exposes additional HID top
level collections.

## Protocol

The device is driven through HID **feature reports**, each 256 bytes long.

### Color report (one per channel)

```
byte 0        report ID          0x10 + channel  (channel 0 -> 0x10)
byte 1        0x00
byte 2        0x00
byte 3..N     LED colors, 3 bytes each (R, G, B)
remaining     padding zeros
```

### Apply report

After a color report has been written an apply report must be sent so the
device latches the new colors:

```
byte 0        0x01
byte 1        0x00
byte 2        0x88
byte 3        0xFF
remaining     padding zeros
```

If fewer colors than the channel size are supplied, the last color is repeated
to fill the channel.  Supplied colors beyond the channel maximum are ignored.

## Source layout

```
Controllers/TOPCRGBController/
├── TOPCRGBController.h          Low level HID driver
├── TOPCRGBController.cpp        Protocol implementation
├── RGBController_TOPCRGB.h      RGBController wrapper
├── RGBController_TOPCRGB.cpp    Zone / mode / update logic
├── TOPCRGBControllerDetect.cpp  Detector and registration
└── backend/                     Original Python reference implementation
```

### Driver layer (`TOPCRGBController`)

* `SetChannelLEDs(channel, colors)` - builds and sends the color report for a
  channel followed by the apply report.  Thread safe (guarded by a mutex).
* `SetAllChannels(colors)` - calls `SetChannelLEDs()` for every channel.

### Controller layer (`RGBController_TOPCRGB`)

* Creates one `ZONE_TYPE_LINEAR` zone per channel, each marked with
  `ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE` / `NAME` / `TYPE` / `MATRIX_MAP` /
  `SEGMENTS` so the user can set the real LED count and layout in the UI.
* Provides a single `Direct` mode (`MODE_FLAG_HAS_PER_LED_COLOR`,
  `MODE_COLORS_PER_LED`).
* `DeviceConfigureZone()` rebuilds the LED/color arrays whenever the user
  changes a zone size.

## Design references

* `Controllers/CoolerMasterController/CMARGBController/` - direct-only ARGB
  controller with manually configurable zones (structural template).
* `Controllers/HYTEKeyboardController/` - `REGISTER_HID_DETECTOR_PU`
  usage/usage-page filtered detector.
* `backend/effects/*.py` - reverse engineered protocol details.

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


/*---------------------------------------------------------*\
| MSIGE76Controller.h                                       |
|                                                           |
|   Driver for MSI GE76 Raider SteelSeries KLC/ALC         |
|   KLC = Keyboard Light Controller (VID 0x1038 PID 0x1122)|
|   ALC = Ambient Light Controller  (VID 0x1038 PID 0x1134)|
|                                                           |
|   Protocol reverse-engineered from msi-perkeyrgb:        |
|     https://github.com/Askannz/msi-perkeyrgb             |
|   Color: hid_send_feature_report, 524 bytes,             |
|     buf[0]=0x0e (cmd), buf[2]=region_id                  |
|   Effect config: same length, buf[0]=0x0b (msi-perkeyrgb |
|     captures — loads animation layers on the controller) |
|   Commit: hid_write, 64 bytes, buf[0]=0x09               |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "MSIGE76HardwarePresets.h"
#include "RGBController.h"

/*---------------------------------------------------------*\
| Packet sizes (no report-ID prefix – cmd byte is at [0])  |
\*---------------------------------------------------------*/
#define MSIGE76_COLOR_PKT_SIZE      524    /* feature report: [0x0e, 0x00, region, 0x00, ...] */
#define MSIGE76_COMMIT_PKT_SIZE     64     /* output report:  [0x09, 0x00, ...]               */

/*---------------------------------------------------------*\
| Command bytes (buf[0])                                    |
\*---------------------------------------------------------*/
#define MSIGE76_CMD_COLOR           0x0e   /* set key colours, per region  */
#define MSIGE76_CMD_EFFECT          0x0b   /* effect layer / animation cfg */
#define MSIGE76_CMD_COMMIT          0x09   /* apply / refresh              */

#define MSIGE76_BRIGHTNESS_MIN      0x00
#define MSIGE76_BRIGHTNESS_MAX      0x10   /* legacy scale for SetColors API */

/*---------------------------------------------------------*\
| Region IDs (buf[2] of the colour packet)                  |
\*---------------------------------------------------------*/
#define MSIGE76_REGION_ALPHANUM     0x2a
#define MSIGE76_REGION_ENTER        0x0b
#define MSIGE76_REGION_MODIFIERS    0x18
#define MSIGE76_REGION_NUMPAD       0x24

/*---------------------------------------------------------*\
| Key-fragment layout constants                             |
| Each of the 42 slots in a colour packet is 12 bytes:     |
|   [R, G, B, reactR, reactG, reactB, 0, 0, 0, mode, 0, kc]|
| mode = 0x01 → inline/static colour                       |
\*---------------------------------------------------------*/
#define MSIGE76_KEYS_PER_REGION     42
#define MSIGE76_KEY_ENTRY_SIZE      12
#define MSIGE76_MODE_STATIC         0x01

class MSIGE76Controller
{
public:
    MSIGE76Controller(hid_device* dev_handle, const char* path,
                      const std::string& dev_name, unsigned int led_count);
    ~MSIGE76Controller();

    std::string     GetDeviceLocation();
    std::string     GetSerialString();
    std::string     GetDeviceName();
    unsigned int    GetLEDCount();

    void            SetAllColor(RGBColor color);

    /*-------------------------------------------------*\
    | KLC: per-LED colours (full matrix, see MSIGE76KeyboardLayout). |
    | ALC: per-zone colours (29 zones, numpad region; see |
    |      SetColorsALC — slots 0–28, HID keycodes 1–29)   |
    | brightness parameter ignored; luminance is from RGB.   |
    \*-------------------------------------------------*/
    void            SetColors(const std::vector<RGBColor>& colors, uint8_t brightness = MSIGE76_BRIGHTNESS_MAX);

    /*-------------------------------------------------*\
    | KLC / ALC: replay msi-perkeyrgb capture. Not built |
    | from GE76 matrix — Direct mode uses the layout.   |
    | Before non-default presets: static black +        |
    | \"default\" capture to avoid stacked 0x0b layers.  |
    \*-------------------------------------------------*/
    void            ApplyHardwarePreset(unsigned int preset_index);

private:
    void            SendOneHardwarePreset(const MSIGE76HardwarePresetDesc& preset);
    void            SendRegionPacket(uint8_t region_id,
                                     const uint8_t keycodes[MSIGE76_KEYS_PER_REGION],
                                     const RGBColor slot_colors[MSIGE76_KEYS_PER_REGION]);
    void            Commit();

    void            SetColorsKLC(const std::vector<RGBColor>& colors, bool single_commit_flush = false);
    void            SetColorsALC(const std::vector<RGBColor>& colors);
    void            SetColorsSolidSweep(RGBColor);

    hid_device*     dev;
    std::string     location;
    std::string     name;
    unsigned int    num_leds;
};

/*---------------------------------------------------------*\
| Zotac4AGPUController.h                                    |
|                                                           |
|   Driver for Zotac 4A GPU                                 |
|                                                           |
|   Peter Berendi                               29 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "i2c_smbus.h"
#include "RGBController.h"

#define ZOTAC_4A_GPU_WAIT_BETWEEN_REQUESTS std::chrono::milliseconds(200)

#define ZOTAC_4A_RGB_ID_LEN 10

enum
{
    ZOTAC_4A_GPU_REG_RGB = 0x5A,
};

#define ZOTAC_4A_RGB_ID_LEN 10

#define ZOTAC_4A_RGB_MAXSPEED 0xFE

/*---------------------------------------------------------------------------*\
| Layout of the control and status packets.                                   |
|                                                                             |
| In the status response, bytes 0 to 9 contain the controller identifier,     |
| followed by a copy of the last control packet starting at the on/off flag   |
| i.e. must add 9 to the offset when interpreting the response.               |
\*---------------------------------------------------------------------------*/
#define ZOTAC_4A_RGB_STATUS_OFFSET 9

#define ZOTAC_4A_GPU_RGB_CONTROL_LENGTH 10

enum
{
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_RESET     = 0,  // 0=no reset, 1=reset
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_ONOFF     = 1,  // 0=off, 1=on
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_MODE      = 2,  // ZOTAC_4A_GPU_MODE_* values
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_RED       = 3,
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_GREEN     = 4,
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_BLUE      = 5,
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_ON   = 6,  // LED ON time in Wink and Random mode, speed in other modes
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_OFF  = 7,  // LED OFF time in Wink and Random mode, apparently unused in other modes
    ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_PRESET    = 8,  // preset color index
};

enum
{
    ZOTAC_4A_GPU_MODE_STATIC           = 0x00,  // Basic static color
    ZOTAC_4A_GPU_MODE_BREATHE          = 0x01,  // Single color fades on and off
    ZOTAC_4A_GPU_MODE_FADE             = 0x02,  // All colors fade through the spectrum
    ZOTAC_4A_GPU_MODE_WINK             = 0x03,  // Single color flashes on and off
    ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE  = 0x04,  // Breathe effect, colors cycling through the spectrum (not in the OEM app)
    ZOTAC_4A_GPU_MODE_RANDOM           = 0x05,  // Random colors flash on and off
};

struct Zotac4AGPUZone
{
    int mode = 0;
    RGBColor color = ToRGBColor(0, 0, 0);
    u8 speed = ZOTAC_4A_RGB_MAXSPEED - 45;
    u8 on_time = 10;
};

class Zotac4AGPUController
{
public:
    Zotac4AGPUController(i2c_smbus_interface* bus, u8 dev, std::string detectedversion);
    ~Zotac4AGPUController();

    std::string GetDeviceLocation();
    std::string GetVersion();

    bool TurnOnOff(bool on);
    bool ResetToDefaults();
    bool GetMode(Zotac4AGPUZone& zoneConfig, bool& on);
    bool SetMode(Zotac4AGPUZone zoneConfig);

private:
    bool SendCommand(bool on, bool reset, Zotac4AGPUZone zoneConfig);
    bool ParseCommand(bool& on, Zotac4AGPUZone& zoneConfig);

    i2c_smbus_interface* bus;
    u8                   dev;
    std::string          version;
};

/*---------------------------------------------------------*\
| ZotacV3GPUController.h                                    |
|                                                           |
|   Driver for Zotac V3 GPU                                 |
|                                                           |
|   Based on ZotacV2GPUController                           |
|   Adapted for Zotac RTX 5070 SOLIC OC                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "i2c_smbus.h"
#include "RGBController.h"

enum
{
    ZOTAC_V3_GPU_REG_RGB = 0x5A,
};

enum
{
    ZOTAC_V3_GPU_MODE_STATIC  = 0x00,  // Basic static color
    ZOTAC_V3_GPU_MODE_BREATH  = 0x01,  // Single color fades on and off
    ZOTAC_V3_GPU_MODE_FADE    = 0x02,  // All colors fade through the spectrum
    ZOTAC_V3_GPU_MODE_WINK    = 0x03,  // Single color flashes on and off
    ZOTAC_V3_GPU_MODE_RANDOM  = 0x05,  // Single color, random patern
};

enum
{
    ZOTAC_V3_GPU_CIRCUIT_ON  = 0x00,
    ZOTAC_V3_GPU_CIRCUIT_OFF = 0x01,
};

struct ZotacV3GPUConfig
{
    int numberOfZones = 0;
    bool supportsExternalLEDStrip = false;
};

struct ZotacV3GPUZone
{
    int mode = 0;
    RGBColor color1 = ToRGBColor(0, 0, 0);
    unsigned int colorPreset = 0;
};

class ZotacV3GPUController
{
public:
    ZotacV3GPUController(i2c_smbus_interface* bus, u8 dev, std::string dev_name);
    ~ZotacV3GPUController();

    std::string             GetDeviceLocation();
    std::string             GetName();
    std::string             GetVersion();

    bool                    ResetToDefaults();
    bool                    GetMode(ZotacV3GPUZone& zoneConfig, bool& on);
    bool                    SetMode(ZotacV3GPUZone zoneConfig);

private:
    i2c_smbus_interface*    bus;
    u8                      dev;
    std::string             name;
    std::string             version;

    bool                    ReadVersion();
    bool                    SendCommand(bool on, bool reset, ZotacV3GPUZone zoneConfig);
    bool                    ParseCommand(bool& on, bool& reset, ZotacV3GPUZone& zoneConfig);
};

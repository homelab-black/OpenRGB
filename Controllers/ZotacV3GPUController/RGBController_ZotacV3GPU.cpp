/*---------------------------------------------------------*\
| RGBController_ZotacV3GPU.cpp                              |
|                                                           |
|   RGBController for Zotac V3 GPU                          |
|                                                           |
|   Based on RGBController_ZotacV2GPU                       |
|   Adapted for Zotac RTX 5070 SOLIC OC                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <map>
#include "RGBController_ZotacV3GPU.h"
#include "LogManager.h"

std::map<std::string, ZotacV3GPUConfig> ZOTAC_V3_GPU_CONFIG =
{
    { "N764E-1001", { 1, false } },  // ZOTAC GAMING GeForce RTX 5070 SOLID OC
};

/**------------------------------------------------------------------*\
    @name ZOTAC 50 series GPU
    @category GPU
    @type I2C
    @save :robot:
    @direct :x:
    @effects :tools:
    @detectors DetectZotacV3GPUControllers
    @comment
        To add new cards, in addition to entries in `pci_ids/pci_ids.h`
        and `Controllers/ZotacV3GPUController/ZotacV3GPUControllerDetect.cpp`
        an entry associating the controller version with the LED
        configuration must be added to the [ZotacV3GPUConfig map](https://gitlab.com/CalcProgrammer1/OpenRGB/-/blob/master/Controllers/ZotacV3GPUController/RGBController_ZotacV3GPU.cpp?ref_type=heads#L14) in
        `Controllers/ZotacV3GPUController/RGBController_ZotacV3GPU.cpp`.

        Controller version is identified by polling the controller on address
        0x4A(so far for all known cards) at register 0x5A.

        The first ten bytes when read back from that address are the version,
        this must be converted into Unicode.

        The polling and reading is done by the ZOTAC Firestorm app on startup
        and can be monitored using NvAPIspy on Windows.

        For RTX 5070 SOLIC OC 12GB the relevant NvAPISpy log entries are:

        > ```plaintext
        > NvAPI_I2CRead:  Dev: 0x94 RegSize: 0x01 Reg: 0x5A Size: 0x20 Data: 0x4E 0x37 0x36 0x34 0x45 0x2D 0x31 0x30 0x30 0x31 0x01 0x01 0xFF 0xFF 0x00 0x14 0x0A 0x02 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
        > ```

        The read data `0x4E 0x37 0x36 0x34 0x45 0x2D 0x31 0x30 0x30 0x31` converts
        to Unicode `N612E-1011`. The remaining bytes are the current settings.

        Cards with correct entries in `pci_ids/pci_ids.h` and `Controllers/ZotacV3GPUController/ZotacV3GPUControllerDetect.cpp`
        but not in `Controllers/ZotacV3GPUController/RGBController_ZotacV3GPU.cpp`
        should produce a log error with the controller version read from the card.
\*-------------------------------------------------------------------*/

RGBController_ZotacV3GPU::RGBController_ZotacV3GPU(ZotacV3GPUController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetName();
    vendor                  = "ZOTAC";
    description             = "ZOTAC 50 series RGB GPU Device (" + controller->GetVersion() + ")";
    location                = controller->GetDeviceLocation();
    type                    = DEVICE_TYPE_GPU;

    if(ZOTAC_V3_GPU_CONFIG.count(controller->GetVersion()) > 0)
    {
        config = ZOTAC_V3_GPU_CONFIG.at(controller->GetVersion());
    }
    else
    {
        LOG_ERROR("[%s] Unrecognized controller version %s", name.c_str(), controller->GetVersion().c_str());
        config = { 0, false };
    }


    version += std::to_string(config.numberOfZones) + " zones, "
            + (config.supportsExternalLEDStrip ? "with" : "without") + " external LED strip support";

    mode STATIC;
    STATIC.name             = "Static";
    STATIC.value            = ZOTAC_V3_GPU_MODE_STATIC;
    STATIC.flags            = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    STATIC.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    STATIC.colors_min       = 1;
    STATIC.colors_max       = 1;
    STATIC.colors.resize(1);
    STATIC.colors[0]        = ToRGBColor(0, 0, 255);
    modes.push_back(STATIC);

    mode BREATH;
    BREATH.name             = "Breath";
    BREATH.value            = ZOTAC_V3_GPU_MODE_BREATH;
    BREATH.flags            = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    BREATH.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    BREATH.colors_min       = 1;
    BREATH.colors_max       = 1;
    BREATH.colors.resize(1);
    BREATH.colors[0]        = ToRGBColor(0, 0, 255);
    modes.push_back(BREATH);

    mode FADE;
    FADE.name               = "Fade";
    FADE.value              = ZOTAC_V3_GPU_MODE_FADE;
    FADE.flags              = MODE_FLAG_AUTOMATIC_SAVE;
    FADE.color_mode         = MODE_COLORS_NONE;
    modes.push_back(FADE);

    mode WINK;
    WINK.name               = "Wink";
    WINK.value              = ZOTAC_V3_GPU_MODE_WINK;
    WINK.flags              = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    WINK.color_mode         = MODE_COLORS_MODE_SPECIFIC;
    WINK.colors_min         = 1;
    WINK.colors_max         = 1;
    WINK.colors.resize(1);
    WINK.colors[0]          = ToRGBColor(0, 0, 255);
    modes.push_back(WINK);

    mode RANDOM;
    RANDOM.name             = "Random";
    RANDOM.value            = ZOTAC_V3_GPU_MODE_RANDOM;
    RANDOM.flags            = MODE_FLAG_AUTOMATIC_SAVE;
    RANDOM.color_mode       = MODE_COLORS_NONE;
    modes.push_back(RANDOM);

    SetupZones();
}

RGBController_ZotacV3GPU::~RGBController_ZotacV3GPU()
{
    delete controller;
}

void RGBController_ZotacV3GPU::SetupZones()
{
    led new_led;
    new_led.name           = "GPU LED";
    leds.push_back(new_led);

    zone new_zone;
    new_zone.name          = "GPU Zone";
    new_zone.type          = ZONE_TYPE_SINGLE;
    new_zone.leds_min      = 1;
    new_zone.leds_max      = 1;
    new_zone.leds_count    = 1;
    new_zone.matrix_map    = NULL;
    zones.push_back(new_zone);

    SetupColors();
    SetupInitialValues();
}

void RGBController_ZotacV3GPU::SetupInitialValues()
{
    /*---------------------------------------------------------*\
    | Retrieve current values by reading the device             |
    \*---------------------------------------------------------*/

    bool on;
    ZotacV3GPUZone zoneConfig;

    if(!controller->GetMode(zoneConfig, on))
    {
        return;
    }

    for(unsigned int i = 0; i < modes.size(); ++i)
    {
        if(zoneConfig.mode != modes[i].value)
        {
            continue;
        }

        active_mode = i;
    }

    colors[0] = zoneConfig.color1;
    if(modes[active_mode].colors.size() >= 1)
    {
        modes[active_mode].colors[0] = zoneConfig.color1;
    }

    SignalUpdate();
}

void RGBController_ZotacV3GPU::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_ZotacV3GPU::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_ZotacV3GPU::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_ZotacV3GPU::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_ZotacV3GPU::DeviceUpdateMode()
{
    ZotacV3GPUZone zoneConfig;
    zoneConfig.mode = modes[active_mode].value;

    zoneConfig.color1 = modes[active_mode].colors.size() >= 1 ? modes[active_mode].colors[0] : ToRGBColor(0, 0, 0);
    zoneConfig.colorPreset = 8;

    controller->SetMode(zoneConfig);
}

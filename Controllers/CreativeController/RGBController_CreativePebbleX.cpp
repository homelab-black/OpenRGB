/*---------------------------------------------------------*\
| RGBController_CreativePebbleX.cpp                           |
|                                                           |
|   RGBController for Creative Pebble X                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_CreativePebbleX.h"

/**------------------------------------------------------------------*\
    @name Creative Pebble X
    @category Speaker
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectCreativePebbleXDevice
    @comment
\*-------------------------------------------------------------------*/

RGBController_CreativePebbleX::RGBController_CreativePebbleX(CreativePebbleXController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = "Creative Pebble X Device";
    vendor                      = "Creative";
    type                        = DEVICE_TYPE_SPEAKER;
    description                 = "Creative Pebble X Device";
    location                    = controller->GetDeviceLocation();
    serial                      = "";

    mode Static;
    Static.name                 = "Direct";
    Static.value                = 0;
    Static.flags                = MODE_COLORS_PER_LED | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_PER_LED;
    Static.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Static.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Static.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    modes.push_back(Static);

    SetupZones();
}

RGBController_CreativePebbleX::~RGBController_CreativePebbleX()
{
    delete controller;
}

void RGBController_CreativePebbleX::SetupZones()
{
    zone pebble_speaker_zone;
    pebble_speaker_zone.name                  = "Speakers";
    pebble_speaker_zone.type                  = ZONE_TYPE_LINEAR;
    pebble_speaker_zone.leds_min              = 6;
    pebble_speaker_zone.leds_max              = 6;
    pebble_speaker_zone.leds_count            = 6;
    pebble_speaker_zone.matrix_map            = NULL;
    zones.push_back(pebble_speaker_zone);

    for (int i = 0; i < 6; ++i)
    {
        led led;
        led.name = "LED " + std::to_string(i + 1);
        leds.push_back(led);
    }

    SetupColors();
}

void RGBController_CreativePebbleX::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_CreativePebbleX::DeviceUpdateLEDs()
{
    controller->UpdateDeviceLEDs(colors);
}

void RGBController_CreativePebbleX::UpdateZoneLEDs(int zone)
{
    DeviceUpdateLEDs();
}

void RGBController_CreativePebbleX::UpdateSingleLED(int led)
{
    DeviceUpdateLEDs();
}

void RGBController_CreativePebbleX::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

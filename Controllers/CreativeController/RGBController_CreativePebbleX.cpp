/*---------------------------------------------------------*\
| RGBController_CreativePebbleX.cpp                         |
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
    Static.value                = 0x03;
    Static.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_PER_LED;
    Static.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Static.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Static.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode Chasers;
    Chasers.name                 = "Chasers";
    Chasers.value                = 0x0B;
    Chasers.flags                = MODE_COLORS_MODE_SPECIFIC | MODE_FLAG_HAS_SPEED; // | MODE_FLAG_HAS_BRIGHTNESS;
    Chasers.color_mode           = 2;
    // Chasers.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    // Chasers.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    // Chasers.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Chasers.speed                = 0x00;
    Chasers.speed_max            = 0xFF;
    Chasers.speed_min            = 0x00;
    modes.push_back(Chasers);

    mode Aurora;
    Aurora.name                 = "Aurora";
    Aurora.value                = 0x08;
    Aurora.flags                = MODE_COLORS_MODE_SPECIFIC | MODE_FLAG_HAS_SPEED; // | MODE_FLAG_HAS_BRIGHTNESS;
    Aurora.color_mode           = 0;
    // Aurora.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    // Aurora.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    // Aurora.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Aurora.speed                = 0x00;
    Aurora.speed_max            = 0xFF;
    Aurora.speed_min            = 0x00;
    modes.push_back(Aurora);

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
    pebble_speaker_zone.leds_min              = 5;
    pebble_speaker_zone.leds_max              = 5;
    pebble_speaker_zone.leds_count            = 5;
    pebble_speaker_zone.matrix_map            = NULL;
    zones.push_back(pebble_speaker_zone);

    for (int i = 0; i < 5; ++i)
    {
        led led;
        led.name = "LED " + std::to_string(i + 1);
        leds.push_back(led);
    }

    SetupColors();
}


// void RGBController_CreativePebbleX::SetMode(unsigned char mode)
// {
//     controller->UpdateDeviceMode(mode);
// }


void RGBController_CreativePebbleX::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}


void RGBController_CreativePebbleX::DeviceUpdateLEDs()
{
    unsigned char red = RGBGetRValue(colors[0]);
    unsigned char grn = RGBGetGValue(colors[0]);
    unsigned char blu = RGBGetBValue(colors[0]);

    controller->UpdateDeviceLEDs(red, grn, blu, modes[active_mode].brightness);
    // controller->UpdateDeviceLEDs(colors, modes[active_mode].brightness); // workaround for dirty ????
}


void RGBController_CreativePebbleX::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}


void RGBController_CreativePebbleX::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}


void RGBController_CreativePebbleX::DeviceUpdateMode()
{
    // if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    // {
    //     controller->SetBrightness(modes[active_mode].brightness);
    // }
    // else
    // {
    //     controller->SetBrightness(0xFF);
    // }
    // controller->UpdateDeviceMode(modes[active_mode].value);
    DeviceUpdateLEDs();
}

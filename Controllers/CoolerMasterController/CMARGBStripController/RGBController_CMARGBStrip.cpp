/*---------------------------------------------------------*\
| RGBController_CMARGBStrip.cpp                             |
|                                                           |
|   RGBController for Cooler Master ARGB light strip        |
|   devices                                                 |
|                                                           |
|   Morgan Guimard (morg)                       18 Sep 2023 |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_CMARGBStrip.h"

RGBController_CMARGBStrip::RGBController_CMARGBStrip(CMARGBStripController* controller_ptr, device_type dev_type)
{
    controller                  = controller_ptr;

    name                        = controller->GetDeviceName();
    vendor                      = "Cooler Master";
    type                        = dev_type;
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();

    /*---------------------------------------------------------*\
    | Set description based on device type                      |
    \*---------------------------------------------------------*/

    if(type == DEVICE_TYPE_MONITOR)
    {
        description         = "Cooler Master ARGB Monitor Device";
    }
    else
    {
        description         = "Cooler Master ARGB Gaming Desk Device";
    }

    /*---------------------------------------------------------*\
    | Set up modes                                              |
    \*---------------------------------------------------------*/
    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = CM_ARGB_STRIP_DIRECT_MODE;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Spectrum;
    Spectrum.name               = "Spectrum Cycle";
    Spectrum.value              = CM_ARGB_STRIP_SPECTRUM_MODE;
    Spectrum.flags              = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Spectrum.color_mode         = MODE_COLORS_NONE;
    Spectrum.speed_min          = CM_ARGB_STRIP_SPEED_MIN;
    Spectrum.speed_max          = CM_ARGB_STRIP_SPEED_MAX;
    Spectrum.speed              = CM_ARGB_STRIP_SPEED_MAX/2;
    Spectrum.brightness_min     = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Spectrum.brightness_max     = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Spectrum.brightness         = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Spectrum);

    mode Reload;
    Reload.name                 = "Reload";
    Reload.value                = CM_ARGB_STRIP_RELOAD_MODE;
    Reload.flags                = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    Reload.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Reload.colors_min           = 1;
    Reload.colors_max           = 1;
    Reload.colors.resize(1);
    Reload.speed_min            = CM_ARGB_STRIP_SPEED_MIN;
    Reload.speed_max            = CM_ARGB_STRIP_SPEED_MAX;
    Reload.speed                = CM_ARGB_STRIP_SPEED_MAX/2;
    Reload.brightness_min       = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Reload.brightness_max       = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Reload.brightness           = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Reload);

    mode Recoil;
    Recoil.name                 = "Recoil";
    Recoil.value                = CM_ARGB_STRIP_RECOIL_MODE;
    Recoil.flags                = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    Recoil.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Recoil.colors_min           = 1;
    Recoil.colors_max           = 1;
    Recoil.colors.resize(1);
    Recoil.speed_min            = CM_ARGB_STRIP_SPEED_MIN;
    Recoil.speed_max            = CM_ARGB_STRIP_SPEED_MAX;
    Recoil.speed                = CM_ARGB_STRIP_SPEED_MAX/2;
    Recoil.brightness_min       = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Recoil.brightness_max       = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Recoil.brightness           = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Recoil);

    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = CM_ARGB_STRIP_BREATHING_MODE;
    Breathing.flags             = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    Breathing.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min        = 1;
    Breathing.colors_max        = 1;
    Breathing.colors.resize(1);
    Breathing.speed_min         = CM_ARGB_STRIP_SPEED_MIN;
    Breathing.speed_max         = CM_ARGB_STRIP_SPEED_MAX;
    Breathing.speed             = CM_ARGB_STRIP_SPEED_MAX/2;
    Breathing.brightness_min    = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Breathing.brightness_max    = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Breathing.brightness        = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Breathing);

    mode Refill;
    Refill.name                 = "Refill";
    Refill.value                = CM_ARGB_STRIP_REFILL_MODE;
    Refill.flags                = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    Refill.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Refill.colors_min           = 1;
    Refill.colors_max           = 1;
    Refill.colors.resize(1);
    Refill.speed_min            = CM_ARGB_STRIP_SPEED_MIN;
    Refill.speed_max            = CM_ARGB_STRIP_SPEED_MAX;
    Refill.speed                = CM_ARGB_STRIP_SPEED_MAX/2;
    Refill.brightness_min       = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Refill.brightness_max       = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Refill.brightness           = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Refill);

    mode Custom;
    Custom.name                 = "Custom";
    Custom.value                = CM_ARGB_STRIP_CUSTOM_MODE;
    Custom.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_BRIGHTNESS;
    Custom.color_mode           = MODE_COLORS_PER_LED;
    Custom.brightness_min       = CM_ARGB_STRIP_BRIGHTNESS_MIN;
    Custom.brightness_max       = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    Custom.brightness           = CM_ARGB_STRIP_BRIGHTNESS_MAX;
    modes.push_back(Custom);

    mode Off;
    Off.name                    = "Off";
    Off.value                   = CM_ARGB_STRIP_OFF_MODE;
    Off.flags                   = MODE_FLAG_AUTOMATIC_SAVE;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_CMARGBStrip::~RGBController_CMARGBStrip()
{
    if(modes[active_mode].value == CM_ARGB_STRIP_DIRECT_MODE)
    {
        controller->SendColorData(colors, 0x10, 0x80, CM_ARGB_STRIP_BRIGHTNESS_MAX, false);
    }

    delete controller;
}

void RGBController_CMARGBStrip::SetupZones()
{
    unsigned int strip_count = controller->GetStripCount();
    unsigned int leds_per_strip = controller->GetLEDCount() / strip_count;

    /*---------------------------------------------------------*\
    | Create zones for each strip                               |
    \*---------------------------------------------------------*/

    for(unsigned int strip = 0; strip < strip_count; strip++)
    {
        zone strip_zone;

        if(strip_count == 1)
        {
            strip_zone.name = (type == DEVICE_TYPE_MONITOR) ? "Monitor" : "Desk";
        }
        else if(strip == 0)
        {
            strip_zone.name = "Front Desk";
        }
        else
        {
            strip_zone.name = "Back Desk";
        }

        strip_zone.type          = ZONE_TYPE_LINEAR;
        strip_zone.leds_min      = leds_per_strip;
        strip_zone.leds_max      = leds_per_strip;
        strip_zone.leds_count    = leds_per_strip;
        strip_zone.matrix_map    = NULL;
        zones.push_back(strip_zone);

        for(unsigned int i = 0; i < leds_per_strip; i++)
        {
            led l;

            if(strip_count == 1)
            {
                l.name = "LED "       + std::to_string(i + 1);
            }
            else if(strip == 0)
            {
                l.name = "Front LED " + std::to_string(i + 1);
            }
            else
            {
                l.name = "Back LED " + std::to_string(i + 1);
            }

            l.value = strip * leds_per_strip + i;
            leds.push_back(l);
        }
    }

    SetupColors();

}

void RGBController_CMARGBStrip::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_CMARGBStrip::DeviceUpdateLEDs()
{
    switch(modes[active_mode].value)
    {
        case CM_ARGB_STRIP_DIRECT_MODE:
            controller->SendColorData(colors, 0x07, 0x01, 0xFF, true);
            break;

        case CM_ARGB_STRIP_CUSTOM_MODE:
            controller->SendColorData(colors, 0x10, 0x80, modes[active_mode].brightness, false);
            break;

        default:
            break;
    }
}

void RGBController_CMARGBStrip::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CMARGBStrip::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CMARGBStrip::DeviceUpdateMode()
{
    bool random = (modes[active_mode].color_mode == MODE_COLORS_RANDOM);

    switch(modes[active_mode].value)
    {
        case CM_ARGB_STRIP_OFF_MODE:
        case CM_ARGB_STRIP_SPECTRUM_MODE:
            controller->SetMode(modes[active_mode].value, modes[active_mode].speed, modes[active_mode].brightness, 0, false);
            break;

        case CM_ARGB_STRIP_RELOAD_MODE:
        case CM_ARGB_STRIP_RECOIL_MODE:
        case CM_ARGB_STRIP_BREATHING_MODE:
        case CM_ARGB_STRIP_REFILL_MODE:
            controller->SetMode(modes[active_mode].value, modes[active_mode].speed, modes[active_mode].brightness, modes[active_mode].colors[0], random);
            break;

        default:
            break;
    }
}

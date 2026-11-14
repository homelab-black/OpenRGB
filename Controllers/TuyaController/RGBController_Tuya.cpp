/*---------------------------------------------------------*\
| RGBController_Tuya.cpp                                     |
|                                                             |
|   RGBController for Tuya-based WiFi RGB lights              |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#include "RGBController_Tuya.h"
#include "hsv.h"

RGBController_Tuya::RGBController_Tuya(TuyaController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "Tuya WiFi Light " + controller->GetDeviceID().substr(0, 8);
    vendor      = "Tuya";
    type        = DEVICE_TYPE_LEDSTRIP;
    description = "Tuya-protocol WiFi RGB Light (compatible with LEDVANCE SMART+ and other Tuya OEM brands)";
    location    = controller->GetLocation();
    version     = "Local LAN protocol 3.3";

    mode Static;
    Static.name         = "Static";
    Static.value        = 0;
    Static.flags        = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Static.color_mode   = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min   = 1;
    Static.colors_max   = 1;
    Static.colors.resize(1);
    modes.push_back(Static);

    SetupZones();
}

RGBController_Tuya::~RGBController_Tuya()
{
    delete controller;
}

void RGBController_Tuya::SetupZones()
{
    zone strip;
    strip.name          = "Tuya Light";
    strip.type          = ZONE_TYPE_SINGLE;
    strip.leds_min      = 1;
    strip.leds_max      = 1;
    strip.leds_count    = 1;
    zones.push_back(strip);

    led strip_led;
    strip_led.name = "Tuya Light";
    leds.push_back(strip_led);

    SetupColors();
}

void RGBController_Tuya::DeviceConfigureZone(int /*zone_idx*/)
{

}

void RGBController_Tuya::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_Tuya::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_Tuya::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_Tuya::DeviceUpdateMode()
{
    if(colors.empty())
    {
        return;
    }

    hsv_t hsv;
    rgb2hsv(colors[0], &hsv);

    unsigned int hue        = (hsv.hue == (unsigned int)-1) ? 0 : hsv.hue;
    unsigned int saturation = hsv.saturation * 1000 / 255;
    unsigned int value      = hsv.value      * 1000 / 255;

    controller->SetColorHSV(hue, saturation, value);
}

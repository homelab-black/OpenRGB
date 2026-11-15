/*---------------------------------------------------------*\
| RGBController_GigabyteM6980X.cpp                          |
|                                                           |
|   RGBController for the Gigabyte M6980X mouse             |
|                                                           |
|   Tom Rehm (Tom-8192)                         09 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_GigabyteM6980X.h"

/**------------------------------------------------------------------*\
    @name Gigabyte M6980X
    @category Mouse
    @type USB
    @save :robot:
    @direct :x:
    @effects :x:
    @detectors DetectGigabyteM6980XControllers
    @comment The mouse stores one color per hardware profile; each
    profile is a zone.  Every channel has three levels (off, half,
    full), so colors are rounded to the nearest level.  Colors are
    only written when the rounded color differs from the stored one.
\*-------------------------------------------------------------------*/

RGBController_GigabyteM6980X::RGBController_GigabyteM6980X(GigabyteM6980XController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetNameString();
    vendor                  = "Gigabyte";
    type                    = DEVICE_TYPE_MOUSE;
    description             = "Gigabyte M6980X Mouse Device";
    location                = controller->GetDeviceLocation();
    serial                  = controller->GetSerialString();

    mode Static;
    Static.name             = "Static";
    Static.value            = 0x00;
    Static.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Static.color_mode       = MODE_COLORS_PER_LED;

    modes.push_back(Static);

    SetupZones();

    /*-----------------------------------------------------*\
    | Show the colors stored in the mouse                   |
    \*-----------------------------------------------------*/
    for(unsigned int profile_idx = 0; profile_idx < GIGABYTE_M6980X_PROFILES; profile_idx++)
    {
        controller->GetColor(profile_idx, &colors[profile_idx]);
    }
}

RGBController_GigabyteM6980X::~RGBController_GigabyteM6980X()
{
    Shutdown();

    delete controller;
}

void RGBController_GigabyteM6980X::SetupZones()
{
    const char* zone_names[GIGABYTE_M6980X_PROFILES] = { "Profile 1", "Profile 2", "Profile 3" };

    for(unsigned int profile_idx = 0; profile_idx < GIGABYTE_M6980X_PROFILES; profile_idx++)
    {
        zone new_zone;
        new_zone.name       = zone_names[profile_idx];
        new_zone.type       = ZONE_TYPE_SINGLE;
        new_zone.leds_min   = 1;
        new_zone.leds_max   = 1;
        new_zone.leds_count = 1;
        zones.push_back(new_zone);

        led new_led;
        new_led.name        = zone_names[profile_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_GigabyteM6980X::DeviceUpdateLEDs()
{
    for(unsigned int profile_idx = 0; profile_idx < GIGABYTE_M6980X_PROFILES; profile_idx++)
    {
        controller->SetColor(profile_idx, colors[profile_idx]);
    }
}

void RGBController_GigabyteM6980X::DeviceUpdateZoneLEDs(int zone)
{
    /*-----------------------------------------------------*\
    | Every zone has one LED, so zone and LED index match   |
    \*-----------------------------------------------------*/
    controller->SetColor(zone, colors[zone]);
}

void RGBController_GigabyteM6980X::DeviceUpdateSingleLED(int led)
{
    controller->SetColor(led, colors[led]);
}

void RGBController_GigabyteM6980X::DeviceUpdateMode()
{

}

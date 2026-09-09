/*---------------------------------------------------------*\
| RGBController_LogitechG300.cpp                            |
|                                                           |
|   RGBController for Logitech G300s Optical Gaming Mouse   |
|                                                           |
|   Aman Ali                                  08 Sep 2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_LogitechG300.h"

/**------------------------------------------------------------------*\
    @name Logitech G300s
    @category Mouse
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectLogitechMouseG300
    @comment The illumination has a single bit per channel, so only
        eight colors are reachable and any other color is snapped to
        the nearest of them.  Direct mode writes a volatile register
        which reverts on a power cycle; saving writes the color into
        the active onboard profile so that it persists.
\*-------------------------------------------------------------------*/

RGBController_LogitechG300::RGBController_LogitechG300(LogitechG300Controller* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "Logitech";
    type                            = DEVICE_TYPE_MOUSE;
    description                     = "Logitech Mouse Device";
    serial                          = controller->GetSerialString();

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = LOGITECH_G300_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_LogitechG300::~RGBController_LogitechG300()
{
    Shutdown();

    delete controller;
}

void RGBController_LogitechG300::SetupZones()
{
    /*-----------------------------------------------------*\
    | The illumination is a pair of strips on the sides of   |
    | the mouse, driven together from a single register.     |
    \*-----------------------------------------------------*/
    zone side_lights_zone;
    side_lights_zone.name       = "Side Lights";
    side_lights_zone.type       = ZONE_TYPE_SINGLE;
    side_lights_zone.leds_min   = 1;
    side_lights_zone.leds_max   = 1;
    side_lights_zone.leds_count = 1;
    zones.push_back(side_lights_zone);

    led g300_led;
    g300_led.name = "All";
    leds.push_back(g300_led);

    SetupColors();
}

void RGBController_LogitechG300::DeviceUpdateLEDs()
{
    controller->SetLED(colors[0]);
}

void RGBController_LogitechG300::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechG300::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechG300::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechG300::DeviceSaveMode()
{
    controller->Save(colors[0]);
}

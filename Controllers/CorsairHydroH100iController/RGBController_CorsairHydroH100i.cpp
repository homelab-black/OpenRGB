/*---------------------------------------------------------*\
| RGBController_CorsairHydroH100i.cpp                       |
|                                                           |
|   RGBController for Corsair H100i Hydro Series cooler     |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          27 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_CorsairHydroH100i.h"

/**------------------------------------------------------------------*\
    @name Corsair H100i
    @category Cooler
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectCorsairHydroH100iControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_CorsairHydroH100i::RGBController_CorsairHydroH100i(CorsairHydroH100iController* controller_ptr)
{
    controller      = controller_ptr;

    name            = controller->GetNameString();
    vendor          = "Corsair";
    description     = "Corsair H100i Hydro Series Device";
    type            = DEVICE_TYPE_COOLER;
    location        = controller->GetLocation();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = 0;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_CorsairHydroH100i::~RGBController_CorsairHydroH100i()
{
    delete controller;
}

void RGBController_CorsairHydroH100i::SetupZones()
{
    zone new_zone;

    new_zone.name       = "Pump Zone";
    new_zone.type       = ZONE_TYPE_SINGLE;
    new_zone.leds_min   = 1;
    new_zone.leds_max   = 1;
    new_zone.leds_count = 1;
    new_zone.matrix_map = NULL;
    zones.push_back(new_zone);

    led new_led;

    new_led.name        = "Pump LED";
    leds.push_back(new_led);

    SetupColors();
}

void RGBController_CorsairHydroH100i::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_CorsairHydroH100i::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_CorsairHydroH100i::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CorsairHydroH100i::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CorsairHydroH100i::DeviceUpdateMode()
{
    switch(modes[active_mode].value)
    {
        case 0:
            controller->SetFixed(colors);
            break;
    }
}

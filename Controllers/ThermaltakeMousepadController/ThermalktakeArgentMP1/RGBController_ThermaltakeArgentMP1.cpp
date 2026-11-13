
/*---------------------------------------------------------*\
|   RGBController_ThermaltakeArgentMP1.cpp                  |
|                                                           |
|   OpenRGB UI Wrapper for Thermaltake Argent MP1 Mousepad  |
|                                                           |
|   John Scripter                                4 Nov 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_ThermaltakeArgentMP1.h"

RGBController_ThermaltakeArgentMP1::RGBController_ThermaltakeArgentMP1(ThermaltakeArgentMP1Controller* controller_ptr)
{
    controller  = controller_ptr;

    name        = "Thermaltake Argent MP1";
    type        = DEVICE_TYPE_MOUSEMAT;
    description = "Thermaltake Argent MP1 RGB Mousepad Device Driver";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = 0;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();
}

RGBController_ThermaltakeArgentMP1::~RGBController_ThermaltakeArgentMP1()
{
    Shutdown();
    
    delete controller;
}

void RGBController_ThermaltakeArgentMP1::SetupZones()
{
    zone mousepad_zone;
    mousepad_zone.name       = "Mousepad Rim";
    mousepad_zone.type       = ZONE_TYPE_LINEAR;
    mousepad_zone.leds_min   = 20;
    mousepad_zone.leds_max   = 20;
    mousepad_zone.leds_count = 20;
    zones.push_back(mousepad_zone);

    for(int i = 0; i < 20; i++)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_ThermaltakeArgentMP1::ResizeZone(int /*zone*/, int /*new_size*/)
{
    // Mousepad sizes are statically defined constraints; resizing is unsupported
}

void RGBController_ThermaltakeArgentMP1::DeviceUpdateLEDs()
{
    controller->SetLedColors(colors);
}

void RGBController_ThermaltakeArgentMP1::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ThermaltakeArgentMP1::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ThermaltakeArgentMP1::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

/*---------------------------------------------------------*\
| RGBController_SkydimoHID.cpp                             |
|                                                           |
|   RGBController for Skydimo HID devices                   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_SkydimoHID.h"

RGBController_SkydimoHID::RGBController_SkydimoHID(SkydimoHIDController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetDeviceName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_HEADSET_STAND; // Matches 7x7 grid best
    description         = "Skydimo HID Device";
    serial              = controller->GetSerial();
    location            = controller->GetLocation();

    mode Direct;
    Direct.name         = "Direct";
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_SkydimoHID::~RGBController_SkydimoHID()
{
    delete controller;
}

void RGBController_SkydimoHID::SetupZones()
{
    zone led_zone;
    led_zone.name       = "Matrix";
    led_zone.type       = ZONE_TYPE_MATRIX;
    led_zone.leds_min   = 49;
    led_zone.leds_max   = 49;
    led_zone.leds_count = 49;

    led_zone.matrix_map.height    = 7;
    led_zone.matrix_map.width     = 7;

    // Construct matrix map
    std::vector<unsigned int> map(7 * 7, -1);
    const int vKeyPositions[49][2] = {
        {0, 6}, {0, 5}, {0, 4}, {0, 3}, {0, 2}, {0, 1}, {0, 0},
        {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6},
        {2, 6}, {2, 5}, {2, 4}, {2, 3}, {2, 2}, {2, 1}, {2, 0},
        {3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6},
        {4, 6}, {4, 5}, {4, 4}, {4, 3}, {4, 2}, {4, 1}, {4, 0},
        {5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6},
        {6, 6}, {6, 5}, {6, 4}, {6, 3}, {6, 2}, {6, 1}, {6, 0}
    };
    for(unsigned int i = 0; i < 49; i++)
    {
        int col = vKeyPositions[i][0];
        int row = vKeyPositions[i][1];
        map[row * 7 + col] = i;
    }
    led_zone.matrix_map.map = map;

    zones.push_back(led_zone);

    for(unsigned int led_idx = 0; led_idx < 49; led_idx++)
    {
        led new_led;
        new_led.name = "LED ";
        new_led.name.append(std::to_string(led_idx));

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_SkydimoHID::DeviceUpdateLEDs()
{
    controller->SetLEDs(colors);
}

void RGBController_SkydimoHID::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoHID::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoHID::DeviceUpdateMode()
{

}

/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.cpp                          |
|                                                           |
|   RGBController for Skydimo serial devices               |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include "RGBController_SkydimoSerial.h"

/**------------------------------------------------------------------*\
    @name Skydimo Serial
    @category LEDStrip
    @type Serial
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectSkydimoSerialControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_SkydimoSerial::RGBController_SkydimoSerial(SkydimoSerialController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetName();
    vendor              = "Skydimo";
    type                = DEVICE_TYPE_LEDSTRIP;
    description         = "Skydimo Serial Device";
    serial              = controller->GetSerial();
    location            = controller->GetLocation();

    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_SkydimoSerial::~RGBController_SkydimoSerial()
{
    controller->SetBlack();
    ClearZoneData();
    delete controller;
}

void RGBController_SkydimoSerial::ClearZoneData()
{
    for(zone& led_zone: zones)
    {
        if(led_zone.matrix_map != nullptr)
        {
            delete[] led_zone.matrix_map->map;
            delete led_zone.matrix_map;
            led_zone.matrix_map = nullptr;
        }
    }

    zones.clear();
    leds.clear();
}

void RGBController_SkydimoSerial::SetupZones()
{
    ClearZoneData();

    zone led_zone;
    led_zone.name       = "Output 1";
    led_zone.type       = controller->GetMatrixMap().empty() ? ZONE_TYPE_LINEAR : ZONE_TYPE_MATRIX;
    led_zone.leds_min   = controller->GetMinLEDCount();
    led_zone.leds_max   = controller->GetMaxLEDCount();
    led_zone.leds_count = controller->GetLEDCount();
    led_zone.matrix_map = nullptr;

    if(led_zone.type == ZONE_TYPE_MATRIX)
    {
        led_zone.matrix_map            = new matrix_map_type;
        led_zone.matrix_map->height    = controller->GetMatrixHeight();
        led_zone.matrix_map->width     = controller->GetMatrixWidth();

        unsigned int map_size          = led_zone.matrix_map->height * led_zone.matrix_map->width;
        led_zone.matrix_map->map       = new unsigned int[map_size];

        const std::vector<unsigned int>& source_map = controller->GetMatrixMap();
        std::copy(source_map.begin(), source_map.end(), led_zone.matrix_map->map);
    }

    zones.push_back(led_zone);

    for(unsigned int led_idx = 0; led_idx < led_zone.leds_count; led_idx++)
    {
        led new_led;
        new_led.name = "LED ";
        new_led.name.append(std::to_string(led_idx));

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_SkydimoSerial::ResizeZone(int zone_idx, int new_size)
{
    if(!controller->IsResizable())
    {
        return;
    }

    if((zone_idx < 0) || (static_cast<unsigned int>(zone_idx) >= zones.size()))
    {
        return;
    }

    if(new_size < 0)
    {
        return;
    }

    const unsigned int new_size_u = static_cast<unsigned int>(new_size);
    zone& led_zone = zones[zone_idx];

    if((new_size_u < led_zone.leds_min) || (new_size_u > led_zone.leds_max))
    {
        return;
    }

    if(led_zone.leds_count == new_size_u)
    {
        return;
    }

    if(leds.size() > new_size_u)
    {
        leds.resize(new_size_u);
    }

    while(leds.size() < new_size_u)
    {
        led new_led;
        new_led.name = "LED ";
        new_led.name.append(std::to_string(leds.size()));

        leds.push_back(new_led);
    }

    led_zone.leds_count = new_size_u;
    controller->SetLEDCount(new_size_u);

    SetupColors();
}

void RGBController_SkydimoSerial::DeviceUpdateLEDs()
{
    controller->SetLEDs(colors);
}

void RGBController_SkydimoSerial::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoSerial::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoSerial::DeviceUpdateMode()
{

}

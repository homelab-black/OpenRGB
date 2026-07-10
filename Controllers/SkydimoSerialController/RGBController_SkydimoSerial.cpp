/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.cpp                           |
|                                                           |
|   RGBController for Skydimo serial devices                |
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
    zones.clear();
    leds.clear();
}

void RGBController_SkydimoSerial::SetupZones()
{
    if (zones.empty())
    {
        zone led_zone;
        led_zone.name       = "Output 1";
        led_zone.type       = controller->GetMatrixMap().empty() ? ZONE_TYPE_LINEAR : ZONE_TYPE_MATRIX;
        led_zone.leds_min   = controller->GetMinLEDCount();
        led_zone.leds_max   = controller->GetMaxLEDCount();
        led_zone.leds_count = controller->GetLEDCount();

        if (controller->IsResizable())
        {
            led_zone.flags  = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE
                            | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME
                            | ZONE_FLAG_MANUALLY_CONFIGURABLE_TYPE
                            | ZONE_FLAG_MANUALLY_CONFIGURABLE_MATRIX_MAP
                            | ZONE_FLAG_MANUALLY_CONFIGURABLE_SEGMENTS;
        }

        if (led_zone.type == ZONE_TYPE_MATRIX)
        {
            led_zone.matrix_map.height    = controller->GetMatrixHeight();
            led_zone.matrix_map.width     = controller->GetMatrixWidth();
            led_zone.matrix_map.map       = controller->GetMatrixMap();
        }

        zones.push_back(led_zone);
    }

    leds.clear();
    for (unsigned int led_idx = 0; led_idx < zones[0].leds_count; led_idx++)
    {
        led new_led;
        new_led.name = "LED ";
        new_led.name.append(std::to_string(led_idx));

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_SkydimoSerial::DeviceConfigureZone(int zone_idx)
{
    if (static_cast<unsigned int>(zone_idx) < zones.size())
    {
        controller->SetLEDCount(zones[zone_idx].leds_count);
        SetupZones();
    }
}

void RGBController_SkydimoSerial::DeviceUpdateLEDs()
{
    controller->SetLEDs(colors);
}

void RGBController_SkydimoSerial::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoSerial::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SkydimoSerial::DeviceUpdateMode()
{

}

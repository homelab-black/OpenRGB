/*---------------------------------------------------------*\
| RGBController_RazerNari.cpp                               |
|                                                           |
|   RGBController for Razer Nari devices                    |
|                                                           |
|   Greg Sandstrom (superstrom)                 22 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_RazerNari.h"
#include "RazerDevices.h"

RGBController_RazerNari::RGBController_RazerNari(RazerNariController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetName();
    vendor                  = "Razer";
    type                    = controller->GetDeviceType();
    description             = "Razer Device";
    location                = controller->GetDeviceLocation();
    version                 = controller->GetFirmwareString();
    serial                  = controller->GetSerialString();
    uint8_t max_brightness  = controller->GetMaxBrightness();

    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = RAZER_NARI_MODE_DIRECT;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    Direct.brightness_min   = 0;
    Direct.brightness_max   = max_brightness;
    Direct.brightness       = max_brightness;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_RazerNari::~RGBController_RazerNari()
{
    delete controller;
}

void RGBController_RazerNari::SetupZones()
{
    unsigned int device_index = controller->GetDeviceIndex();

    /*-----------------------------------------------------*\
    | Fill in zone information based on device table        |
    | Kraken V4 devices are assumed to only have one row    |
    \*-----------------------------------------------------*/
    for(unsigned int zone_id = 0; zone_id < RAZER_MAX_ZONES; zone_id++)
    {
        if(device_list[device_index]->zones[zone_id] != NULL)
        {
            zone new_zone;

            new_zone.name       = device_list[device_index]->zones[zone_id]->name;
            new_zone.type       = device_list[device_index]->zones[zone_id]->type;

            new_zone.leds_count = device_list[device_index]->zones[zone_id]->cols;
            new_zone.leds_min   = new_zone.leds_count;
            new_zone.leds_max   = new_zone.leds_count;

            zones.push_back(new_zone);

            for(unsigned int col_id = 0; col_id < device_list[device_index]->zones[zone_id]->cols; col_id++)
            {
                led* new_led = new led();

                new_led->name = device_list[device_index]->zones[zone_id]->name;

                if(zones[zone_id].leds_count > 1)
                {
                    new_led->name.append(" LED ");
                    new_led->name.append(std::to_string(col_id + 1));
                }

                leds.push_back(*new_led);
            }
        }
    }

    SetupColors();
}

void RGBController_RazerNari::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == RAZER_NARI_MODE_DIRECT)
    {
        controller->SetDirect(&colors[0]);
    }
}

void RGBController_RazerNari::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RazerNari::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RazerNari::DeviceUpdateMode()
{
    controller->SetBrightness(modes[active_mode].brightness);
}

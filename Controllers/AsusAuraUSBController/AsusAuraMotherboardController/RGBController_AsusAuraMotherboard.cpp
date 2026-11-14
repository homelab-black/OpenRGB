/*---------------------------------------------------------*\
| RGBController_AsusAuraMotherboard.cpp                         |
|                                                           |
|   RGBController for ASUS Aura motherboard                     |
|                                                           |
|   Mola19                                      08 Mar 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AsusAuraMotherboard.h"

/**------------------------------------------------------------------*\
    @name Asus Aura Motherboard
    @category Accessory
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectAsusAuraUSBMotherboard
    @comment
\*-------------------------------------------------------------------*/

RGBController_AuraMotherboard::RGBController_AuraMotherboard(AuraMotherboardController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetNameString();
    vendor              = "ASUS";
    type                = DEVICE_TYPE_MOTHERBOARD;
    description         = "ASUS Aura Motherboard Device";
    location            = controller->GetDeviceLocation();
    serial              = controller->GetSerialString();

    controller->SetGen1();

    SetupZones();
    SetupModes();
}

RGBController_AuraMotherboard::~RGBController_AuraMotherboard()
{
    Shutdown();

    delete controller;
}

void RGBController_AuraMotherboard::SetupModes()
{
    mode Direct;
    Direct.name                     = "Direct" ;
    Direct.value                    = 0;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Direct);
}

void RGBController_AuraMotherboard::SetupZones()
{
    /*-----------------------------------------------------*\
    | Only set LED count on the first run                   |
    \*-----------------------------------------------------*/
    bool first_run = false;

    if(zones.size() == 0)
    {
        first_run = true;
    }

    /*-----------------------------------------------------*\
    | Clear any existing color/LED configuration            |
    \*-----------------------------------------------------*/
    leds.clear();
    colors.clear();
    zones.resize(4);

    /*-----------------------------------------------------*\
    | Set up addressable zones                              |
    \*-----------------------------------------------------*/
    // 240 leds
    // 3 headers
    // 80 per header
    for(unsigned int i = 0; i < AURA_MOTHERBOARD_HEADERS_COUNT; i++)
    {
        zones[i].leds_min                 = 0;
        zones[i].leds_max                 = AURA_MOTHERBOARD_MAX_LED_PER_HEADERS;

        if(first_run)
        {
            zones[i].flags             = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE
                                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME
                                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_TYPE
                                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_MATRIX_MAP
                                       | ZONE_FLAG_MANUALLY_CONFIGURABLE_SEGMENTS;
        }

        if(!(zones[i].flags & ZONE_FLAG_MANUALLY_CONFIGURED_NAME))
        {
            zones[i].name = "ADD GEN2_" + std::to_string(i + 1);
        }

        if(!(zones[i].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
        {
            zones[i].leds_count           = 0;
        }

        if(!(zones[i].flags & ZONE_FLAG_MANUALLY_CONFIGURED_TYPE))
        {
            zones[i].type                 = ZONE_TYPE_LINEAR;
        }

        if(!(zones[i].flags & ZONE_FLAG_MANUALLY_CONFIGURED_MATRIX_MAP))
        {
            zones[i].matrix_map.width     = 0;
            zones[i].matrix_map.height    = 0;
            zones[i].matrix_map.map.resize(0);
        }

        for(unsigned int l = 0; l < zones[i].leds_count; l++)
        {
            led new_led;
            new_led.name            = zones[i].name;
            new_led.name.append(", LED ");
            new_led.name.append(std::to_string(l + 1));
            new_led.value           = i;

            leds.push_back(new_led);
        }
    }

    /*-----------------------------------------------------*\
    | Set up Logo zone                                      |
    \*-----------------------------------------------------*/
    zones[3].type                   = ZONE_TYPE_SINGLE;
    zones[3].name                   = "Logo";
    zones[3].leds_min               = AURA_MOTHERBOARD_LOGO_LEDS_COUNT;
    zones[3].leds_max               = AURA_MOTHERBOARD_LOGO_LEDS_COUNT;
    zones[3].leds_count             = AURA_MOTHERBOARD_LOGO_LEDS_COUNT;

    for(unsigned int l = 0; l < zones[3].leds_count; l++)
    {
        led new_led;
        new_led.name            = zones[3].name;
        new_led.name.append(", LED ");
        new_led.name.append(std::to_string(l + 1));
        new_led.value           = 3;

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_AuraMotherboard::DeviceConfigureZone(int zone_idx)
{
    if((size_t)zone_idx < zones.size())
    {
        SetupZones();
    }
}

void RGBController_AuraMotherboard::DeviceUpdateLEDs()
{
    for(unsigned int i = 0; i < zones.size(); i++)
    {
        controller->SendDirect(i, colors, zones[i].start_idx, zones[i].leds_count);
    }
}

void RGBController_AuraMotherboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AuraMotherboard::DeviceUpdateSingleLED(int led)
{

}

void RGBController_AuraMotherboard::DeviceUpdateMode()
{
    /*---------------------------------------------------------*\
    | This device does not support Mode changing                |
    \*---------------------------------------------------------*/
}

void RGBController_AuraMotherboard::DeviceSaveMode()
{
    /*---------------------------------------------------------*\
    | This device does not support Mode saving                  |
    \*---------------------------------------------------------*/
}

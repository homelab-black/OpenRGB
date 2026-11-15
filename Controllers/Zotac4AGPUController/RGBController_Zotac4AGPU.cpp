/*---------------------------------------------------------*\
| RGBController_Zotac4AGPU.cpp                              |
|                                                           |
|   RGBController for Zotac 4A GPU                          |
|                                                           |
|   Peter Berendi                               29 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_Zotac4AGPU.h"
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name ZOTAC RTX 5080 SOLID CORE
    @category GPU
    @type I2C
    @save :robot:
    @direct :x:
    @effects :tools:
    @detectors DetectZotac4AGPUControllers
    @comment
        Based on Peter Berendi's Zotac 4A controller in merge request !2625.
        Tested on PCI 10DE:2C02 / 19DA:1765, firmware N765E-1001.

        The controller uses I2C address 0x4A, register 0x5A, and 32-byte
        control and status packets. The first ten status bytes contain the
        firmware identifier. Status bytes 10-17 contain on/off, mode, RGB,
        on time, off time, and the color preset index.

        Windows uses the standard NVAPI I2C entry points at 10 kHz through
        DDC port 1, matching FireStorm 5.0.0.010E captures on this card.
        Only the verified Static and Breathing effects are exposed.
        Hardware writes save automatically, so Direct mode is omitted.
        Linux support has not been verified and is not registered.
\*-------------------------------------------------------------------*/

RGBController_Zotac4AGPU::RGBController_Zotac4AGPU(Zotac4AGPUController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "ZOTAC RTX 5080 SOLID CORE";
    vendor      = "ZOTAC";
    description = "ZOTAC 4A RGB GPU Device";
    version     = controller->GetVersion();
    location    = controller->GetDeviceLocation();
    type        = DEVICE_TYPE_GPU;

    mode STATIC;
    STATIC.name       = "Static";
    STATIC.value      = ZOTAC_4A_GPU_MODE_STATIC;
    STATIC.flags      = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    STATIC.color_mode = MODE_COLORS_MODE_SPECIFIC;
    STATIC.colors_min = 1;
    STATIC.colors_max = 1;
    STATIC.colors.resize(1);
    STATIC.colors[0] = ToRGBColor(0, 0, 255);
    modes.push_back(STATIC);

    mode BREATHE;
    BREATHE.name       = "Breathing";
    BREATHE.value      = ZOTAC_4A_GPU_MODE_BREATHE;
    BREATHE.flags      = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED;
    BREATHE.speed_min  = 0;
    BREATHE.speed_max  = ZOTAC_4A_RGB_MAXSPEED;
    BREATHE.speed      = ZOTAC_4A_RGB_MAXSPEED - 20;
    BREATHE.color_mode = MODE_COLORS_MODE_SPECIFIC;
    BREATHE.colors_min = 1;
    BREATHE.colors_max = 1;
    BREATHE.colors.resize(1);
    BREATHE.colors[0]  = ToRGBColor(0, 0, 255);
    modes.push_back(BREATHE);

    SetupZones();
}

RGBController_Zotac4AGPU::~RGBController_Zotac4AGPU()
{
    Shutdown();
    delete controller;
}

void RGBController_Zotac4AGPU::SetupZones()
{
    led new_led;
    new_led.name           = "GPU LED";
    leds.push_back(new_led);

    zone new_zone;
    new_zone.name          = "GPU Zone";
    new_zone.type          = ZONE_TYPE_SINGLE;
    new_zone.leds_min      = 1;
    new_zone.leds_max      = 1;
    new_zone.leds_count    = 1;
    zones.push_back(new_zone);

    SetupColors();
    SetupInitialValues();
}

void RGBController_Zotac4AGPU::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_Zotac4AGPU::DeviceUpdateMode()
{
    Zotac4AGPUZone zone_config;
    zone_config.mode = modes[active_mode].value;
    zone_config.color = modes[active_mode].colors.size() >= 1 ? modes[active_mode].colors[0] : ToRGBColor(0, 0, 0);
    zone_config.speed = modes[active_mode].speed;
    zone_config.on_time = modes[active_mode].brightness;
    if(!controller->SetMode(zone_config))
    {
        LOG_ERROR("[Zotac4A] Color update failed");
    }

}

void RGBController_Zotac4AGPU::SetupInitialValues()
{
    /*---------------------------------------------------------*\
    | Retrieve current values by reading the device             |
    \*---------------------------------------------------------*/

    bool on;
    Zotac4AGPUZone zone_config;

    if(!controller->GetMode(zone_config, on))
    {
        return;
    }

    for(unsigned int i = 0; i < modes.size(); ++i)
    {
        if(zone_config.mode != modes[i].value)
        {
            continue;
        }
        active_mode = i;
    }

    colors[0] = zone_config.color;
    if(modes[active_mode].colors.size() >= 1)
    {
        modes[active_mode].colors[0] = zone_config.color;
    }

    modes[active_mode].speed = zone_config.speed;
    modes[active_mode].brightness = zone_config.on_time;

}

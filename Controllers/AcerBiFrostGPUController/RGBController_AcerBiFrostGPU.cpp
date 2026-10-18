/*---------------------------------------------------------*\
| RGBController_AcerBiFrostGPU.cpp                          |
|                                                           |
|   RGBController for Acer Predator BiFrost GPU             |
|                                                           |
|   Baobaosheng                                 18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AcerBiFrostGPU.h"

/**------------------------------------------------------------------*\
    @name Acer Predator BiFrost GPU
    @category GPU
    @type I2C
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectAcerBiFrostGPUControllers
    @comment The logo is driven by an ITE lighting MCU on the GPU's
        OEM I2C bus. OpenRGB takes it out of its autonomous mode and
        sets the colors directly. The MCU
        keeps them across a warm reboot but loses them when power is
        removed, so load a profile at startup to restore them.
\*-------------------------------------------------------------------*/

RGBController_AcerBiFrostGPU::RGBController_AcerBiFrostGPU(AcerBiFrostGPUController* controller_ptr)
{
    controller                          = controller_ptr;

    name                                = controller->GetDeviceName();
    vendor                              = "Acer";
    description                         = "Acer Predator BiFrost GPU Device";
    location                            = controller->GetDeviceLocation();
    type                                = DEVICE_TYPE_GPU;

    mode Direct;
    Direct.name                         = "Direct";
    Direct.value                        = ACER_BIFROST_MODE_DIRECT;
    Direct.flags                        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode                   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Off;
    Off.name                            = "Off";
    Off.value                           = ACER_BIFROST_MODE_OFF;
    Off.flags                           = 0;
    Off.color_mode                      = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_AcerBiFrostGPU::~RGBController_AcerBiFrostGPU()
{
    Shutdown();

    delete controller;
}

void RGBController_AcerBiFrostGPU::SetupZones()
{
    zone logo;
    logo.name                       = "Logo";
    logo.type                       = ZONE_TYPE_LINEAR;
    logo.leds_min                   = ACER_BIFROST_NUM_LEDS;
    logo.leds_max                   = ACER_BIFROST_NUM_LEDS;
    logo.leds_count                 = ACER_BIFROST_NUM_LEDS;
    zones.push_back(logo);

    /*---------------------------------------------------------*\
    | LEDs are listed left to right. The MCU numbers its lamps  |
    | right to left, see DeviceUpdateLEDs                       |
    \*---------------------------------------------------------*/
    for(unsigned int i = 0; i < logo.leds_count; i++)
    {
        led new_led;
        new_led.name = logo.name + " " + std::to_string(i + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_AcerBiFrostGPU::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == ACER_BIFROST_MODE_OFF)
    {
        controller->SetLedColorAll(ToRGBColor(0, 0, 0));
        return;
    }

    /*---------------------------------------------------------*\
    | Check if all colors are identical. If they are do a       |
    | single range write instead of writing each lamp           |
    \*---------------------------------------------------------*/
    bool all_same = true;
    for(std::size_t i = 1; i < colors.size(); i++)
    {
        if(colors[i-1] != colors[i])
        {
            all_same = false;
            break;
        }
    }

    if(all_same)
    {
        controller->SetLedColorAll(colors[0]);
    }
    else
    {
        RGBColor lamp_colors[ACER_BIFROST_NUM_LEDS];

        for(unsigned int lamp = 0; lamp < ACER_BIFROST_NUM_LEDS; lamp++)
        {
            lamp_colors[lamp] = colors[ACER_BIFROST_NUM_LEDS - 1 - lamp];
        }

        controller->SetLedColors(lamp_colors, ACER_BIFROST_NUM_LEDS);
    }
}

void RGBController_AcerBiFrostGPU::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AcerBiFrostGPU::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AcerBiFrostGPU::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

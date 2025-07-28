/*---------------------------------------------------------*\
| RGBController_GalaxGPUAmpere.cpp                          |
|                                                           |
|   RGBController for Galax/KFA2 GPU                        |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_GalaxGPUAmpere.h"

/**------------------------------------------------------------------*\
    @name Galax GPU Ampere
    @category GPU
    @type I2C
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectGalaxGPUControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_GalaxGPUAmpere::RGBController_GalaxGPUAmpere(GalaxGPUAmpereController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "GALAX";
    type        = DEVICE_TYPE_GPU;
    description = "GALAX / KFA2 RTX Ampere GPU";
    location    = controller->GetDeviceLocation();

    mode Static;
    Static.name       = "Static";
    Static.value      = GALAX_AMPERE_MODE_STATIC;
    Static.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE;
    Static.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name       = "Breathing";
    Breathing.value      = GALAX_AMPERE_MODE_BREATHING;
    Breathing.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE;
    Breathing.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Breathing);

    mode BreathingCycle;
    BreathingCycle.name       = "Breathing Cycle";
    BreathingCycle.value      = GALAX_AMPERE_MODE_BREATHING_CYCLE;
    BreathingCycle.flags      = MODE_FLAG_MANUAL_SAVE;
    BreathingCycle.color_mode = MODE_COLORS_NONE;
    modes.push_back(BreathingCycle);

    mode SpectrumCycle;
    SpectrumCycle.name       = "Spectrum Cycle";
    SpectrumCycle.value      = GALAX_AMPERE_MODE_SPECTRUM_CYCLE;
    SpectrumCycle.flags      = MODE_FLAG_MANUAL_SAVE;
    SpectrumCycle.color_mode = MODE_COLORS_NONE;
    modes.push_back(SpectrumCycle);

    mode Off;
    Off.name       = "Off";
    Off.value      = GALAX_AMPERE_MODE_OFF;
    Off.flags      = MODE_FLAG_MANUAL_SAVE;
    Off.color_mode = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();

    active_mode = controller->GetMode();
}

RGBController_GalaxGPUAmpere::~RGBController_GalaxGPUAmpere()
{
    delete controller;
}

void RGBController_GalaxGPUAmpere::SetupZones()
{
    /*---------------------------------------------------------*\
    | Set up zone                                               |
    \*---------------------------------------------------------*/
    zone galax_gpu_zone;
    galax_gpu_zone.name         = "GPU";
    galax_gpu_zone.type         = ZONE_TYPE_SINGLE;
    galax_gpu_zone.leds_min     = 1;
    galax_gpu_zone.leds_max     = 1;
    galax_gpu_zone.leds_count   = 1;
    galax_gpu_zone.matrix_map   = NULL;
    zones.push_back(galax_gpu_zone);

    /*---------------------------------------------------------*\
    | Set up LED                                                |
    \*---------------------------------------------------------*/
    led galax_gpu_led;
    galax_gpu_led.name          = "GPU";
    leds.push_back(galax_gpu_led);

    SetupColors();

    /*---------------------------------------------------------*\
    | Initialize color                                          |
    \*---------------------------------------------------------*/
    unsigned char r = controller->GetLEDRed();
    unsigned char g = controller->GetLEDGreen();
    unsigned char b = controller->GetLEDBlue();
    colors[0] = ToRGBColor(r, g, b);
}

void RGBController_GalaxGPUAmpere::ResizeZone(int /* zone */, int /* new_size */)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_GalaxGPUAmpere::DeviceUpdateLEDs()
{
    if(active_mode == GALAX_AMPERE_MODE_OFF)
    {
        controller->SetLEDColors(0, 0, 0);
        return;
    }

    for(std::size_t led = 0; led < colors.size(); led++)
    {
        unsigned char r = RGBGetRValue(colors[led]);
        unsigned char g = RGBGetGValue(colors[led]);
        unsigned char b = RGBGetBValue(colors[led]);
        controller->SetLEDColors(r, g, b);
    }
}

void RGBController_GalaxGPUAmpere::UpdateZoneLEDs(int /* zone */)
{
    DeviceUpdateLEDs();
}

void RGBController_GalaxGPUAmpere::UpdateSingleLED(int /* led */)
{
    DeviceUpdateLEDs();
}

void RGBController_GalaxGPUAmpere::DeviceUpdateMode()
{
    controller->SetMode(modes[active_mode].value);
}

void RGBController_GalaxGPUAmpere::DeviceSaveMode()
{
    DeviceUpdateMode();
    controller->SaveMode();
}

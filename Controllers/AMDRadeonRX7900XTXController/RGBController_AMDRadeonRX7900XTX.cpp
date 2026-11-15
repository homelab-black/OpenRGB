/*---------------------------------------------------------*\
| RGBController_AMDRadeonRX7900XTX.cpp                      |
|                                                           |
|   RGBController for AMD Radeon RX 7900 XTX reference RGB  |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "RGBController_AMDRadeonRX7900XTX.h"

/**------------------------------------------------------------------*\
    @name AMD Radeon RX 7900 XTX Reference
    @category GPU
    @type I2C
    @save :o:
    @direct :o:
    @effects :tools:
    @detectors DetectAMDRadeonRX7900XTXControllers
    @comment Linux SMU I2C support. One logical light bar.
        Requires successful identification before writes.
\*-------------------------------------------------------------------*/

RGBController_AMDRadeonRX7900XTX::RGBController_AMDRadeonRX7900XTX(AMDRadeonRX7900XTXController* controller_ptr)
{
    controller  = controller_ptr;
    name        = "AMD Radeon RX 7900 XTX Reference";
    vendor      = "AMD";
    description = "Reference card RGB light bar (Cooler Master SMU I2C)";
    location    = controller->GetLocation();
    type        = DEVICE_TYPE_GPU;

    mode Static;
    Static.name                 = "Static";
    Static.value                = AMD_RX7900XTX_MODE_STATIC;
    Static.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_PER_LED;
    Static.brightness_min       = 0;
    Static.brightness_max       = 255;
    Static.brightness           = 255;
    modes.push_back(Static);

    mode Breathing              = Static;
    Breathing.name              = "Breathing";
    Breathing.value             = AMD_RX7900XTX_MODE_BREATHING;
    modes.push_back(Breathing);

    mode Cycle                  = Static;
    Cycle.name                  = "Spectrum Cycle";
    Cycle.value                 = AMD_RX7900XTX_MODE_CYCLE;
    Cycle.flags                 = MODE_FLAG_HAS_BRIGHTNESS;
    Cycle.color_mode            = MODE_COLORS_NONE;
    Cycle.brightness_max        = 127;
    Cycle.brightness            = 127;
    modes.push_back(Cycle);

    mode Off;
    Off.name                    = "Off";
    Off.value                   = AMD_RX7900XTX_MODE_OFF;
    Off.flags                   = 0;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_AMDRadeonRX7900XTX::~RGBController_AMDRadeonRX7900XTX()
{
    Shutdown();
    delete controller;
}

void RGBController_AMDRadeonRX7900XTX::SetupZones()
{
    zone bar;
    bar.name        = "Light Bar";
    bar.type        = ZONE_TYPE_SINGLE;
    bar.leds_min    = 1;
    bar.leds_max    = 1;
    bar.leds_count  = 1;
    zones.push_back(bar);

    led bar_led;
    bar_led.name    = "Light Bar";
    leds.push_back(bar_led);

    SetupColors();
}

void RGBController_AMDRadeonRX7900XTX::DeviceUpdateLEDs()
{
    RGBColor color = colors[0];

    if(modes[active_mode].value == AMD_RX7900XTX_MODE_CYCLE)
    {
        color = ToRGBColor(255, 255, 255);
    }

    if(!controller->SetMode(modes[active_mode].value, modes[active_mode].brightness, color))
    {
        LOG_ERROR("[AMD RX 7900 XTX] RGB update failed on %s; rescan required before further writes", location.c_str());
    }
}

void RGBController_AMDRadeonRX7900XTX::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AMDRadeonRX7900XTX::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AMDRadeonRX7900XTX::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

/*---------------------------------------------------------*\
| RGBController_SamsungSSD_Windows_Linux.cpp                |
|                                                           |
|   RGBController for Samsung SSD heatsink LED              |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_SamsungSSD_Windows_Linux.h"

/**------------------------------------------------------------------*\
    @name Samsung SSD
    @category Storage
    @type NVMe
    @save :x:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectSamsungSSDControllers
    @comment Samsung SSD 990 PRO with Heatsink. The drive has one LED.
        It is not known if the drive keeps the LED state after a
        power cycle. Needs administrator or root rights.
\*-------------------------------------------------------------------*/

enum
{
    SAMSUNG_SSD_OPENRGB_MODE_OFF    = 0x00,
};

RGBController_SamsungSSD::RGBController_SamsungSSD(SamsungSSDController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetName();
    vendor                  = "Samsung";
    type                    = DEVICE_TYPE_STORAGE;
    description             = "Samsung SSD Heatsink LED";
    version                 = controller->GetFirmwareVersion();
    serial                  = controller->GetSerial();
    location                = controller->GetLocation();

    mode Static;
    Static.name             = "Static";
    Static.value            = SAMSUNG_SSD_LED_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Static.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Flashing;
    Flashing.name           = "Flashing";
    Flashing.value          = SAMSUNG_SSD_LED_MODE_BLINKING;
    Flashing.flags          = MODE_FLAG_HAS_PER_LED_COLOR;
    Flashing.color_mode     = MODE_COLORS_PER_LED;
    modes.push_back(Flashing);

    mode Breathing;
    Breathing.name          = "Breathing";
    Breathing.value         = SAMSUNG_SSD_LED_MODE_BREATHING;
    Breathing.flags         = MODE_FLAG_HAS_PER_LED_COLOR;
    Breathing.color_mode    = MODE_COLORS_PER_LED;
    modes.push_back(Breathing);

    mode Off;
    Off.name                = "Off";
    Off.value               = SAMSUNG_SSD_OPENRGB_MODE_OFF;
    Off.flags               = 0;
    Off.color_mode          = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_SamsungSSD::~RGBController_SamsungSSD()
{
    Shutdown();

    delete controller;
}

void RGBController_SamsungSSD::SetupZones()
{
    zone led_zone;
    led_zone.name           = "Heatsink";
    led_zone.type           = ZONE_TYPE_SINGLE;
    led_zone.leds_min       = 1;
    led_zone.leds_max       = 1;
    led_zone.leds_count     = 1;
    zones.push_back(led_zone);

    led heatsink_led;
    heatsink_led.name       = "Heatsink LED";
    leds.push_back(heatsink_led);

    SetupColors();
}

void RGBController_SamsungSSD::DeviceUpdateLEDs()
{
    RGBColor        color   = colors[0];
    unsigned int    value   = modes[active_mode].value;

    /*-----------------------------------------------------*\
    | Magician keeps the mode and the color when it turns   |
    | the LED off, so Off sends Static with the LED color   |
    \*-----------------------------------------------------*/
    if(value == SAMSUNG_SSD_OPENRGB_MODE_OFF)
    {
        controller->SetLED(false, SAMSUNG_SSD_LED_MODE_STATIC, RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color));
    }
    else
    {
        controller->SetLED(true, (unsigned char)value, RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color));
    }
}

void RGBController_SamsungSSD::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SamsungSSD::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SamsungSSD::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

/*---------------------------------------------------------*\
| RGBController_SayoDevice.cpp                              |
|                                                           |
|   Controller for Sayo Devices                             |
|                                                           |
|   Richard Harris                              24 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_SayoDevice.h"
#include "SayoDeviceController.h"
#include "LogManager.h"

/**--------------------------------------------------------------------*\
    @name SayoDevice E1
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectSayoDeviceController
    @comment
\*---------------------------------------------------------------------*/

RGBController_SayoDevice::RGBController_SayoDevice(SayoDeviceController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetDeviceName();
    vendor                          = "SayoDevice";
    description                     = "SayoDevice E1 Knob";
    type                            = DEVICE_TYPE_KEYBOARD;
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();
    version                         = controller->GetFirmwareVersion();

    mode Static;
    Static.name                     = "Static";
    Static.value                    = SAYO_MODE_STATIC;
    Static.flags                    = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    Static.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min               = 1;
    Static.colors_max               = 1;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = SAYO_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    Direct.colors_min               = 1;
    Direct.colors_max               = 1;
    Direct.colors.resize(1);
    modes.push_back(Direct);

    /*-----------------------------------------------------*\
    | Add dynamic modes                                     |
    \*-----------------------------------------------------*/
    struct DynamicMode
    {
        const char* name;
        int         value;
    };

    DynamicMode dynamic_modes[] =
    {
        { "Breathing",              SAYO_MODE_BREATHING },
        { "Wave",                   SAYO_MODE_WAVE      },
        { "Switch",                 SAYO_MODE_SWITCH    },
        { "Blink",                  SAYO_MODE_BLINK     },
    };

    for(unsigned int i = 0; i < (sizeof(dynamic_modes) / sizeof(DynamicMode)); i++)
    {
        mode new_mode;
        new_mode.name           = dynamic_modes[i].name;
        new_mode.value          = dynamic_modes[i].value;
        new_mode.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
        new_mode.color_mode     = MODE_COLORS_MODE_SPECIFIC;
        new_mode.colors_min     = 1;
        new_mode.colors_max     = 1;
        new_mode.colors.resize(1);
        new_mode.speed_min      = 0;
        new_mode.speed_max      = 3;
        new_mode.speed          = 1;
        modes.push_back(new_mode);
    }

    SetupZones();
};

RGBController_SayoDevice::~RGBController_SayoDevice()
{
    delete controller;
}

void RGBController_SayoDevice::SetupZones()
{
    int led_count = controller->GetLEDCount();

    zone zone;
    zone.name       = "Underglow";
    zone.type       = ZONE_TYPE_LINEAR;
    zone.leds_count = led_count;
    zone.leds_min   = led_count;
    zone.leds_max   = led_count;
    zone.matrix_map = NULL;

    zones.clear();
    zones.push_back(zone);

    leds.clear();
    leds.resize(led_count);
    for(int i = 0; i < led_count; i++)
    {
        led& new_led = leds[i];
        new_led.name                = "LED " + std::to_string(i + 1);
        new_led.value               = i;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_SayoDevice::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*-----------------------------------------------------*\
    | This device does not support resizing zones           |
    \*-----------------------------------------------------*/
}

void RGBController_SayoDevice::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == SAYO_MODE_DIRECT)
    {
        controller->SetMode(SAYO_MODE_STATIC, SAYO_SPEED_1X, colors[0], false);
    }
}

void RGBController_SayoDevice::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SayoDevice::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SayoDevice::DeviceUpdateMode()
{
    unsigned int mode   = modes[active_mode].value;
    bool random         = modes[active_mode].color_mode == MODE_COLORS_RANDOM;
    RGBColor color      = modes[active_mode].colors[0];

    switch(modes[active_mode].value)
    {
        case SAYO_MODE_STATIC:
            controller->SetMode(mode, SAYO_SPEED_1X, color, random);
            break;
        case SAYO_MODE_DIRECT:
            break;
        case SAYO_MODE_BREATHING:
        case SAYO_MODE_WAVE:
        case SAYO_MODE_SWITCH:
        case SAYO_MODE_BLINK:
        {
            unsigned int speed = 3 - modes[active_mode].speed;
            controller->SetMode(mode, speed, color, random);
            break;
        }
        default:
            LOG_ERROR("[SayoDevice] Requested mode (%02x) is not supported", mode);
    }
}

void RGBController_SayoDevice::DeviceSaveMode()
{
    controller->Save();
}

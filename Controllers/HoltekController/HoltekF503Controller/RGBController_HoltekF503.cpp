/*---------------------------------------------------------*\
| RGBController_HoltekF503.cpp                              |
|                                                           |
|   RGBController for Holtek mouse                          |
|                                                           |
|   Remigiusz Pietroń (remememe)             24 Aug 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_HoltekF503.h"

/**------------------------------------------------------------------*\
    @name Holtek F503
    @category Mouse
    @type USB
    @save :x:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectHoltekControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_HoltekF503::RGBController_HoltekF503(HoltekF503Controller* controller_ptr)
{
    controller                          = controller_ptr;

    name                                = controller->GetNameString();
    vendor                              = "Holtek";
    type                                = DEVICE_TYPE_MOUSE;
    description                         = "Holtek USB Gaming Mouse Device";
    location                            = controller->GetDeviceLocation();
    serial                              = controller->GetSerialString();

    mode Static;
    Static.name                         = "Static";
    Static.value                        = HOLTEK_F503_MODE_STATIC;
    Static.flags                        = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.color_mode                   = MODE_COLORS_PER_LED;
    Static.brightness_min               = 0;
    Static.brightness                   = 3;
    Static.brightness_max               = 3;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name                      = "Breathing";
    Breathing.value                     = HOLTEK_F503_MODE_BREATHING;
    Breathing.flags                     = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.color_mode                = MODE_COLORS_PER_LED;
    Breathing.speed_min                 = HOLTEK_F503_SPEED_SLOW;
    Breathing.speed_max                 = HOLTEK_F503_SPEED_FAST;
    Breathing.speed                     = HOLTEK_F503_SPEED_NORMAL;
    Breathing.brightness_min            = 0;
    Breathing.brightness                = 3;
    Breathing.brightness_max            = 3;
    modes.push_back(Breathing);

    mode Breathing7Colors;
    Breathing7Colors.name               = "Breathing 7 Colors";
    Breathing7Colors.value              = HOLTEK_F503_MODE_BREATHING;
    Breathing7Colors.flags              = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing7Colors.color_mode         = MODE_COLORS_NONE;
    Breathing7Colors.speed_min          = HOLTEK_F503_SPEED_SLOW;
    Breathing7Colors.speed_max          = HOLTEK_F503_SPEED_FAST;
    Breathing7Colors.speed              = HOLTEK_F503_SPEED_NORMAL;
    Breathing7Colors.brightness_min     = 0;
    Breathing7Colors.brightness         = 3;
    Breathing7Colors.brightness_max     = 3;
    Breathing7Colors.colors_min         = 1;
    Breathing7Colors.colors_max         = 7;
    Breathing7Colors.colors.resize(7);
    modes.push_back(Breathing7Colors);

    mode Wave;
    Wave.name                           = "Wave";
    Wave.value                          = HOLTEK_F503_MODE_WAVE;
    Wave.flags                          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    Wave.color_mode                     = MODE_COLORS_NONE;
    Wave.speed_min                      = HOLTEK_F503_SPEED_SLOW;
    Wave.speed_max                      = HOLTEK_F503_SPEED_FAST;
    Wave.speed                          = HOLTEK_F503_SPEED_NORMAL;
    Wave.brightness_min                 = 0;
    Wave.brightness                     = 3;
    Wave.brightness_max                 = 3;
    modes.push_back(Wave);

    SetupZones();
}

RGBController_HoltekF503::~RGBController_HoltekF503()
{
    Shutdown();

    delete controller;
}

void RGBController_HoltekF503::SetupZones()
{
    zone mouse_zone;
    mouse_zone.name         = "Mouse";
    mouse_zone.type         = ZONE_TYPE_SINGLE;
    mouse_zone.leds_min     = 1;
    mouse_zone.leds_max     = 1;
    mouse_zone.leds_count   = 1;
    zones.push_back(mouse_zone);

    led mouse_led;
    mouse_led.name          = "Mouse";
    leds.push_back(mouse_led);

    SetupColors();
}
void RGBController_HoltekF503::DeviceUpdateLEDs()
{
    unsigned char flag          = 0x00;
    unsigned char mode          = modes[active_mode].value;
    unsigned char speed         = modes[active_mode].speed;
    unsigned char brightness    = modes[active_mode].brightness;
    unsigned char red           = RGBGetRValue(colors[0]);
    unsigned char green         = RGBGetGValue(colors[0]);
    unsigned char blue          = RGBGetBValue(colors[0]);

    if (modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_LR)
    {
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            flag = HOLTEK_F503_DIRECTION_LEFT;
        }
        else
        {
            flag = HOLTEK_F503_DIRECTION_RIGHT;
        }
    }
    else
    {
        flag = GetHoltekModeFlag();
    }

    controller->SendCustomColor(red,green,blue,mode,speed,flag);
    controller->SendBrightness(brightness);
    controller->Apply();
}

void RGBController_HoltekF503::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HoltekF503::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HoltekF503::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

unsigned char RGBController_HoltekF503::GetHoltekModeFlag()
{
    switch (active_mode)
    {
        case 0:
            return 0x02;

        case 1:
            return 0x04;

        case 2:
            return 0x01;

        default:
            return 0x00;
    }
}

/*---------------------------------------------------------*\
| RGBController_LogitechG515.cpp                            |
|                                                           |
|   RGBController for Logitech G515 LIGHTSPEED TKL         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_LogitechG515.h"

/**------------------------------------------------------------------*\
    @name Logitech G515 LIGHTSPEED TKL
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectLogitechKeyboardG515
    @comment Single-zone implementation based on captured mode packets.
\*-------------------------------------------------------------------*/

RGBController_LogitechG515::RGBController_LogitechG515(LogitechG515Controller* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "Logitech";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "Logitech G515 LIGHTSPEED TKL Keyboard Device";
    serial                          = controller->GetSerialString();

    mode Off;
    Off.name                        = "Off";
    Off.value                       = LOGITECH_G515_MODE_OFF;
    Off.flags                       = 0;
    Off.color_mode                  = MODE_COLORS_NONE;
    modes.push_back(Off);

    mode Static;
    Static.name                     = "Static";
    Static.value                    = LOGITECH_G515_MODE_STATIC;
    Static.flags                    = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Static.colors_min               = 1;
    Static.colors_max               = 1;
    Static.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.value                 = LOGITECH_G515_MODE_BREATHING;
    Breathing.flags                 = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors.resize(1);
    Breathing.brightness_min        = 1;
    Breathing.brightness_max        = 100;
    Breathing.brightness            = 100;
    Breathing.speed_min             = LOGITECH_G515_SPEED_FASTEST_MS;
    Breathing.speed_max             = LOGITECH_G515_SPEED_SLOWEST_MS;
    Breathing.speed                 = LOGITECH_G515_SPEED_NORMAL_MS;
    modes.push_back(Breathing);

    mode Cycle;
    Cycle.name                      = "Spectrum Cycle";
    Cycle.value                     = LOGITECH_G515_MODE_CYCLE;
    Cycle.flags                     = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Cycle.color_mode                = MODE_COLORS_NONE;
    Cycle.speed_min                 = LOGITECH_G515_SPEED_FASTEST_MS;
    Cycle.speed_max                 = LOGITECH_G515_SPEED_SLOWEST_MS;
    Cycle.speed                     = LOGITECH_G515_SPEED_NORMAL_MS;
    Cycle.brightness_min            = 1;
    Cycle.brightness_max            = 100;
    Cycle.brightness                = 100;
    modes.push_back(Cycle);

    mode Wave;
    Wave.name                       = "Rainbow Wave";
    Wave.value                      = LOGITECH_G515_MODE_WAVE;
    Wave.flags                      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Wave.color_mode                 = MODE_COLORS_NONE;
    Wave.speed_min                  = LOGITECH_G515_SPEED_FASTEST_MS;
    Wave.speed_max                  = LOGITECH_G515_SPEED_SLOWEST_MS;
    Wave.speed                      = LOGITECH_G515_SPEED_NORMAL_MS;
    Wave.brightness_min             = 1;
    Wave.brightness_max             = 100;
    Wave.brightness                 = 100;
    modes.push_back(Wave);

    SetupZones();
}

RGBController_LogitechG515::~RGBController_LogitechG515()
{
    delete controller;
}

void RGBController_LogitechG515::SetupZones()
{
    zone new_zone;
    new_zone.name                   = "Keyboard";
    new_zone.type                   = ZONE_TYPE_SINGLE;
    new_zone.leds_min               = 1;
    new_zone.leds_max               = 1;
    new_zone.leds_count             = 1;
    new_zone.matrix_map             = NULL;

    zones.push_back(new_zone);

    led new_led;
    new_led.name                    = "Keyboard";
    new_led.value                   = 0;
    leds.push_back(new_led);

    SetupColors();
}

void RGBController_LogitechG515::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_LogitechG515::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == LOGITECH_G515_MODE_STATIC)
    {
        controller->SetStaticColor(RGBGetRValue(colors[0]),
                                   RGBGetGValue(colors[0]),
                                   RGBGetBValue(colors[0]));
    }
}

void RGBController_LogitechG515::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechG515::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_LogitechG515::DeviceUpdateMode()
{
    unsigned char red         = 0x00;
    unsigned char grn         = 0x00;
    unsigned char blu         = 0x00;
    unsigned char brightness  = 100;
    unsigned short speed_ms   = LOGITECH_G515_SPEED_NORMAL_MS;

    if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
    {
        speed_ms = modes[active_mode].speed;
    }

    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness = modes[active_mode].brightness;
    }

    if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        red = RGBGetRValue(modes[active_mode].colors[0]);
        grn = RGBGetGValue(modes[active_mode].colors[0]);
        blu = RGBGetBValue(modes[active_mode].colors[0]);
    }

    if(modes[active_mode].value == LOGITECH_G515_MODE_OFF)
    {
        controller->SetOff();
        return;
    }

    if(modes[active_mode].value == LOGITECH_G515_MODE_STATIC)
    {
        controller->SetStaticColor(red, grn, blu);
        return;
    }

    if(modes[active_mode].value == LOGITECH_G515_MODE_CYCLE)
    {
        controller->SetSpectrumCycle(speed_ms, brightness);
        return;
    }

    if(modes[active_mode].value == LOGITECH_G515_MODE_BREATHING)
    {
        controller->SetBreathing(speed_ms, brightness, red, grn, blu);
        return;
    }

    if(modes[active_mode].value == LOGITECH_G515_MODE_WAVE)
    {
        controller->SetRainbowWave(speed_ms, brightness);
        return;
    }
}

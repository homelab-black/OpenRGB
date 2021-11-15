/*-------------------------------------------------------------------*\
| RGBController_CMMM731Controller.cpp                                 |
|                                                                     |
|   Driver for the Cooler Master MM731 Mouse                          |
|                                                                     |
|   Chris M (Dr_No)                              1 Jan 2022           |
|                                                                     |
|   This file is part of the OpenRGB project                          |
|   SPDX-License-Identifier: GPL-2.0-only                             |
\*-------------------------------------------------------------------*/

#include "RGBController_CMMM731Controller.h"

/**------------------------------------------------------------------*\
    @name Coolermaster Master Mouse 731
    @category Mouse
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectCoolerMasterMouse
    @comment
\*-------------------------------------------------------------------*/

RGBController_CMMM731Controller::RGBController_CMMM731Controller(CMMM731Controller *controller_ptr)
{
    controller                  = controller_ptr;
    uint8_t speed               = controller->GetLedSpeed();

    name                        = controller->GetDeviceName();
    vendor                      = "Cooler Master";
    type                        = DEVICE_TYPE_MOUSE;
    description                 = controller->GetDeviceName();
    version                     = "1.0";
    serial                      = controller->GetSerial();
    location                    = controller->GetLocation();

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = CM_MM731_MODE_STATIC; //Direct is Static that does not send a save packet
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Static;
    Static.name                 = "Static";
    Static.value                = CM_MM731_MODE_STATIC;
    Static.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Static.colors_min           = 1;
    Static.colors_max           = 1;
    Static.colors.resize(Static.colors_max);
    Static.brightness_min       = CM_MM731_BRIGHTNESS_MIN;
    Static.brightness_max       = CM_MM731_BRIGHTNESS_MAX;
    Static.brightness           = CM_MM731_BRIGHTNESS_MAX;
    Static.speed_min            = CM_MM731_SPEED_SLOWEST;
    Static.speed_max            = CM_MM731_SPEED_FASTEST;
    Static.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Static.speed                = speed;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = CM_MM731_MODE_BREATHING;
    Breathing.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Breathing.colors_min        = 1;
    Breathing.colors_max        = 1;
    Breathing.colors.resize(Breathing.colors_max);
    Breathing.brightness_min    = CM_MM731_BRIGHTNESS_MIN;
    Breathing.brightness_max    = CM_MM731_BRIGHTNESS_MAX;
    Breathing.brightness        = CM_MM731_BRIGHTNESS_MAX;
    Breathing.speed_min         = CM_MM731_SPEED_SLOWEST;
    Breathing.speed_max         = CM_MM731_SPEED_FASTEST;
    Breathing.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Breathing.speed             = speed;
    modes.push_back(Breathing);


    mode Spectrum;
    Spectrum.name               = "Spectrum";
    Spectrum.value              = CM_MM731_MODE_SPECTRUM;
    Spectrum.flags              = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Spectrum.brightness_min     = CM_MM731_BRIGHTNESS_MIN;
    Spectrum.brightness_max     = CM_MM731_BRIGHTNESS_MAX;
    Spectrum.brightness         = CM_MM731_BRIGHTNESS_MAX;
    Spectrum.speed_min          = CM_MM731_SPEED_SLOWEST;
    Spectrum.speed_max          = CM_MM731_SPEED_FASTEST;
    Spectrum.color_mode         = MODE_COLORS_NONE;
    Spectrum.speed              = speed;
    modes.push_back(Spectrum);

    mode Off;
    Off.name                    = "Turn Off";
    Off.value                   = CM_MM731_MODE_OFF;
    Off.flags                   = MODE_FLAG_MANUAL_SAVE;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    Init_Controller();
    SetupZones();

    /*int temp_mode               = controller->GetMode();
    for(std::size_t mode_idx = 0; mode_idx < modes.size() ; mode_idx++)
    {
        if (temp_mode == modes[mode_idx].value)
        {
            active_mode = mode_idx;
            break;
        }
    }
    if (modes[active_mode].flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        modes[active_mode].colors[0] = ToRGBColor(cmargb->GetLedRed(), cmargb->GetLedGreen(), cmargb->GetLedBlue());
        modes[active_mode].color_mode = (cmargb->GetRandomColours()) ? MODE_COLORS_RANDOM : MODE_COLORS_MODE_SPECIFIC;
    }
    if (modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
    {
        modes[active_mode].speed = cmargb->GetLedSpeed();
    }*/
}

RGBController_CMMM731Controller::~RGBController_CMMM731Controller()
{
    delete controller;
}

void RGBController_CMMM731Controller::Init_Controller()
{
    zone mouse_zone;
    mouse_zone.name         = name;
    mouse_zone.type         = ZONE_TYPE_SINGLE;
    mouse_zone.leds_min     = 1;
    mouse_zone.leds_max     = 1;
    mouse_zone.leds_count   = 1;
    mouse_zone.matrix_map   = NULL;
    zones.push_back(mouse_zone);

    led logo_led;
    logo_led.name           = "Logo LED";
    logo_led.value          = 1;
    leds.push_back(logo_led);
}

void RGBController_CMMM731Controller::SetupZones()
{
    SetupColors();
}

void RGBController_CMMM731Controller::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_CMMM731Controller::DeviceUpdateLEDs()
{
    controller->SetLedsDirect( colors[0] );
}

void RGBController_CMMM731Controller::UpdateZoneLEDs(int /*zone*/)
{
    controller->SetLedsDirect( colors[0] );
}

void RGBController_CMMM731Controller::UpdateSingleLED(int /*led*/)
{
    controller->SetLedsDirect( colors[0] );
}

void RGBController_CMMM731Controller::SetCustomMode()
{
    active_mode             = 0;
}

void RGBController_CMMM731Controller::DeviceUpdateMode()
{
    bool random_colours     = (modes[active_mode].color_mode == MODE_COLORS_RANDOM);
    RGBColor colour         = (modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC) ? modes[active_mode].colors[0] : 0;

    controller->SetMode( modes[active_mode].value, modes[active_mode].speed, modes[active_mode].brightness, colour, random_colours );
}

void RGBController_CMMM731Controller::DeviceSaveMode()
{
    DeviceUpdateMode();
    controller->SendSavePacket();
}

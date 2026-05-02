
/*---------------------------------------------------------*\
|   RGBController_Clevo8910.cpp                             |
|                                                           |   
|   RGBController for Clevo RGB Colour per key              |
|                                                           |
|   EzyUser     9 Jan 2026                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_Clevo8910.h"
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name Clevo 8910
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectClevo8910Controller
    @comment
\*-------------------------------------------------------------------*/


RGBController_Clevo8910::RGBController_Clevo8910(Clevo8910Controller* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetDeviceName();
    vendor                          = "OEM - Clevo";
    description                     = name;
    location                        = controller->GetDeviceLocation();
    flags                           = CONTROLLER_FLAG_LOCAL;
    type                            = DEVICE_TYPE_KEYBOARD;

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = CLEVO8910_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_PER_LED_COLOR |
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    Direct.brightness_min           = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Direct.brightness_max           = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Direct.speed                    = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Direct.brightness               = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    modes.push_back(Direct);


    mode Ripple;
    Ripple.name                     = "Ripple";
    Ripple.value                    = CLEVO8910_MODE_RIPPLE;
    Ripple.flags                    = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS | 
                                        MODE_FLAG_HAS_PER_LED_COLOR | 
                                        MODE_FLAG_HAS_RANDOM_COLOR;
    Ripple.speed_min                = CLEVO8910_DIRECT_SPEED_MIN;
    Ripple.speed_max                = CLEVO8910_DIRECT_SPEED_MAX;
    Ripple.brightness_min           = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Ripple.brightness_max           = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Ripple.speed                    = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Ripple.brightness               = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Ripple.color_mode               = MODE_COLORS_RANDOM;
    Ripple.colors.resize(1);
    modes.push_back(Ripple);

    mode Fade;
    Fade.name                       = "Fade";
    Fade.value                      = CLEVO8910_MODE_FADE;
    Fade.flags                      = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS | 
                                        MODE_FLAG_HAS_PER_LED_COLOR | 
                                        MODE_FLAG_HAS_RANDOM_COLOR;
    Fade.speed_min                  = CLEVO8910_DIRECT_SPEED_MIN;
    Fade.speed_max                  = CLEVO8910_DIRECT_SPEED_MAX;
    Fade.brightness_min             = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Fade.brightness_max             = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Fade.speed                      = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Fade.brightness                 = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Fade.color_mode                 = MODE_COLORS_RANDOM;
    Fade.colors.resize(1);
    modes.push_back(Fade);

    mode Breathe;
<<<<<<< HEAD
    Breathe.name		    = "Breathing";
=======
    Breathe.name                    = "Breathing";
>>>>>>> 9154aedf (Change tab characters to spaces)
    Breathe.value                   = CLEVO8910_MODE_BREATHE;
    Breathe.flags                   = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS | 
                                        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | 
                                        MODE_FLAG_HAS_RANDOM_COLOR;
    Breathe.speed_min               = CLEVO8910_DIRECT_SPEED_MIN;
    Breathe.speed_max               = CLEVO8910_DIRECT_SPEED_MAX;
    Breathe.brightness_min          = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Breathe.brightness_max          = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Breathe.speed                   = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Breathe.brightness              = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Breathe.color_mode              = MODE_COLORS_RANDOM;
    Breathe.colors.resize(1);
    modes.push_back(Breathe);

    mode Flashing;
    Flashing.name                      = "Flashing";
    Flashing.value                     = CLEVO8910_MODE_FLASHING;
    Flashing.flags                     = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS | 
                                        MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | 
                                        MODE_FLAG_HAS_RANDOM_COLOR;
    Flashing.speed_min                 = CLEVO8910_DIRECT_SPEED_MIN;
    Flashing.speed_max                 = CLEVO8910_DIRECT_SPEED_MAX;
    Flashing.brightness_min            = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Flashing.brightness_max            = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Flashing.speed                     = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Flashing.brightness                = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Flashing.color_mode                = MODE_COLORS_RANDOM;
    Flashing.colors.resize(1);
    modes.push_back(Flashing);

    mode Blink;
    Blink.name                    = "Blink";
    Blink.value                   = CLEVO8910_TYPE_BLINK;
    Blink.flags                   = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Blink.speed_min               = CLEVO8910_DIRECT_SPEED_MIN;
    Blink.speed_max               = CLEVO8910_DIRECT_SPEED_MAX;
    Blink.brightness_min          = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Blink.brightness_max          = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Blink.speed                   = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Blink.brightness                = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Blink.color_mode                = MODE_COLORS_NONE;
    modes.push_back(Blink);

    mode Cycle;
    Cycle.name                      = "Spectrum Cycle";
    Cycle.value                     = CLEVO8910_TYPE_CYCLE;
    Cycle.flags                     = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Cycle.speed_min                 = CLEVO8910_DIRECT_SPEED_MIN;
    Cycle.speed_max                 = CLEVO8910_DIRECT_SPEED_MAX;
    Cycle.brightness_min            = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Cycle.brightness_max            = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Cycle.speed                     = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Cycle.brightness                = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Cycle.color_mode                = MODE_COLORS_NONE;
    modes.push_back(Cycle);

    mode Stop;
    Stop.name                       = "Stop";
    Stop.value                      = CLEVO8910_TYPE_STOP;
    Stop.flags                      = MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Stop.speed_min                  = CLEVO8910_DIRECT_SPEED_MIN;
    Stop.speed_max                  = CLEVO8910_DIRECT_SPEED_MAX;
    Stop.brightness_min             = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Stop.brightness_max             = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Stop.speed                      = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Stop.brightness                 = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Stop.color_mode                 = MODE_COLORS_NONE;
    modes.push_back(Stop);

    mode Noop;
    Noop.name                       = "Noop";
    Noop.value                      = CLEVO8910_TYPE_NOOP;
    Noop.flags                      = MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Noop.speed_min                  = CLEVO8910_DIRECT_SPEED_MIN;
    Noop.speed_max                  = CLEVO8910_DIRECT_SPEED_MAX;
    Noop.brightness_min             = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Noop.brightness_max             = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Noop.speed                      = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Noop.brightness                 = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Noop.color_mode                 = MODE_COLORS_NONE;
    modes.push_back(Noop);

    mode Wave;
    Wave.name                       = "Rainbow Wave";
    Wave.value                      = CLEVO8910_TYPE_WAVE;
    Wave.flags                      = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Wave.speed_min                  = CLEVO8910_DIRECT_SPEED_MIN;
    Wave.speed_max                  = CLEVO8910_DIRECT_SPEED_MAX;
    Wave.brightness_min             = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Wave.brightness_max             = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Wave.speed                      = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Wave.brightness                 = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Wave.color_mode                 = MODE_COLORS_NONE;
    modes.push_back(Wave);

    mode Random;
    Random.name                     = "Random";
    Random.value                    = CLEVO8910_TYPE_RANDOM;
    Random.flags                    = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Random.speed_min                = CLEVO8910_DIRECT_SPEED_MIN;
    Random.speed_max                = CLEVO8910_DIRECT_SPEED_MAX;
    Random.brightness_min           = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Random.brightness_max           = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Random.speed                    = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Random.brightness               = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Random.color_mode               = MODE_COLORS_NONE;
    modes.push_back(Random);

    mode Scan;
    Scan.name                       = "Scan";
    Scan.value                      = CLEVO8910_TYPE_SCAN;
    Scan.flags                      = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Scan.speed_min                  = CLEVO8910_DIRECT_SPEED_MIN;
    Scan.speed_max                  = CLEVO8910_DIRECT_SPEED_MAX;
    Scan.brightness_min             = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Scan.brightness_max             = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Scan.speed                      = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Scan.brightness                 = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Scan.color_mode                 = MODE_COLORS_NONE;
    modes.push_back(Scan);

    mode Snake;
    Snake.name                      = "Snake";
    Snake.value                     = CLEVO8910_TYPE_SNAKE;
    Snake.flags                     = MODE_FLAG_MANUAL_SAVE |
                                        MODE_FLAG_HAS_SPEED | 
                                        MODE_FLAG_HAS_BRIGHTNESS;
    Snake.speed_min                 = CLEVO8910_DIRECT_SPEED_MIN;
    Snake.speed_max                 = CLEVO8910_DIRECT_SPEED_MAX;
    Snake.brightness_min            = CLEVO8910_DIRECT_BRIGHTNESS_MIN;
    Snake.brightness_max            = CLEVO8910_DIRECT_BRIGHTNESS_MAX;
    Snake.speed                     = CLEVO8910_DIRECT_SPEED_DEFAULT;
    Snake.brightness                = CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT;
    Snake.color_mode                = MODE_COLORS_NONE;
    modes.push_back(Snake);

    mode Off;
    Off.name                        = "Off";
    Off.value                       = CLEVO8910_TYPE_OFF;
    Off.flags                       = 0;
    modes.push_back(Off);

    SetupZones();
}

RGBController_Clevo8910::~RGBController_Clevo8910()
{
    delete controller;
}

void RGBController_Clevo8910::SetupZones()
{
    leds.clear();
    leds.resize(CLEVO8910_KEYBOARD_LED_COUNT);
    for(size_t i = 0; i < leds.size(); ++i)
    {
        leds[i].name                    = clevo8910_leds[i].name;
        leds[i].value                   = clevo8910_leds[i].value;
    }

    matrix_map_type* new_zone_map       = new matrix_map_type;

    zone zone_keyboard;
    zone_keyboard.name                  = "Clevo Keyboard";
    zone_keyboard.type                  = ZONE_TYPE_MATRIX;
    zone_keyboard.name                  = "Keyboard";
    zone_keyboard.leds_min              = (unsigned int)leds.size();
    zone_keyboard.leds_max              = (unsigned int)leds.size();
    zone_keyboard.leds_count            = (unsigned int)leds.size();
    zone_keyboard.matrix_map            = new_zone_map;
    zone_keyboard.matrix_map->height    = CLEVO8910_KEYROWS;
    zone_keyboard.matrix_map->width     = CLEVO8910_KEYSINROW;
    zone_keyboard.matrix_map->map       = (unsigned int *)clevo8910_leds_map;
    zones.emplace_back(zone_keyboard);

    SetupColors();
}

void RGBController_Clevo8910::ResizeZone(int /*zone*/, int /*new_size*/)
{
    // This device does not support resizing zones               |
}

void RGBController_Clevo8910::DeviceUpdateLEDs()
{
    uint8_t effect = (uint8_t)(modes[active_mode].value & 0xFF);

    controller->SetColors(effect, leds, false, colors);
}

void RGBController_Clevo8910::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();  //Only one zone and same as device
}

void RGBController_Clevo8910::DeviceSaveMode()
{
    bool saveflg = (modes[active_mode].value != CLEVO8910_MODE_FADE && modes[active_mode].value != CLEVO8910_MODE_RIPPLE );
        
    controller->SaveBios(saveflg);
}

void RGBController_Clevo8910::UpdateSingleLED(int led)
{
    const RGBColor rndcolor = ToRGBColor(0, 0, 0);

    if(led != (int)NA)
    {
        uint8_t effect = (uint8_t)(modes[active_mode].value & 0xFF);
        /*---------------------------------------------------------*\
        | A hack is required for oneshot modes supported by the     |
        | device. In these modes, the colour can be random but      |
        | the OpenRGB does not suport this. Instead, we can         |
        | simulate the effect by using black which otherwise        |
        | would be a no-op.                                         |
        \*---------------------------------------------------------*/
        if (modes[active_mode].color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetSingleLED(effect, leds[led].value, rndcolor);
        }
        else
        {
            controller->SetSingleLED(effect, leds[led].value, colors[led]);
        }
    }

}

void RGBController_Clevo8910::DeviceUpdateMode()
{
    uint8_t effect = (uint8_t)(modes[active_mode].value & 0xFF);
    uint8_t key=0xAA;
    switch (modes[active_mode].value)
    {
    case CLEVO8910_TYPE_OFF:
        controller->TurnOff();
        break;
    case CLEVO8910_MODE_RIPPLE:
    case CLEVO8910_MODE_FADE:
    case CLEVO8910_MODE_DIRECT:
        controller->SetParams((uint8_t)modes[active_mode].brightness, (uint8_t)modes[active_mode].speed);
        if  (modes[active_mode].color_mode == MODE_COLORS_RANDOM)
            controller->SetColors(effect, leds, true, colors);
        break;

    case CLEVO8910_TYPE_BLINK:
    case CLEVO8910_TYPE_CYCLE:
    case CLEVO8910_TYPE_STOP:
    case CLEVO8910_TYPE_NOOP:
    case CLEVO8910_TYPE_WAVE:
    case CLEVO8910_TYPE_RANDOM:
    case CLEVO8910_TYPE_SCAN:
    case CLEVO8910_TYPE_SNAKE:
        controller->SetSimpleMode(effect, (uint8_t)modes[active_mode].brightness, (uint8_t)modes[active_mode].speed);
        break;

    case CLEVO8910_MODE_BREATHE:
    case CLEVO8910_MODE_FLASHING:
        key = (modes[active_mode].color_mode == MODE_COLORS_RANDOM) ? 0 : 0xAA;
        controller->SetMode(effect,
        (uint8_t)modes[active_mode].brightness, (uint8_t)modes[active_mode].speed, 
        key,
        modes[active_mode].colors[0] );
        break;

    default:
        controller->SetParams((uint8_t)modes[active_mode].brightness, (uint8_t)modes[active_mode].speed);
//      controller->SetColors(effect, leds, false, colors);
        break;
    }
}

/*---------------------------------------------------------*\
| RGBController_RoyuanController.cpp                        |
|                                                           |
|   RGBController for Royuan keyboard                       |
|                                                           |
|   Alvaro Munoz (alvaromunoz)                  05 Jun 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_RoyuanController.h"

/**------------------------------------------------------------------*\
    @name Royuan TH80 Pro
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectRoyuanControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_RoyuanController::RGBController_RoyuanController(RoyuanController* controller_ptr)
{
    controller  = controller_ptr;

    name                = controller->GetName();
    type                = DEVICE_TYPE_KEYBOARD;
    description         = controller->GetDescription();
    vendor              = controller->GetVendor();
    location            = controller->GetLocation();
    serial              = controller->GetSerialNumber();
    version             = controller->GetFirmwareVersion();

    mode Off;
    Off.name                                    = "Off";
    Off.value                                   = 0;
    Off.flags                                   = 0;
    Off.color_mode                              = MODE_COLORS_NONE;
    modes.push_back(Off);

    mode Always_on;
    Always_on.name                              = "Custom";
    Always_on.value                             = ROYUAN_MODE_ALWAYS_ON;
    Always_on.flags                             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Always_on.color_mode                        = MODE_COLORS_RANDOM;
    Always_on.brightness_min                    = ROYUAN_BRIGHTNESS_MIN;
    Always_on.brightness_max                    = ROYUAN_BRIGHTNESS_MAX;
    Always_on.brightness                        = ROYUAN_BRIGHTNESS_DEFAULT;
    modes.push_back(Always_on);

    mode Dynamic_breathing;
    Dynamic_breathing.name                      = "Breathing";
    Dynamic_breathing.value                     = ROYUAN_MODE_DYNAMIC_BREATHING;
    Dynamic_breathing.flags                     = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Dynamic_breathing.color_mode                = MODE_COLORS_RANDOM;
    Dynamic_breathing.brightness_min            = ROYUAN_BRIGHTNESS_MIN;
    Dynamic_breathing.brightness_max            = ROYUAN_BRIGHTNESS_MAX;
    Dynamic_breathing.brightness                = ROYUAN_BRIGHTNESS_DEFAULT;
    Dynamic_breathing.speed_min                 = ROYUAN_SPEED_MIN;
    Dynamic_breathing.speed_max                 = ROYUAN_SPEED_MAX;
    Dynamic_breathing.speed                     = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Dynamic_breathing);

    mode Spectrum_cycle;
    Spectrum_cycle.name                         = "Spectrum Cycle";
    Spectrum_cycle.value                        = ROYUAN_MODE_SPECTRUM_CYCLE;
    Spectrum_cycle.flags                        = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_AUTOMATIC_SAVE;
    Spectrum_cycle.color_mode                   = MODE_COLORS_NONE;
    Spectrum_cycle.brightness_min               = ROYUAN_BRIGHTNESS_MIN;
    Spectrum_cycle.brightness_max               = ROYUAN_BRIGHTNESS_MAX;
    Spectrum_cycle.brightness                   = ROYUAN_BRIGHTNESS_DEFAULT;
    Spectrum_cycle.speed_min                    = ROYUAN_SPEED_MIN;
    Spectrum_cycle.speed_max                    = ROYUAN_SPEED_MAX_SPECIAL;
    Spectrum_cycle.speed                        = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Spectrum_cycle);

    mode Drift;
    Drift.name                                  = "Drift";
    Drift.value                                 = ROYUAN_MODE_DRIFT;
    Drift.flags                                 = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_AUTOMATIC_SAVE;
    Drift.color_mode                            = MODE_COLORS_RANDOM;
    Drift.brightness_min                        = ROYUAN_BRIGHTNESS_MIN;
    Drift.brightness_max                        = ROYUAN_BRIGHTNESS_MAX;
    Drift.brightness                            = ROYUAN_BRIGHTNESS_DEFAULT;
    Drift.speed_min                             = ROYUAN_SPEED_MIN;
    Drift.speed_max                             = ROYUAN_SPEED_MAX;
    Drift.speed                                 = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Drift);

    mode Waves_ripple;
    Waves_ripple.name                           = "Waves ripple";
    Waves_ripple.value                          = ROYUAN_MODE_WAVES_RIPPLE;
    Waves_ripple.flags                          = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Waves_ripple.color_mode                     = MODE_COLORS_RANDOM;
    Waves_ripple.brightness_min                 = ROYUAN_BRIGHTNESS_MIN;
    Waves_ripple.brightness_max                 = ROYUAN_BRIGHTNESS_MAX;
    Waves_ripple.brightness                     = ROYUAN_BRIGHTNESS_DEFAULT;
    Waves_ripple.speed_min                      = ROYUAN_SPEED_MIN;
    Waves_ripple.speed_max                      = ROYUAN_SPEED_MAX;
    Waves_ripple.speed                          = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Waves_ripple);

    mode Stars_twinkle;
    Stars_twinkle.name                          = "Stars twinkle";
    Stars_twinkle.value                         = ROYUAN_MODE_STARS_TWINKLE;
    Stars_twinkle.flags                         = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Stars_twinkle.color_mode                    = MODE_COLORS_RANDOM;
    Stars_twinkle.brightness_min                = ROYUAN_BRIGHTNESS_MIN;
    Stars_twinkle.brightness_max                = ROYUAN_BRIGHTNESS_MAX;
    Stars_twinkle.brightness                    = ROYUAN_BRIGHTNESS_DEFAULT;
    Stars_twinkle.speed_min                     = ROYUAN_SPEED_MIN;
    Stars_twinkle.speed_max                     = ROYUAN_SPEED_MAX;
    Stars_twinkle.speed                         = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Stars_twinkle);

    mode Steady_stream;
    Steady_stream.name                          = "Steady stream";
    Steady_stream.value                         = ROYUAN_MODE_STEADY_STREAM;
    Steady_stream.flags                         = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    Steady_stream.color_mode                    = MODE_COLORS_RANDOM;
    Steady_stream.brightness_min                = ROYUAN_BRIGHTNESS_MIN;
    Steady_stream.brightness_max                = ROYUAN_BRIGHTNESS_MAX;
    Steady_stream.brightness                    = ROYUAN_BRIGHTNESS_DEFAULT;
    Steady_stream.speed_min                     = ROYUAN_SPEED_MIN;
    Steady_stream.speed_max                     = ROYUAN_SPEED_MAX;
    Steady_stream.speed                         = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Steady_stream);

    mode Shadowing;
    Shadowing.name                              = "Reactive";
    Shadowing.value                             = ROYUAN_MODE_SHADOWING;
    Shadowing.flags                             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Shadowing.color_mode                        = MODE_COLORS_RANDOM;
    Shadowing.brightness_min                    = ROYUAN_BRIGHTNESS_MIN;
    Shadowing.brightness_max                    = ROYUAN_BRIGHTNESS_MAX;
    Shadowing.brightness                        = ROYUAN_BRIGHTNESS_DEFAULT;
    Shadowing.speed_min                         = ROYUAN_SPEED_MIN;
    Shadowing.speed_max                         = ROYUAN_SPEED_MAX;
    Shadowing.speed                             = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Shadowing);

    mode Peaks_rising;
    Peaks_rising.name                           = "Peaks rising one after another";
    Peaks_rising.value                          = ROYUAN_MODE_PEAKS_RISING_ONE_AFTER_ANOTHER;
    Peaks_rising.flags                          = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Peaks_rising.color_mode                     = MODE_COLORS_RANDOM;
    Peaks_rising.brightness_min                 = ROYUAN_BRIGHTNESS_MIN;
    Peaks_rising.brightness_max                 = ROYUAN_BRIGHTNESS_MAX;
    Peaks_rising.brightness                     = ROYUAN_BRIGHTNESS_DEFAULT;
    Peaks_rising.speed_min                      = ROYUAN_SPEED_MIN;
    Peaks_rising.speed_max                      = ROYUAN_SPEED_MAX;
    Peaks_rising.speed                          = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Peaks_rising);

    mode Sine_wave;
    Sine_wave.name                              = "Sine wave";
    Sine_wave.value                             = ROYUAN_MODE_SINE_WAVE;
    Sine_wave.flags                             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Sine_wave.color_mode                        = MODE_COLORS_RANDOM;
    Sine_wave.brightness_min                    = ROYUAN_BRIGHTNESS_MIN;
    Sine_wave.brightness_max                    = ROYUAN_BRIGHTNESS_MAX;
    Sine_wave.brightness                        = ROYUAN_BRIGHTNESS_DEFAULT;
    Sine_wave.speed_min                         = ROYUAN_SPEED_MIN;
    Sine_wave.speed_max                         = ROYUAN_SPEED_MAX;
    Sine_wave.speed                             = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Sine_wave);

    mode Caispring;
    Caispring.name                              = "Caispring Surging";
    Caispring.value                             = ROYUAN_MODE_CAISPRING_SURGING;
    Caispring.flags                             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    Caispring.color_mode                        = MODE_COLORS_RANDOM;
    Caispring.brightness_min                    = ROYUAN_BRIGHTNESS_MIN;
    Caispring.brightness_max                    = ROYUAN_BRIGHTNESS_MAX;
    Caispring.brightness                        = ROYUAN_BRIGHTNESS_DEFAULT;
    Caispring.speed_min                         = ROYUAN_SPEED_MIN;
    Caispring.speed_max                         = ROYUAN_SPEED_MAX;
    Caispring.speed                             = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Caispring);

    mode Flowers_blooming;
    Flowers_blooming.name                       = "Flowers blooming";
    Flowers_blooming.value                      = ROYUAN_MODE_FLOWERS_BLOOMING;
    Flowers_blooming.flags                      = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    Flowers_blooming.color_mode                 = MODE_COLORS_RANDOM;
    Flowers_blooming.brightness_min             = ROYUAN_BRIGHTNESS_MIN;
    Flowers_blooming.brightness_max             = ROYUAN_BRIGHTNESS_MAX;
    Flowers_blooming.brightness                 = ROYUAN_BRIGHTNESS_DEFAULT;
    Flowers_blooming.speed_min                  = ROYUAN_SPEED_MIN;
    Flowers_blooming.speed_max                  = ROYUAN_SPEED_MAX;
    Flowers_blooming.speed                      = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Flowers_blooming);

    mode Laser;
    Laser.name                                  = "Laser";
    Laser.value                                 = ROYUAN_MODE_LASER;
    Laser.flags                                 = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Laser.color_mode                            = MODE_COLORS_RANDOM;
    Laser.brightness_min                        = ROYUAN_BRIGHTNESS_MIN;
    Laser.brightness_max                        = ROYUAN_BRIGHTNESS_MAX;
    Laser.brightness                            = ROYUAN_BRIGHTNESS_DEFAULT;
    Laser.speed_min                             = ROYUAN_SPEED_MIN;
    Laser.speed_max                             = ROYUAN_SPEED_MAX;
    Laser.speed                                 = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Laser);

    mode Peak_turn;
    Peak_turn.name                              = "Peak turn";
    Peak_turn.value                             = ROYUAN_MODE_PEAK_TURN;
    Peak_turn.flags                             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    Peak_turn.color_mode                        = MODE_COLORS_RANDOM;
    Peak_turn.brightness_min                    = ROYUAN_BRIGHTNESS_MIN;
    Peak_turn.brightness_max                    = ROYUAN_BRIGHTNESS_MAX;
    Peak_turn.brightness                        = ROYUAN_BRIGHTNESS_DEFAULT;
    Peak_turn.speed_min                         = ROYUAN_SPEED_MIN;
    Peak_turn.speed_max                         = ROYUAN_SPEED_MAX_SPECIAL;
    Peak_turn.speed                             = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Peak_turn);

    mode Inclined_rain;
    Inclined_rain.name                          = "Inclined Rain";
    Inclined_rain.value                         = ROYUAN_MODE_INCLINED_RAIN;
    Inclined_rain.flags                         = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Inclined_rain.color_mode                    = MODE_COLORS_RANDOM;
    Inclined_rain.brightness_min                = ROYUAN_BRIGHTNESS_MIN;
    Inclined_rain.brightness_max                = ROYUAN_BRIGHTNESS_MAX;
    Inclined_rain.brightness                    = ROYUAN_BRIGHTNESS_DEFAULT;
    Inclined_rain.speed_min                     = ROYUAN_SPEED_MIN;
    Inclined_rain.speed_max                     = ROYUAN_SPEED_MAX;
    Inclined_rain.speed                         = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Inclined_rain);

    mode Snow;
    Snow.name                                   = "Snow";
    Snow.value                                  = ROYUAN_MODE_SNOW;
    Snow.flags                                  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Snow.color_mode                             = MODE_COLORS_RANDOM;
    Snow.brightness_min                         = ROYUAN_BRIGHTNESS_MIN;
    Snow.brightness_max                         = ROYUAN_BRIGHTNESS_MAX;
    Snow.brightness                             = ROYUAN_BRIGHTNESS_DEFAULT;
    Snow.speed_min                              = ROYUAN_SPEED_MIN;
    Snow.speed_max                              = ROYUAN_SPEED_MAX_SPECIAL;
    Snow.speed                                  = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Snow);

    mode Meteor;
    Meteor.name                                 = "Meteor";
    Meteor.value                                = ROYUAN_MODE_METEOR;
    Meteor.flags                                = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Meteor.color_mode                           = MODE_COLORS_RANDOM;
    Meteor.brightness_min                       = ROYUAN_BRIGHTNESS_MIN;
    Meteor.brightness_max                       = ROYUAN_BRIGHTNESS_MAX;
    Meteor.brightness                           = ROYUAN_BRIGHTNESS_DEFAULT;
    Meteor.speed_min                            = ROYUAN_SPEED_MIN;
    Meteor.speed_max                            = ROYUAN_SPEED_MAX_SPECIAL;
    Meteor.speed                                = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Meteor);

    mode Through_the_snow;
    Through_the_snow.name                       = "Through the snow (non trace)";
    Through_the_snow.value                      = ROYUAN_MODE_THROUGH_THE_SNOW_NON_TRACE;
    Through_the_snow.flags                      = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Through_the_snow.color_mode                 = MODE_COLORS_RANDOM;
    Through_the_snow.brightness_min             = ROYUAN_BRIGHTNESS_MIN;
    Through_the_snow.brightness_max             = ROYUAN_BRIGHTNESS_MAX;
    Through_the_snow.brightness                 = ROYUAN_BRIGHTNESS_DEFAULT;
    Through_the_snow.speed_min                  = ROYUAN_SPEED_MIN;
    Through_the_snow.speed_max                  = ROYUAN_SPEED_MAX_SPECIAL;
    Through_the_snow.speed                      = ROYUAN_SPEED_DEFAULT;
    modes.push_back(Through_the_snow);

    SetupZones();
}

RGBController_RoyuanController::~RGBController_RoyuanController()
{
    delete controller;
}

void RGBController_RoyuanController::DeviceUpdateLEDs()
{
    unsigned char red = RGBGetRValue(colors[0]);
    unsigned char grn = RGBGetGValue(colors[0]);
    unsigned char blu = RGBGetBValue(colors[0]);

    controller->SetDazzle(modes[active_mode].color_mode == MODE_COLORS_RANDOM);
    controller->SetColor(red, grn, blu);
}

void RGBController_RoyuanController::DeviceUpdateMode()
{
    if(modes[active_mode].value == ROYUAN_MODE_DRIFT)
    {
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            controller->SetOption(ROYUAN_OPTION_DRIFT_LEFT);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_RIGHT)
        {
            controller->SetOption(ROYUAN_OPTION_DRIFT_RIGHT);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_UP)
        {
            controller->SetOption(ROYUAN_OPTION_DRIFT_UP);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_DOWN)
        {
            controller->SetOption(ROYUAN_OPTION_DRIFT_DOWN);
        }
    }
    else if(modes[active_mode].value == ROYUAN_MODE_STEADY_STREAM)
    {
        /*---------------------------------------------------------*\
        | TODO: These OPTIONS (zig-zag, return) should not          |
        |   be DIRECTIONS (left, right)                             |
        \*---------------------------------------------------------*/
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            controller->SetOption(ROYUAN_OPTION_STEADY_STREAM_ZIG_ZAG);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_RIGHT)
        {
            controller->SetOption(ROYUAN_OPTION_STEADY_STREAM_RETURN);
        }
    }
    else if(modes[active_mode].value == ROYUAN_MODE_CAISPRING_SURGING)
    {
        /*---------------------------------------------------------*\
        | TODO: These OPTIONS (in, out) should not                  |
        |   be DIRECTIONS (left, right)                             |
        \*---------------------------------------------------------*/
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            controller->SetOption(ROYUAN_OPTION_CAISPRING_SURGING_OUT);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_RIGHT)
        {
            controller->SetOption(ROYUAN_OPTION_CAISPRING_SURGING_IN);
        }
    }
    else if(modes[active_mode].value == ROYUAN_MODE_FLOWERS_BLOOMING)
    {
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            controller->SetOption(ROYUAN_OPTION_FLOWERS_BLOOMING_LEFT);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_RIGHT)
        {
            controller->SetOption(ROYUAN_OPTION_FLOWERS_BLOOMING_RIGHT);
        }
    }
    else if(modes[active_mode].value == ROYUAN_MODE_PEAK_TURN)
    {
        /*---------------------------------------------------------*\
        | TODO: These OPTIONS (clockwise, anti-clockwise)           |
        |   should not be DIRECTIONS (left, right)                  |
        \*---------------------------------------------------------*/
        if(modes[active_mode].direction == MODE_DIRECTION_LEFT)
        {
            controller->SetOption(ROYUAN_OPTION_PEAK_TURN_ANTI_CLOCKWISE);
        }
        else if(modes[active_mode].direction == MODE_DIRECTION_RIGHT)
        {
            controller->SetOption(ROYUAN_OPTION_PEAK_TURN_CLOCKWISE);
        }
    }
    else
    {
        controller->SetOption(ROYUAN_OPTION_DEFAULT);
    }

    controller->SetDazzle(modes[active_mode].color_mode == MODE_COLORS_RANDOM);

    controller->SetMode(modes[active_mode].value, modes[active_mode].speed, modes[active_mode].brightness);
}

void RGBController_RoyuanController::SetupZones()
{
    zone new_zone;
    new_zone.name          = "Keyboard";
    new_zone.type          = ZONE_TYPE_SINGLE;
    new_zone.leds_min      = 1;
    new_zone.leds_max      = 1;
    new_zone.leds_count    = 1;
    new_zone.matrix_map    = NULL;

    zones.push_back(new_zone);

    led keyboard_led;
    keyboard_led.name = "Keyboard LEDs";
    keyboard_led.value = 0x00;
    leds.push_back(keyboard_led);

    SetupColors();
}


void RGBController_RoyuanController::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | Not implemented                                           |
    \*---------------------------------------------------------*/
}

void RGBController_RoyuanController::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyuanController::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

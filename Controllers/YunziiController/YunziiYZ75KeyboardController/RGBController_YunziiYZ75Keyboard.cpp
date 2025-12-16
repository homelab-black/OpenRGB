/*---------------------------------------------------------*\
| RGBController_YunziiYZ75Keyboard.cpp                      |
|                                                           |
|   RGBController for Yunzii YZ75 keyboard                  |
|                                                           |
|   Kosta A. (kostaarvanitis)                   23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#include "RGBController_YunziiYZ75Keyboard.h"
#include "LogManager.h"

/**------------------------------------------------------------------*\
    @name Yunzii YZ75 Keyboard
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectYunziiYZ75Keyboard
    @comment
        OpenRGB does not support per-zone effect modes, so we list
        a combination of modes for both back light and light strip
        options. Sound based and image effects are not supported.
\*-------------------------------------------------------------------*/

RGBController_YunziiYZ75Keyboard::RGBController_YunziiYZ75Keyboard(YunziiYZ75KeyboardController* controller_ptr)
{
    controller                              = controller_ptr;

    name                                    = controller->GetName();
    type                                    = DEVICE_TYPE_KEYBOARD;
    description                             = "Yunzii YZ75";
    vendor                                  = controller->GetVendor();
    location                                = controller->GetLocation();
    serial                                  = controller->GetSerialNumber();
    version                                 = controller->GetFirmwareVersion();

    /*---------------------------------------------------------*\
    | Light modes                                               |
    \*---------------------------------------------------------*/

    mode Off;
    Off.name                                = "Off";
    Off.flags                               = MODE_FLAG_AUTOMATIC_SAVE;
    Off.color_mode                          = MODE_COLORS_NONE;
    Off.value                               = YUNZII_MODE_OFF;
    modes.push_back(Off);
    
    mode Custom;
    Custom.name                             = "Custom";
    Custom.flags                            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Custom.color_mode                       = MODE_COLORS_PER_LED;
    Custom.brightness_min                   = 0;
    Custom.brightness_max                   = 4;
    Custom.brightness                       = 2;
    Custom.value                            = YUNZII_MODE_ALWAYS_ON;
    modes.push_back(Custom);

    mode DynamicBreathing;
    DynamicBreathing.name                   = "Dynamic Breathing";
    DynamicBreathing.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    DynamicBreathing.color_mode             = MODE_COLORS_RANDOM;
    DynamicBreathing.colors_min             = 1;
    DynamicBreathing.colors_max             = 1;
    DynamicBreathing.colors.resize(2, ToRGBColor(255, 0, 0));
    DynamicBreathing.speed_min              = 4;
    DynamicBreathing.speed_max              = 0;
    DynamicBreathing.speed                  = 2;
    DynamicBreathing.brightness_min         = 0;
    DynamicBreathing.brightness_max         = 4;
    DynamicBreathing.brightness             = 2;
    DynamicBreathing.value                  = YUNZII_MODE_DYNAMIC_BREATHING;
    modes.push_back(DynamicBreathing);

    mode SpectrumCycle;
    SpectrumCycle.name                      = "Spectrum Cycle";
    SpectrumCycle.flags                     = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    SpectrumCycle.color_mode                = MODE_COLORS_NONE;
    SpectrumCycle.speed_min                 = 4;
    SpectrumCycle.speed_max                 = 0;
    SpectrumCycle.speed                     = 2;
    SpectrumCycle.brightness_min            = 0;
    SpectrumCycle.brightness_max            = 4;
    SpectrumCycle.brightness                = 2;
    SpectrumCycle.value                     = YUNZII_MODE_SPECTRUM_CYCLE;
    modes.push_back(SpectrumCycle);

    mode Drift;
    Drift.name                              = "Drift";
    Drift.flags                             = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_DIRECTION_LR;
    Drift.color_mode                        = MODE_COLORS_RANDOM;
    Drift.colors_min                        = 1;
    Drift.colors_max                        = 1;
    Drift.colors.resize(1, ToRGBColor(255, 0, 0));
    Drift.speed_min                         = 4;
    Drift.speed_max                         = 0;
    Drift.speed                             = 2;
    Drift.brightness_min                    = 0;
    Drift.brightness_max                    = 4;
    Drift.brightness                        = 2;
    Drift.direction                         = MODE_DIRECTION_UP;
    Drift.value                             = YUNZII_MODE_DRIFT;
    modes.push_back(Drift);

    mode Ripple;
    Ripple.name                             = "Ripple";
    Ripple.flags                            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Ripple.color_mode                       = MODE_COLORS_RANDOM;
    Ripple.colors_min                       = 1;
    Ripple.colors_max                       = 1;
    Ripple.colors.resize(1, ToRGBColor(255, 0, 0));
    Ripple.speed_min                        = 4;
    Ripple.speed_max                        = 0;
    Ripple.speed                            = 2;
    Ripple.brightness_min                   = 0;
    Ripple.brightness_max                   = 4;
    Ripple.brightness                       = 2;
    Ripple.value                            = YUNZII_MODE_RIPPLE;
    modes.push_back(Ripple);

    mode StarsTwinkly;
    StarsTwinkly.name                       = "Stars Twinkly";
    StarsTwinkly.flags                      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    StarsTwinkly.color_mode                 = MODE_COLORS_RANDOM;
    StarsTwinkly.colors_min                 = 1;
    StarsTwinkly.colors_max                 = 1;
    StarsTwinkly.colors.resize(1, ToRGBColor(255, 0, 0));
    StarsTwinkly.speed_min                  = 4;
    StarsTwinkly.speed_max                  = 0;
    StarsTwinkly.speed                      = 2;
    StarsTwinkly.brightness_min             = 0;
    StarsTwinkly.brightness_max             = 4;
    StarsTwinkly.brightness                 = 2;
    StarsTwinkly.value                      = YUNZII_MODE_STARS_TWINKLY;
    modes.push_back(StarsTwinkly);

    mode SteadyStream;
    SteadyStream.name                       = "Steady Stream";
    SteadyStream.flags                      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    SteadyStream.color_mode                 = MODE_COLORS_RANDOM;
    SteadyStream.colors_min                 = 1;
    SteadyStream.colors_max                 = 1;
    SteadyStream.colors.resize(1, ToRGBColor(255, 0, 0));
    SteadyStream.speed_min                  = 4;
    SteadyStream.speed_max                  = 0;
    SteadyStream.speed                      = 2;
    SteadyStream.brightness_min             = 0;
    SteadyStream.brightness_max             = 4;
    SteadyStream.brightness                 = 2;
    SteadyStream.direction                  = MODE_DIRECTION_LEFT;  // Zig-Zag vs Return
    SteadyStream.value                      = YUNZII_MODE_STEADY_STREAM;
    modes.push_back(SteadyStream);
    
    mode Shadowing;
    Shadowing.name                          = "Shadowing";
    Shadowing.flags                         = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Shadowing.color_mode                    = MODE_COLORS_RANDOM;
    Shadowing.colors_min                    = 1;
    Shadowing.colors_max                    = 1;
    Shadowing.colors.resize(1, ToRGBColor(255, 0, 0));
    Shadowing.speed_min                     = 4;
    Shadowing.speed_max                     = 0;
    Shadowing.speed                         = 2;
    Shadowing.brightness_min                = 0;
    Shadowing.brightness_max                = 4;
    Shadowing.brightness                    = 2;
    Shadowing.value                         = YUNZII_MODE_SHADOWING;
    modes.push_back(Shadowing);

    mode PeaksRising;
    PeaksRising.name                        = "Peaks Rising";
    PeaksRising.flags                       = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    PeaksRising.color_mode                  = MODE_COLORS_RANDOM;
    PeaksRising.colors_min                  = 1;
    PeaksRising.colors_max                  = 1;
    PeaksRising.colors.resize(1, ToRGBColor(255, 0, 0));
    PeaksRising.speed_min                   = 4;
    PeaksRising.speed_max                   = 0;
    PeaksRising.speed                       = 2;
    PeaksRising.brightness_min              = 0;
    PeaksRising.brightness_max              = 4;
    PeaksRising.brightness                  = 2;
    PeaksRising.value                       = YUNZII_MODE_PEAKS_RISING;
    modes.push_back(PeaksRising);

    mode Sinewave;
    Sinewave.name                           = "Sine Wave";
    Sinewave.flags                          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Sinewave.color_mode                     = MODE_COLORS_RANDOM;
    Sinewave.colors_min                     = 1;
    Sinewave.colors_max                     = 1;
    Sinewave.colors.resize(1, ToRGBColor(255, 0, 0));
    Sinewave.speed_min                      = 4;
    Sinewave.speed_max                      = 0;
    Sinewave.speed                          = 2;
    Sinewave.brightness_min                 = 0;
    Sinewave.brightness_max                 = 4;
    Sinewave.brightness                     = 2;
    Sinewave.value                          = YUNZII_MODE_SINE_WAVE;
    modes.push_back(Sinewave);

    mode CaispringSurging;
    CaispringSurging.name                   = "Caispring Surging";
    CaispringSurging.flags                  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    CaispringSurging.color_mode             = MODE_COLORS_RANDOM;
    CaispringSurging.colors_min             = 1;
    CaispringSurging.colors_max             = 1;
    CaispringSurging.colors.resize(1, ToRGBColor(255, 0, 0));
    CaispringSurging.speed_min              = 4;
    CaispringSurging.speed_max              = 0;
    CaispringSurging.speed                  = 2;
    CaispringSurging.brightness_min         = 0;
    CaispringSurging.brightness_max         = 4;
    CaispringSurging.brightness             = 2;
    CaispringSurging.direction              = MODE_DIRECTION_LEFT;  // Zig-Zag vs Return
    CaispringSurging.value                  = YUNZII_MODE_CAISPRING_SURGING;
    modes.push_back(CaispringSurging);

    mode FlowersBlooming;
    FlowersBlooming.name                    = "Flowers Blooming";
    FlowersBlooming.flags                   = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    FlowersBlooming.color_mode              = MODE_COLORS_RANDOM;
    FlowersBlooming.colors_min              = 1;
    FlowersBlooming.colors_max              = 1;
    FlowersBlooming.colors.resize(1, ToRGBColor(255, 0, 0));
    FlowersBlooming.speed_min               = 4;
    FlowersBlooming.speed_max               = 0;
    FlowersBlooming.speed                   = 2;
    FlowersBlooming.brightness_min          = 0;
    FlowersBlooming.brightness_max          = 4;
    FlowersBlooming.brightness              = 2;
    FlowersBlooming.direction               = MODE_DIRECTION_LEFT;  // Zig-Zag vs Return
    FlowersBlooming.value                   = YUNZII_MODE_FLOWERS_BLOOMING;
    modes.push_back(FlowersBlooming);

    mode Laser;
    Laser.name                              = "Laser";
    Laser.flags                             = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Laser.color_mode                        = MODE_COLORS_RANDOM;
    Laser.colors_min                        = 1;
    Laser.colors_max                        = 1;
    Laser.colors.resize(1, ToRGBColor(255, 0, 0));
    Laser.speed_min                         = 4;
    Laser.speed_max                         = 0;
    Laser.speed                             = 2;
    Laser.brightness_min                    = 0;
    Laser.brightness_max                    = 4;
    Laser.brightness                        = 2;
    Laser.value                             = YUNZII_MODE_LASER;
    modes.push_back(Laser);

    mode PeakTurn;
    PeakTurn.name                           = "PeakTurn";
    PeakTurn.flags                          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    PeakTurn.color_mode                     = MODE_COLORS_RANDOM;
    PeakTurn.colors_min                     = 1;
    PeakTurn.colors_max                     = 1;
    PeakTurn.colors.resize(1, ToRGBColor(255, 0, 0));
    PeakTurn.speed_min                      = 4;
    PeakTurn.speed_max                      = 0;
    PeakTurn.speed                          = 2;
    PeakTurn.brightness_min                 = 0;
    PeakTurn.brightness_max                 = 4;
    PeakTurn.brightness                     = 2;
    PeakTurn.direction                      = MODE_DIRECTION_LEFT;  // Clockwise vs Anti-Clockwise
    PeakTurn.value                          = YUNZII_MODE_PEAK_TURN;
    modes.push_back(PeakTurn);

    mode Wave;
    Wave.name                               = "Wave";
    Wave.flags                              = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Wave.color_mode                         = MODE_COLORS_RANDOM;
    Wave.colors_min                         = 1;
    Wave.colors_max                         = 1;
    Wave.colors.resize(1, ToRGBColor(255, 0, 0));
    Wave.speed_min                          = 4;
    Wave.speed_max                          = 0;
    Wave.speed                              = 2;
    Wave.brightness_min                     = 0;
    Wave.brightness_max                     = 4;
    Wave.brightness                         = 2;
    Wave.value                              = YUNZII_MODE_WAVE;
    modes.push_back(Wave);

    mode Neon;
    Neon.name                               = "Neon";
    Neon.flags                              = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Neon.color_mode                         = MODE_COLORS_RANDOM;
    Neon.colors_min                         = 1;
    Neon.colors_max                         = 1;
    Neon.colors.resize(1, ToRGBColor(255, 0, 0));
    Neon.speed_min                          = 4;
    Neon.speed_max                          = 0;
    Neon.speed                              = 2;
    Neon.brightness_min                     = 0;
    Neon.brightness_max                     = 4;
    Neon.brightness                         = 2;
    Neon.value                              = YUNZII_MODE_NEON;
    modes.push_back(Neon);

    mode Snake;
    Snake.name                              = "Snake";
    Snake.flags                             = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Snake.color_mode                        = MODE_COLORS_RANDOM;
    Snake.colors_min                        = 1;
    Snake.colors_max                        = 1;
    Snake.colors.resize(1, ToRGBColor(255, 0, 0));
    Snake.speed_min                         = 4;
    Snake.speed_max                         = 0;
    Snake.speed                             = 2;
    Snake.brightness_min                    = 0;
    Snake.brightness_max                    = 4;
    Snake.brightness                        = 2;
    Snake.value                             = YUNZII_MODE_SNAKE;
    modes.push_back(Snake);

    mode ThroughSnow;
    ThroughSnow.name                        = "Through the Snow";
    ThroughSnow.flags                       = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    ThroughSnow.color_mode                  = MODE_COLORS_RANDOM;
    ThroughSnow.colors_min                  = 1;
    ThroughSnow.colors_max                  = 1;
    ThroughSnow.colors.resize(1, ToRGBColor(255, 0, 0));
    ThroughSnow.speed_min                   = 4;
    ThroughSnow.speed_max                   = 0;
    ThroughSnow.speed                       = 2;
    ThroughSnow.brightness_min              = 0;
    ThroughSnow.brightness_max              = 4;
    ThroughSnow.brightness                  = 2;
    ThroughSnow.value                       = YUNZII_MODE_THROUGH_THE_SNOW;
    modes.push_back(ThroughSnow);

    SetupZones();
}

void RGBController_YunziiYZ75Keyboard::SetupZones()
{
    zone kdb_zone;
    kdb_zone.name       = "Keyboard";
    kdb_zone.type       = ZONE_TYPE_SINGLE;
    kdb_zone.leds_count = 1;
    kdb_zone.leds_min   = 1;
    kdb_zone.leds_max   = 1;
    kdb_zone.matrix_map = NULL;
    zones.push_back(kdb_zone);

    led kdb_led;
    kdb_led.name = "Keyboard LED";
    leds.push_back(kdb_led);

    zone side_zone;
    side_zone.name       = "Side";
    side_zone.type       = ZONE_TYPE_SINGLE;
    side_zone.leds_count = 1;
    side_zone.leds_min   = 1;
    side_zone.leds_max   = 1;
    side_zone.matrix_map = NULL;
    zones.push_back(side_zone);

    led side_led;
    side_led.name = "Side LED";
    leds.push_back(side_led);

    SetupColors();
}

RGBController_YunziiYZ75Keyboard::~RGBController_YunziiYZ75Keyboard()
{
    delete controller;
}

void RGBController_YunziiYZ75Keyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_YunziiYZ75Keyboard::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_YunziiYZ75Keyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_YunziiYZ75Keyboard::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_YunziiYZ75Keyboard::DeviceUpdateMode()
{
    int flags = modes[active_mode].flags;
    int value = modes[active_mode].value;

    int speed = modes[active_mode].speed_max;
    if(flags & MODE_FLAG_HAS_SPEED)
    {
        speed = modes[active_mode].speed;
    }

    int brightness = modes[active_mode].brightness_max;
    if(flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness = modes[active_mode].brightness;
    }

    int direction = 0;
    if(flags & (MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_DIRECTION_HV))
    {
        direction = modes[active_mode].direction;
    }

    int color_mode = modes[active_mode].color_mode;

    RGBColor* color = 0;
    if(color_mode == MODE_COLORS_PER_LED)
    {
        color = &colors[0];
    }
    else if(color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        color = &modes[active_mode].colors[0];
    }

    bool use_random = false;
    if(color_mode == MODE_COLORS_RANDOM)
    {
        use_random = true;
    }

    // Keyboard zone
    controller->SetLightMode(value, color, use_random, speed, brightness, direction);
    
    if (color) color++;

    // Side zone
    controller->SetLightStripMode(value, color, use_random, speed, brightness, direction);
}

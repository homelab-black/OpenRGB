/*---------------------------------------------------------*\
| RGBController_EVisionKeyboard.cpp                         |
|                                                           |
|   RGBController for EVision keyboard (Redragon, Glorious, |
|   Ajazz, Tecware, and many other brands)                  |
|                                                           |
|   Adam Honse (CalcProgrammer1)                25 Mar 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_EVisionKeyboard.h"

//0xFFFFFFFF indicates an unused entry in matrix
#define NA  0xFFFFFFFF

static unsigned int matrix_map[6][23] =
    { {   0,  NA,  1,    2,   3,   4,  NA,   5,   6,   7,   8,  NA,   9,  10,  11,  12,  14,  15,  16,  NA,  NA,  NA,  NA },
      {  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  NA,  32,  33,  34,  NA,  35,  36,  37,  38,  39,  40,  41 },
      {  42,  NA,  43,  44,  45,  46,  NA,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62 },
      {  63,  NA,  64,  65,  66,  67,  NA,  68,  69,  70,  71,  72,  73,  74,  76,  NA,  NA,  NA,  NA,  80,  81,  82,  NA },
      {  84,  NA,  86,  87,  88,  89,  NA,  90,  NA,  91,  92,  93,  94,  95,  97,  NA,  NA,  99,  NA, 101, 102, 103, 104 },
      { 105, 106, 107,  NA,  NA,  NA,  NA, 108,  NA,  NA,  NA,  NA, 109, 110, 111, 113, 119, 120, 121, 123,  NA, 124,  NA } };

/**------------------------------------------------------------------*\
    @name EVision Keyboard
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectEVisionKeyboards,DetectEVisionKeyboardsSharkoonSGK3
    @comment The Sharkoon Skiller Mech SGK3 uses a model-specific ISO layout; colors and effects are written to the active onboard profile.
\*-------------------------------------------------------------------*/

RGBController_EVisionKeyboard::RGBController_EVisionKeyboard(EVisionKeyboardController* controller_ptr, const evision_keyboard_layout* layout_ptr)
{
    controller                      = controller_ptr;
    layout                          = layout_ptr;

    name                            = controller->GetNameString();
    vendor                          = "EVision";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "EVision Keyboard Device";

    if(layout != nullptr)
    {
        vendor                      = layout->vendor;
        description                 = layout->description;

        /*-------------------------------------------------*\
        | Write to the onboard profile the keyboard is      |
        | currently showing                                 |
        \*-------------------------------------------------*/
        int active_profile          = controller->ReadActiveProfile();

        if(active_profile > 0 && active_profile < 3)
        {
            controller->SetProfile((unsigned char)active_profile);
        }

        controller->SetVerifyReplies(layout->use_begin_end);
    }
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();

    mode Custom;
    Custom.name                     = "Custom";
    Custom.value                    = EVISION_KB_MODE_CUSTOM;
    Custom.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Custom.brightness_min           = EVISION_KB_BRIGHTNESS_LOWEST;
    Custom.brightness_max           = EVISION_KB_BRIGHTNESS_HIGHEST;
    Custom.brightness               = EVISION_KB_BRIGHTNESS_HIGHEST;
    Custom.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    mode ColorWave;
    ColorWave.name                  = "Color Wave";
    ColorWave.value                 = EVISION_KB_MODE_COLOR_WAVE_LONG;
    ColorWave.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWave.speed_min             = EVISION_KB_SPEED_SLOWEST;
    ColorWave.speed_max             = EVISION_KB_SPEED_FASTEST;
    ColorWave.speed                 = EVISION_KB_SPEED_NORMAL;
    ColorWave.brightness_min        = EVISION_KB_BRIGHTNESS_LOWEST;
    ColorWave.brightness_max        = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWave.brightness            = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWave.direction             = MODE_DIRECTION_LEFT;
    ColorWave.colors_min            = 1;
    ColorWave.colors_max            = 1;
    ColorWave.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    ColorWave.colors.resize(1);
    modes.push_back(ColorWave);

    mode ColorWaveShort;
    ColorWaveShort.name             = "Color Wave (Short)";
    ColorWaveShort.value            = EVISION_KB_MODE_COLOR_WAVE_SHORT;
    ColorWaveShort.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWaveShort.speed_min        = EVISION_KB_SPEED_SLOWEST;
    ColorWaveShort.speed_max        = EVISION_KB_SPEED_FASTEST;
    ColorWaveShort.speed            = EVISION_KB_SPEED_NORMAL;
    ColorWaveShort.brightness_min   = EVISION_KB_BRIGHTNESS_LOWEST;
    ColorWaveShort.brightness_max   = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWaveShort.brightness       = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWaveShort.direction        = MODE_DIRECTION_LEFT;
    ColorWaveShort.colors_min       = 1;
    ColorWaveShort.colors_max       = 1;
    ColorWaveShort.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    ColorWaveShort.colors.resize(1);
    modes.push_back(ColorWaveShort);

    mode ColorWheel;
    ColorWheel.name                 = "Color Wheel";
    ColorWheel.value                = EVISION_KB_MODE_COLOR_WHEEL;
    ColorWheel.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWheel.speed_min            = EVISION_KB_SPEED_SLOWEST;
    ColorWheel.speed_max            = EVISION_KB_SPEED_FASTEST;
    ColorWheel.speed                = EVISION_KB_SPEED_NORMAL;
    ColorWheel.brightness_min       = EVISION_KB_BRIGHTNESS_LOWEST;
    ColorWheel.brightness_max       = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWheel.brightness           = EVISION_KB_BRIGHTNESS_HIGHEST;
    ColorWheel.direction            = MODE_DIRECTION_LEFT;
    ColorWheel.colors_min           = 1;
    ColorWheel.colors_max           = 1;
    ColorWheel.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    ColorWheel.colors.resize(1);
    modes.push_back(ColorWheel);

    mode SpectrumCycle;
    SpectrumCycle.name              = "Spectrum Cycle";
    SpectrumCycle.value             = EVISION_KB_MODE_SPECTRUM_CYCLE;
    SpectrumCycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumCycle.speed_min         = EVISION_KB_SPEED_SLOWEST;
    SpectrumCycle.speed_max         = EVISION_KB_SPEED_FASTEST;
    SpectrumCycle.speed             = EVISION_KB_SPEED_NORMAL;
    SpectrumCycle.brightness_min    = EVISION_KB_BRIGHTNESS_LOWEST;
    SpectrumCycle.brightness_max    = EVISION_KB_BRIGHTNESS_HIGHEST;
    SpectrumCycle.brightness        = EVISION_KB_BRIGHTNESS_HIGHEST;
    SpectrumCycle.color_mode        = MODE_COLORS_NONE;
    modes.push_back(SpectrumCycle);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.value                 = EVISION_KB_MODE_BREATHING;
    Breathing.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min             = EVISION_KB_SPEED_SLOWEST;
    Breathing.speed_max             = EVISION_KB_SPEED_FASTEST;
    Breathing.speed                 = EVISION_KB_SPEED_NORMAL;
    Breathing.brightness_min        = EVISION_KB_BRIGHTNESS_LOWEST;
    Breathing.brightness_max        = EVISION_KB_BRIGHTNESS_HIGHEST;
    Breathing.brightness            = EVISION_KB_BRIGHTNESS_HIGHEST;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    mode Hurricane;
    Hurricane.name                  = "Hurricane";
    Hurricane.value                 = EVISION_KB_MODE_HURRICANE;
    Hurricane.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Hurricane.speed_min             = EVISION_KB_SPEED_SLOWEST;
    Hurricane.speed_max             = EVISION_KB_SPEED_FASTEST;
    Hurricane.speed                 = EVISION_KB_SPEED_NORMAL;
    Hurricane.brightness_min        = EVISION_KB_BRIGHTNESS_LOWEST;
    Hurricane.brightness_max        = EVISION_KB_BRIGHTNESS_HIGHEST;
    Hurricane.brightness            = EVISION_KB_BRIGHTNESS_HIGHEST;
    Hurricane.colors_min            = 1;
    Hurricane.colors_max            = 1;
    Hurricane.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Hurricane.colors.resize(1);
    modes.push_back(Hurricane);

    mode Accumulate;
    Accumulate.name                 = "Accumulate";
    Accumulate.value                = EVISION_KB_MODE_ACCUMULATE;
    Accumulate.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Accumulate.speed_min            = EVISION_KB_SPEED_SLOWEST;
    Accumulate.speed_max            = EVISION_KB_SPEED_FASTEST;
    Accumulate.speed                = EVISION_KB_SPEED_NORMAL;
    Accumulate.brightness_min       = EVISION_KB_BRIGHTNESS_LOWEST;
    Accumulate.brightness_max       = EVISION_KB_BRIGHTNESS_HIGHEST;
    Accumulate.brightness           = EVISION_KB_BRIGHTNESS_HIGHEST;
    Accumulate.colors_min           = 1;
    Accumulate.colors_max           = 1;
    Accumulate.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Accumulate.colors.resize(1);
    modes.push_back(Accumulate);

    mode Starlight;
    Starlight.name                  = "Starlight";
    Starlight.value                 = EVISION_KB_MODE_STARLIGHT_FAST;
    Starlight.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Starlight.speed_min             = EVISION_KB_SPEED_SLOWEST;
    Starlight.speed_max             = EVISION_KB_SPEED_FASTEST;
    Starlight.speed                 = EVISION_KB_SPEED_NORMAL;
    Starlight.brightness_min        = EVISION_KB_BRIGHTNESS_LOWEST;
    Starlight.brightness_max        = EVISION_KB_BRIGHTNESS_HIGHEST;
    Starlight.brightness            = EVISION_KB_BRIGHTNESS_HIGHEST;
    Starlight.colors_min            = 1;
    Starlight.colors_max            = 1;
    Starlight.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Starlight.colors.resize(1);
    modes.push_back(Starlight);

    mode Visor;
    Visor.name                      = "Visor";
    Visor.value                     = EVISION_KB_MODE_VISOR;
    Visor.flags                     = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Visor.brightness_min            = EVISION_KB_BRIGHTNESS_LOWEST;
    Visor.brightness_max            = EVISION_KB_BRIGHTNESS_HIGHEST;
    Visor.brightness                = EVISION_KB_BRIGHTNESS_HIGHEST;
    Visor.colors_min                = 1;
    Visor.colors_max                = 1;
    Visor.color_mode                = MODE_COLORS_MODE_SPECIFIC;
    Visor.colors.resize(1);
    modes.push_back(Visor);

    mode Static;
    Static.name                     = "Static";
    Static.value                    = EVISION_KB_MODE_STATIC;
    Static.flags                    = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Static.brightness_min           = EVISION_KB_BRIGHTNESS_LOWEST;
    Static.brightness_max           = EVISION_KB_BRIGHTNESS_HIGHEST;
    Static.brightness               = EVISION_KB_BRIGHTNESS_HIGHEST;
    Static.colors_min               = 1;
    Static.colors_max               = 1;
    Static.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode RainbowCircle;
    RainbowCircle.name              = "Rainbow Circle";
    RainbowCircle.value             = EVISION_KB_MODE_RAINBOW_WAVE_CIRCLE;
    RainbowCircle.flags             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    RainbowCircle.brightness_min    = EVISION_KB_BRIGHTNESS_LOWEST;
    RainbowCircle.brightness_max    = EVISION_KB_BRIGHTNESS_HIGHEST;
    RainbowCircle.brightness        = EVISION_KB_BRIGHTNESS_HIGHEST;
    RainbowCircle.color_mode        = MODE_COLORS_RANDOM;
    modes.push_back(RainbowCircle);

    mode VerticalRainbow;
    VerticalRainbow.name            = "Vertical Rainbow";
    VerticalRainbow.value           = EVISION_KB_MODE_RAINBOW_WAVE_VERTICAL;
    VerticalRainbow.flags           = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_AUTOMATIC_SAVE;
    VerticalRainbow.speed_min       = EVISION_KB_SPEED_SLOWEST;
    VerticalRainbow.speed_max       = EVISION_KB_SPEED_FASTEST;
    VerticalRainbow.speed           = EVISION_KB_SPEED_NORMAL;
    VerticalRainbow.brightness_min  = EVISION_KB_BRIGHTNESS_LOWEST;
    VerticalRainbow.brightness_max  = EVISION_KB_BRIGHTNESS_HIGHEST;
    VerticalRainbow.brightness      = EVISION_KB_BRIGHTNESS_HIGHEST;
    VerticalRainbow.direction       = MODE_DIRECTION_UP;
    VerticalRainbow.colors_min      = 1;
    VerticalRainbow.colors_max      = 1;
    VerticalRainbow.color_mode      = MODE_COLORS_MODE_SPECIFIC;
    VerticalRainbow.colors.resize(1);
    modes.push_back(VerticalRainbow);

    mode Blooming;
    Blooming.name                   = "Blooming";
    Blooming.value                  = EVISION_KB_MODE_BLOOMING;
    Blooming.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Blooming.speed_min              = EVISION_KB_SPEED_SLOWEST;
    Blooming.speed_max              = EVISION_KB_SPEED_FASTEST;
    Blooming.speed                  = EVISION_KB_SPEED_NORMAL;
    Blooming.brightness_min         = EVISION_KB_BRIGHTNESS_LOWEST;
    Blooming.brightness_max         = EVISION_KB_BRIGHTNESS_HIGHEST;
    Blooming.brightness             = EVISION_KB_BRIGHTNESS_HIGHEST;
    Blooming.color_mode             = MODE_COLORS_RANDOM;
    modes.push_back(Blooming);

    mode Reactive;
    Reactive.name                   = "Reactive";
    Reactive.value                  = EVISION_KB_MODE_REACTIVE;
    Reactive.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Reactive.speed_min              = EVISION_KB_SPEED_SLOWEST;
    Reactive.speed_max              = EVISION_KB_SPEED_FASTEST;
    Reactive.speed                  = EVISION_KB_SPEED_NORMAL;
    Reactive.brightness_min         = EVISION_KB_BRIGHTNESS_LOWEST;
    Reactive.brightness_max         = EVISION_KB_BRIGHTNESS_HIGHEST;
    Reactive.brightness             = EVISION_KB_BRIGHTNESS_HIGHEST;
    Reactive.colors_min             = 1;
    Reactive.colors_max             = 1;
    Reactive.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Reactive.colors.resize(1);
    modes.push_back(Reactive);

    mode ReactiveRipple;
    ReactiveRipple.name             = "Reactive Ripple";
    ReactiveRipple.value            = EVISION_KB_MODE_REACTIVE_RIPPLE;
    ReactiveRipple.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ReactiveRipple.speed_min        = EVISION_KB_SPEED_SLOWEST;
    ReactiveRipple.speed_max        = EVISION_KB_SPEED_FASTEST;
    ReactiveRipple.speed            = EVISION_KB_SPEED_NORMAL;
    ReactiveRipple.brightness_min   = EVISION_KB_BRIGHTNESS_LOWEST;
    ReactiveRipple.brightness_max   = EVISION_KB_BRIGHTNESS_HIGHEST;
    ReactiveRipple.brightness       = EVISION_KB_BRIGHTNESS_HIGHEST;
    ReactiveRipple.colors_min       = 1;
    ReactiveRipple.colors_max       = 1;
    ReactiveRipple.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    ReactiveRipple.colors.resize(1);
    modes.push_back(ReactiveRipple);

    mode ReactiveLine;
    ReactiveLine.name               = "Reactive Line";
    ReactiveLine.value              = EVISION_KB_MODE_REACTIVE_LINE;
    ReactiveLine.flags              = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ReactiveLine.speed_min          = EVISION_KB_SPEED_SLOWEST;
    ReactiveLine.speed_max          = EVISION_KB_SPEED_FASTEST;
    ReactiveLine.speed              = EVISION_KB_SPEED_NORMAL;
    ReactiveLine.brightness_min     = EVISION_KB_BRIGHTNESS_LOWEST;
    ReactiveLine.brightness_max     = EVISION_KB_BRIGHTNESS_HIGHEST;
    ReactiveLine.brightness         = EVISION_KB_BRIGHTNESS_HIGHEST;
    ReactiveLine.colors_min         = 1;
    ReactiveLine.colors_max         = 1;
    ReactiveLine.color_mode         = MODE_COLORS_MODE_SPECIFIC;
    ReactiveLine.colors.resize(1);
    modes.push_back(ReactiveLine);

    if(layout != nullptr)
    {
        /*-------------------------------------------------*\
        | Additional effect exposed by the vendor software  |
        | of keyboards with a model-specific layout         |
        \*-------------------------------------------------*/
        mode StarlightSlow;
        StarlightSlow.name              = "Starlight (Slow)";
        StarlightSlow.value             = EVISION_KB_MODE_STARLIGHT_SLOW;
        StarlightSlow.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
        StarlightSlow.speed_min         = EVISION_KB_SPEED_SLOWEST;
        StarlightSlow.speed_max         = EVISION_KB_SPEED_FASTEST;
        StarlightSlow.speed             = EVISION_KB_SPEED_NORMAL;
        StarlightSlow.brightness_min    = EVISION_KB_BRIGHTNESS_LOWEST;
        StarlightSlow.brightness_max    = EVISION_KB_BRIGHTNESS_HIGHEST;
        StarlightSlow.brightness        = EVISION_KB_BRIGHTNESS_HIGHEST;
        StarlightSlow.colors_min        = 1;
        StarlightSlow.colors_max        = 1;
        StarlightSlow.color_mode        = MODE_COLORS_MODE_SPECIFIC;
        StarlightSlow.colors.resize(1);
        modes.push_back(StarlightSlow);

        for(mode& m : modes)
        {
            if(layout->brightness_max != 0 && (m.flags & MODE_FLAG_HAS_BRIGHTNESS))
            {
                m.brightness_max    = layout->brightness_max;
                m.brightness        = layout->brightness_max;
            }

            /*---------------------------------------------*\
            | Effect colors default to black, which leaves  |
            | the keyboard dark when an effect is selected. |
            | Start from red, like the vendor defaults.     |
            \*---------------------------------------------*/
            if(m.color_mode == MODE_COLORS_MODE_SPECIFIC && m.colors.size() > 0)
            {
                m.colors[0]         = ToRGBColor(0xFF, 0x00, 0x00);
            }
        }
    }

    SetupZones();
}

RGBController_EVisionKeyboard::~RGBController_EVisionKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_EVisionKeyboard::SetupZones()
{
    zone new_zone;

    if(layout != nullptr)
    {
        /*-------------------------------------------------*\
        | Model-specific layout: one LED per physical key,  |
        | led.value holds the index into the color buffer   |
        \*-------------------------------------------------*/
        std::vector<unsigned int> map(layout->rows * layout->cols, 0xFFFFFFFF);

        for(unsigned int key_idx = 0; key_idx < layout->key_count; key_idx++)
        {
            const evision_keyboard_key& key = layout->keys[key_idx];

            map[(key.row * layout->cols) + key.col] = key_idx;

            led new_led;

            new_led.name    = key.name;
            new_led.value   = key.index;

            leds.push_back(new_led);
        }

        new_zone.name               = "Keyboard";
        new_zone.type               = ZONE_TYPE_MATRIX;
        new_zone.leds_min           = layout->key_count;
        new_zone.leds_max           = layout->key_count;
        new_zone.leds_count         = layout->key_count;
        new_zone.matrix_map.Set(layout->rows, layout->cols, map.data());

        zones.push_back(new_zone);

        SetupColors();
        return;
    }

    new_zone.name               = "Keyboard";
    new_zone.type               = ZONE_TYPE_MATRIX;
    new_zone.leds_min           = 126;
    new_zone.leds_max           = 126;
    new_zone.leds_count         = 126;
    new_zone.matrix_map.Set(6, 23, (unsigned int *)&matrix_map);

    zones.push_back(new_zone);

    for(int led_idx = 0; led_idx < 126; led_idx++)
    {
        led new_led;

        new_led.name = "Keyboard LED ";
        new_led.name.append(std::to_string(led_idx));

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_EVisionKeyboard::DeviceUpdateLEDs()
{
    unsigned char color_data[7*0x36];

    if(layout != nullptr)
    {
        memset(color_data, 0x00, sizeof(color_data));

        for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
        {
            unsigned int offset = 3 * leds[led_idx].value;

            if(offset + 2 < sizeof(color_data))
            {
                color_data[offset + 0] = RGBGetRValue(colors[led_idx]);
                color_data[offset + 1] = RGBGetGValue(colors[led_idx]);
                color_data[offset + 2] = RGBGetBValue(colors[led_idx]);
            }
        }
    }
    else
    {
        for(int led_idx = 0; led_idx < 126; led_idx++)
        {
            color_data[(3 * led_idx) + 0] = RGBGetRValue(colors[led_idx]);
            color_data[(3 * led_idx) + 1] = RGBGetGValue(colors[led_idx]);
            color_data[(3 * led_idx) + 2] = RGBGetBValue(colors[led_idx]);
        }
    }

    /*-----------------------------------------------------*\
    | Per-key colors are only shown in Custom mode. The     |
    | keyboard may still be running an onboard effect, so   |
    | send the mode once before the first color update.     |
    \*-----------------------------------------------------*/
    if(layout != nullptr && !mode_sent)
    {
        DeviceUpdateMode();
    }

    if(layout != nullptr && layout->use_begin_end)
    {
        controller->SendKeyboardBegin();
    }

    controller->SetKeyboardColors
                    (
                    color_data,
                    0x36 * 7
                    );

    if(layout != nullptr && layout->use_begin_end)
    {
        controller->SendKeyboardEnd();
    }
}

void RGBController_EVisionKeyboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EVisionKeyboard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EVisionKeyboard::DeviceUpdateMode()
{
    unsigned char red    = 0x00;
    unsigned char grn    = 0x00;
    unsigned char blu    = 0x00;
    unsigned char random = (modes[active_mode].color_mode == MODE_COLORS_RANDOM);

    if(modes[active_mode].colors.size() > 0)
    {
        red = RGBGetRValue(modes[active_mode].colors[0]);
        grn = RGBGetGValue(modes[active_mode].colors[0]);
        blu = RGBGetBValue(modes[active_mode].colors[0]);
    }

    if(layout != nullptr && layout->use_begin_end)
    {
        controller->SendKeyboardBegin();
    }

    controller->SendKeyboardModeEx
            (
            modes[active_mode].value,
            modes[active_mode].brightness,
            modes[active_mode].speed,
            modes[active_mode].direction,
            random,
            red,
            grn,
            blu
            );

    if(layout != nullptr && layout->use_begin_end)
    {
        controller->SendKeyboardEnd();
    }

    mode_sent = true;
}

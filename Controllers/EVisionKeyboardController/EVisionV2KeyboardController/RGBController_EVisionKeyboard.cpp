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

using namespace std::chrono_literals;

//0xFFFFFFFF indicates an unused entry in matrix
#define NA  0xFFFFFFFF

static unsigned int matrix_map[6][23] =
    { {   0,  NA,  1,    2,   3,   4,  NA,   5,   6,   7,   8,  NA,   9,  10,  11,  12,  14,  15,  16,  NA,  NA,  NA,  NA },
      {  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  NA,  32,  33,  34,  NA,  35,  36,  37,  38,  39,  40,  41 },
      {  42,  NA,  43,  44,  45,  46,  NA,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62 },
      {  63,  NA,  64,  65,  66,  67,  NA,  68,  69,  70,  71,  72,  73,  74,  76,  NA,  NA,  NA,  NA,  80,  81,  82,  NA },
      {  84,  NA,  86,  87,  88,  89,  NA,  90,  NA,  91,  92,  93,  94,  95,  97,  NA,  NA,  99,  NA, 101, 102, 103, 104 },
      { 105, 106, 107,  NA,  NA,  NA,  NA, 108,  NA,  NA,  NA,  NA, 109, 110, 111, 113, 119, 120, 121, 123,  NA, 124,  NA } };

/*---------------------------------------------------------*\
| K551RGB-1 V2 firmware 0105 has 128 four-byte LED records  |
| in XRAM at 0x066B..0x086A.  HID command 0x12 accepts a    |
| packed RGB stream and expands each triplet into a record. |
| Therefore the exact safe limit is 128 RGB triplets.       |
|                                                           |
| The firmware table at code address 0x30A7 maps these raw  |
| records to the persistent/layout indices used below.  Its |
| three otherwise unrepresented positions are the second   |
| emitters for Enter and both Shift keys.  Raw slot 126 is  |
| NumPad Enter.  Raw slot 127 has no physical LED.           |
|                                                           |
| Sending 133 triplets in the old probe overflowed past      |
| 0x086A into runtime state and caused stuck/repeated keys.  |
| The eighth HID packet is safe when it contains only the    |
| final six bytes for slots 126 and 127.                     |
\*---------------------------------------------------------*/
static unsigned int k551_matrix_map[6][23] =
    { {   0,  NA,   1,   2,   3,   4,  NA,   5,   6,   7,   8,  NA,   9,  10,  11,  12,  13,  14,  15,  NA,  NA,  NA,  NA },
      {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  NA,  27,  28,  29,  NA,  30,  31,  32,  33,  34,  35,  36 },
      {  37,  NA,  38,  39,  40,  41,  NA,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57 },
      {  58,  NA,  59,  60,  61,  62,  NA,  63,  64,  65,  66,  67,  68,  69,  70,  NA,  NA,  NA,  NA,  71,  72,  73,  NA },
      {  74,  NA,  75,  76,  77,  78,  NA,  79,  NA,  80,  81,  82,  83,  84,  85,  NA,  NA,  86,  NA,  87,  88,  89,  90 },
      {  91,  92,  93,  NA,  NA,  NA,  NA,  94,  NA,  NA,  NA,  NA,  95,  96,  97,  98,  99, 100, 101, 102,  NA, 103,  NA } };

static const unsigned int k551_logical_led_count       = 104;
static const unsigned int k551_framebuffer_led_count   = 128;
static const unsigned char k551_unmapped_led           = 0xFF;

/*---------------------------------------------------------*\
| Raw 0x12 framebuffer slot -> OpenRGB logical LED.          |
| Repeated logical indices intentionally drive the two LED   |
| emitters under Enter, Left Shift, and Right Shift.         |
\*---------------------------------------------------------*/
static const unsigned char k551_raw_to_logical_led[k551_framebuffer_led_count] =
    {
          0,  16,  37,  58,  74,  91,  33, k551_unmapped_led,   1,  17,  38,  59,  74,  92,  34, k551_unmapped_led,
          2,  18,  39,  60,  75,  93,  35, k551_unmapped_led,   3,  19,  40,  61,  76, k551_unmapped_led,  36, k551_unmapped_led,
          4,  20,  41,  62,  77, k551_unmapped_led,  54, k551_unmapped_led,   5,  21,  42,  63,  78,  94,  55, k551_unmapped_led,
          6,  22,  43,  64,  79, k551_unmapped_led,  56, k551_unmapped_led,   7,  23,  44,  65,  80, k551_unmapped_led,  71, k551_unmapped_led,
          8,  24,  45,  66,  81, k551_unmapped_led,  72, k551_unmapped_led,   9,  25,  46,  67,  82,  95,  73, k551_unmapped_led,
         10,  26,  47,  68,  83,  96,  57, k551_unmapped_led,  11,  27,  48,  69,  84,  97,  88, k551_unmapped_led,
         12,  28,  49,  70,  85,  98,  89, k551_unmapped_led,  13,  29,  50,  70,  85,  99, 102, k551_unmapped_led,
         14,  30,  51,  32,  86, 100, 103, k551_unmapped_led,  15,  31,  52,  53,  87, 101,  90, k551_unmapped_led
    };

static const char* k551_led_names[k551_logical_led_count] =
    {
        "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Print Screen", "Scroll Lock", "Pause",
        "`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Backspace", "Insert", "Home", "Page Up", "Num Lock", "Num /", "Num *", "Num -",
        "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\", "Delete", "End", "Page Down", "Num 7", "Num 8", "Num 9", "Num +",
        "Caps Lock", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter", "Num 4", "Num 5", "Num 6",
        "Left Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "Right Shift", "Up", "Num 1", "Num 2", "Num 3", "Num Enter",
        "Left Ctrl", "Left Win", "Left Alt", "Space", "Right Alt", "Fn", "Menu", "Right Ctrl", "Left", "Down", "Right", "Num 0", "Num ."
    };

/**------------------------------------------------------------------*\
    @name EVision Keyboard
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectEVisionKeyboards
    @comment
\*-------------------------------------------------------------------*/

RGBController_EVisionKeyboard::RGBController_EVisionKeyboard(EVisionKeyboardController* controller_ptr)
{
    controller                      = controller_ptr;
    direct_active                   = false;
    keepalive_thread_run            = false;
    keepalive_thread                = nullptr;
    last_update_time                = std::chrono::steady_clock::now();

    name                            = controller->GetNameString();
    vendor                          = "EVision";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "EVision Keyboard Device";
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();

    if(controller->SupportsDirect())
    {
        name                        = "Redragon K551RGB-1 V2";
        vendor                      = "Redragon";
        description                 = "Redragon K551RGB-1 V2 Keyboard";

        mode Direct;
        Direct.name                 = "Direct";
        Direct.value                = EVISION_KB_MODE_DIRECT;
        Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
        Direct.color_mode           = MODE_COLORS_PER_LED;
        modes.push_back(Direct);

        /*-------------------------------------------------*
        | This firmware revision uses the RAM-only 0x12     |
        | framebuffer for software effects.  The legacy     |
        | EVision hardware-mode commands target a different |
        | keyboard protocol and are deliberately not shown. |
        \*-------------------------------------------------*/
        SetupZones();

        keepalive_thread_run = true;
        keepalive_thread = new std::thread(&RGBController_EVisionKeyboard::KeepaliveThreadFunction, this);
        return;
    }

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

    SetupZones();

}

RGBController_EVisionKeyboard::~RGBController_EVisionKeyboard()
{
    Shutdown();

    if(keepalive_thread != nullptr)
    {
        keepalive_thread_run = false;
        keepalive_thread->join();
        delete keepalive_thread;
    }

    if(direct_active.exchange(false))
    {
        controller->SendKeyboardDirectEnd();
    }

    delete controller;
}

void RGBController_EVisionKeyboard::SetupZones()
{
    zone new_zone;

    new_zone.name               = "Keyboard";
    new_zone.type               = ZONE_TYPE_MATRIX;
    new_zone.leds_min           = controller->SupportsDirect() ? k551_logical_led_count : 126;
    new_zone.leds_max           = controller->SupportsDirect() ? k551_logical_led_count : 126;
    new_zone.leds_count         = controller->SupportsDirect() ? k551_logical_led_count : 126;
    new_zone.matrix_map.Set(6, 23, controller->SupportsDirect() ? (unsigned int *)&k551_matrix_map : (unsigned int *)&matrix_map);

    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;

        if(controller->SupportsDirect())
        {
            new_led.name = k551_led_names[led_idx];
        }
        else
        {
            new_led.name = "Keyboard LED ";
            new_led.name.append(std::to_string(led_idx));
        }

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_EVisionKeyboard::DeviceUpdateLEDs()
{
    std::lock_guard<std::mutex> update_lock(direct_update_mutex);

    unsigned char color_data[k551_framebuffer_led_count * 3] = { 0 };
    unsigned int  color_data_size = 126 * 3;

    last_update_time = std::chrono::steady_clock::now();

    if(controller->SupportsDirect())
    {
        color_data_size = sizeof(color_data);

        /*-------------------------------------------------*
        | Scatter the 104 OpenRGB keys through the exact    |
        | firmware mapping.  This also mirrors the colors   |
        | onto the secondary Enter/Shift emitters and maps  |
        | NumPad Enter to raw slot 126.                     |
        \*-------------------------------------------------*/
        for(unsigned int framebuffer_slot = 0; framebuffer_slot < k551_framebuffer_led_count; framebuffer_slot++)
        {
            unsigned int led_idx = k551_raw_to_logical_led[framebuffer_slot];

            if(led_idx >= k551_logical_led_count)
            {
                continue;
            }

            color_data[(3 * framebuffer_slot) + 0] = RGBGetRValue(colors[led_idx]);
            color_data[(3 * framebuffer_slot) + 1] = RGBGetGValue(colors[led_idx]);
            color_data[(3 * framebuffer_slot) + 2] = RGBGetBValue(colors[led_idx]);
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

    if(modes[active_mode].value == EVISION_KB_MODE_DIRECT)
    {
        controller->SetKeyboardColorsDirect
                        (
                        color_data,
                        color_data_size
                        );
        direct_active.store(true);
    }
    else
    {
        controller->SetKeyboardColors
                        (
                        color_data,
                        color_data_size
                        );
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
    if(modes[active_mode].value == EVISION_KB_MODE_DIRECT)
    {
        return;
    }

    if(direct_active.exchange(false))
    {
        controller->SendKeyboardDirectEnd();
    }

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
}

void RGBController_EVisionKeyboard::KeepaliveThreadFunction()
{
    while(keepalive_thread_run.load())
    {
        if(direct_active.load())
        {
            bool update_required = false;

            {
                std::lock_guard<std::mutex> update_lock(direct_update_mutex);
                update_required = (std::chrono::steady_clock::now() - last_update_time) > 100ms;
            }

            if(update_required)
            {
                UpdateLEDsInternal();
            }
        }

        std::this_thread::sleep_for(5ms);
    }
}

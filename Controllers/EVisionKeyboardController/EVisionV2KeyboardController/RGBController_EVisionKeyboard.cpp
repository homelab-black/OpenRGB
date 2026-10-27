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

typedef struct
{
    const unsigned int width;
    const unsigned int height;

    std::vector<std::vector<unsigned int>> matrix_map;

    std::vector<std::string> led_names;

    std::vector<unsigned int> led_sequence_positions;
} evision_keyboard;

#define NA  0xFFFFFFFF
// Tecware Phantom 104 logical keys
static evision_keyboard phantom104 =
{
    25,
    6,

    {
        { 
            0, NA, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, NA, 13, 14, 15, NA, NA, NA, NA, NA, NA, NA
        },
        {   
            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, NA, 30, 31, 32, NA, 33, 34, 35, 36, NA, NA
        },
        {   
            37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, NA, 51, 52, 53, NA, 54, 55, 56, 57, NA, NA
        },
        {   
            58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, NA, NA, NA, NA, NA, NA, 71, 72, 73, NA, NA, NA
        },
        {   
            74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, NA, NA, NA, 86, NA, NA, NA, 87, 88, 89, 90, NA, NA,
        },
        {
            91, 92, 93, NA, NA, 94, NA, NA, 95, 96, 97, 98, NA, NA, 99, 100, 101, NA, NA, 102, 103, NA, NA, NA, NA
        },
    },
{
    "Esc",          //0

    "F1",           //1
    "F2",           //2
    "F3",           //3
    "F4",           //4

    "F5",           //5
    "F6",           //6
    "F7",           //7
    "F8",           //8

    "F9",           //9
    "F10",          //10
    "F11",          //11
    "F12",          //12

    "PrtSc",        //13
    "Scroll Lock",  //14
    "Pause",        //15

    "`",            //16

    "1",            //17
    "2",            //18
    "3",            //19
    "4",            //20
    "5",            //21
    "6",            //22
    "7",            //23
    "8",            //24
    "9",            //25
    "0",            //26

    "-",            //27
    "=",            //28
    "Backspace",    //29
    "Insert",       //30
    "Home",         //31
    "Page Up",      //32
    "Num Lock",     //33
    "Num /",        //34
    "Num *",        //35
    "Num -",        //36

    "Tab",          //37

    "Q",            //38
    "W",            //39
    "E",            //40
    "R",            //41
    "T",            //42
    "Y",            //43
    "U",            //44
    "I",            //45
    "O",            //46
    "P",            //47
    "[{",           //48
    "}]",           //49    
    "\\",           //50
    
    "Delete",       //51
    "End",          //52
    "Page Down",    //53
    
    "Num 7",        //54
    "Num 8",        //55
    "Num 9",        //56
    "Num +",        //57

    "Caps Lock",    //58
    "A",            //59
    "S",            //60
    "D",            //61
    "F",            //62
    "G",            //63
    "H",            //64
    "J",            //65
    "K",            //66
    "L",            //67
    ";:",           //68
    "'",            //69
    "Enter",        //70

    "Num 4",        //71
    "Num 5",        //72
    "Num 6",        //73

    "Left Shift",   //74
    "Z",            //75
    "X",            //76
    "C",            //77
    "V",            //78
    "B",            //79
    "N",            //80
    "M",            //81
    ",<",           //82
    ".>",           //83
    "//?",          //84
    "Right Shift",  //85
    
    "Up Arrow",     //86

    "Num 1",        //87
    "Num 2",        //88
    "Num 3",        //89
    "Num Enter",    //90

    "Left Control", //91
    "Windows Key",  //92
    "Left Alt",     //93
    "Space",        //94
    "Menu",         //95
    "Fn",           //96
    "Right Alt",    //97
    "Right Control",//98

    "Left Arrow",   //99
    "Down Arrow",   //100
    "Right Arrow",  //101

    "Num 0",        //102
    "Num ."         //103
},
    {
        1, //KEY_EN_ESCAPE,              //0
        2, //KEY_EN_F1,                  //1
        3, //KEY_EN_F2,                  //2
        4, //KEY_EN_F3,                  //3
        5, //KEY_EN_F4,                  //4
        6, //KEY_EN_F5,                  //5
        7, //KEY_EN_F6,                  //6
        8, //KEY_EN_F7,                  //7
        9, //KEY_EN_F8,                  //8
        10, //KEY_EN_F9,                 //9
        11, //KEY_EN_F10,                 //10
        12, //KEY_EN_F11,                 //11
        13, //KEY_EN_F12,                 //12
        106,//KEY_EN_PRINT_SCREEN,        //13
        107,//KEY_EN_SCROLL_LOCK,         //14
        108,//KEY_EN_PAUSE,               //15
        18, //KEY_EN_BACK_TICK,           //16
        19, //KEY_EN_1,                   //17
        20, //KEY_EN_2,                   //18
        21, //KEY_EN_3,                   //19
        22, //KEY_EN_4,                   //20
        23, //KEY_EN_5,                   //21
        24, //KEY_EN_6,                   //22
        25, //KEY_EN_7,                   //23
        26, //KEY_EN_8,                   //24
        27, //KEY_EN_9,                   //25
        28, //KEY_EN_0,                   //26
        29, //KEY_EN_MINUS,               //27
        30, //KEY_EN_EQUAL,               //28
        98, //KEY_EN_BACKSPACE,           //29
        110,//KEY_EN_INSERT,              //30
        111,//KEY_EN_HOME,                //31
        112,//KEY_EN_PAGE_UP,             //32
        31, //KEY_EN_NUMLOCK,             //33
        32, //KEY_EN_NUMPAD_SLASH,        //34
        33, //KEY_EN_NUMPAD_ASTERISK,     //35
        116,//KEY_EN_NUMPAD_MINUS,        //36
        35, //KEY_EN_TAB,                 //37
        36, //KEY_EN_Q,                   //38
        37, //KEY_EN_W,                   //39
        38, //KEY_EN_E,                   //40
        39, //KEY_EN_R,                   //41
        40, //KEY_EN_T,                   //42
        41, //KEY_EN_Y,                   //43
        42, //KEY_EN_U,                   //44
        43, //KEY_EN_I,                   //45
        44, //KEY_EN_O,                   //46
        45, //KEY_EN_P,                   //47
        46, //KEY_EN_LEFT_BRACKET,        //48
        47, //KEY_EN_RIGHT_BRACKET,       //49
        64, //KEY_ANSI_BACK_SLASH,        //50
        113,//KEY_EN_DELETE,              //51
        114,//KEY_EN_END,                 //52
        115,//KEY_EN_PAGE_DOWN,           //53
        48, //KEY_EN_NUMPAD_7,            //54
        49, //KEY_EN_NUMPAD_8,            //55
        50, //KEY_EN_NUMPAD_9,            //56
        52, //KEY_EN_CAPS,                //57
        53, //KEY_EN_A,                   //58
        54, //KEY_EN_S,                   //59
        55, //KEY_EN_D,                   //60
        56, //KEY_EN_F,                   //61
        57, //KEY_EN_G,                   //62
        58, //KEY_EN_H,                   //63
        59, //KEY_EN_J,                   //64
        60, //KEY_EN_K,                   //65
        61, //KEY_EN_L,                   //66
        62, //KEY_EN_SEMI_COLON,          //67
        63, //KEY_EN_QUOTE,               //68
        81, //KEY_EN_ANSI_ENTER,          //69
        65, //KEY_EN_NUMPAD_4,            //70
        66, //KEY_EN_NUMPAD_5,            //71
        67, //KEY_EN_NUMPAD_6,            //72
        117,//KEY_EN_NUMPAD_PLUS,         //73
        69, //KEY_ANSI_LEFT_SHIFT,        //74
        70, //KEY_EN_Z,                   //75
        71, //KEY_EN_X,                   //76
        72, //KEY_EN_C,                   //77
        73, //KEY_EN_V,                   //78
        74, //KEY_EN_B,                   //79
        75, //KEY_EN_N,                   //80
        76, //KEY_EN_M,                   //81
        77, //KEY_EN_COMMA,               //82
        78, //KEY_EN_PERIOD,              //83
        79, //KEY_EN_FORWARD_SLASH,       //84
        80, //KEY_EN_RIGHT_SHIFT,         //85
        96, //KEY_EN_UP_ARROW,            //86
        82, //KEY_EN_NUMPAD_1,            //87
        83, //KEY_EN_NUMPAD_2,            //88
        84, //KEY_EN_NUMPAD_3,            //89
        101,//KEY_EN_NUMPAD_ENTER,        //90
        86, //KEY_EN_LEFT_CONTROL,        //91
        87, //KEY_EN_LEFT_WINDOWS,        //92
        88, //KEY_EN_LEFT_ALT,            //93
        89, //KEY_EN_SPACE,               //94
        90, //KEY_EN_APPS,                //95
        91, //KEY_EN_FN,                  //96
        92, //KEY_EN_RIGHT_ALT,           //97
        93, //KEY_EN_RIGHT_CONTROL,       //98
        94, //KEY_EN_LEFT_ARROW,          //99
        95, //KEY_EN_DOWN_ARROW,          //100
        97, //KEY_EN_RIGHT_ARROW,         //101
        99, //KEY_EN_NUMPAD_0,            //102
        100,//KEY_EN_NUMPAD_PERIOD,       //103
    }
};

/**------------------------------------------------------------------*\
    @name EVision Keyboard
    @category Keyboard
    @type USB
    @save :robot:m
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectEVisionKeyboards
    @comment
\*-------------------------------------------------------------------*/

RGBController_EVisionKeyboard::RGBController_EVisionKeyboard(EVisionKeyboardController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "EVision";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "EVision Keyboard Device";
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

    SetupZones();
}

RGBController_EVisionKeyboard::~RGBController_EVisionKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_EVisionKeyboard::SetupZones()
{
    std::vector<unsigned int> flat_matrix;

for(unsigned int row_idx = 0;
    row_idx < phantom104.matrix_map.size();
    row_idx++)
{
    flat_matrix.insert(
        flat_matrix.end(),
        phantom104.matrix_map[row_idx].begin(),
        phantom104.matrix_map[row_idx].end()
    );
}

    
    zone new_zone;

    new_zone.name       = "Keyboard";
    new_zone.type       = ZONE_TYPE_MATRIX;

    new_zone.leds_min   = (unsigned int)phantom104.led_names.size();
    new_zone.leds_max   = (unsigned int)phantom104.led_names.size();
    new_zone.leds_count = (unsigned int)phantom104.led_names.size();

    new_zone.matrix_map.Set(
        phantom104.height,
        phantom104.width,
        flat_matrix.data()
    );

    zones.push_back(new_zone);

    for(size_t led_idx = 0; led_idx < phantom104.led_names.size(); led_idx++)
    {
        led new_led;
        new_led.name = phantom104.led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_EVisionKeyboard::DeviceUpdateLEDs()
{
    unsigned char color_data[7 * 0x36] = { 0 };

    for(size_t logical_idx = 0;
        logical_idx < phantom104.led_sequence_positions.size();
        logical_idx++)
    {
        unsigned int hw_led = phantom104.led_sequence_positions[logical_idx];

        if(hw_led == 0)
        {
            continue;
        }

        color_data[(3 * hw_led) + 0] =
            RGBGetRValue(colors[logical_idx]);

        color_data[(3 * hw_led) + 1] =
            RGBGetGValue(colors[logical_idx]);

        color_data[(3 * hw_led) + 2] =
            RGBGetBValue(colors[logical_idx]);
    }

    controller->SetKeyboardColors(
        color_data,
        0x36 * 7
    );
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

/*---------------------------------------------------------*\
| RGBController_FnaticStreak.cpp                            |
|                                                           |
|   RGBController for Fnatic Streak keyboard                |
|                                                           |
|   Based on leddy by Hanna Czenczek                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <vector>
#include "RGBControllerKeyNames.h"
#include "RGBController_FnaticStreak.h"

//0xFFFFFFFF indicates an unused entry in matrix
#define NA  0xFFFFFFFF

/*-----------------------------------------------------*\
| Full size Fnatic Streak matrix map (6 rows x 22 cols)|
\*-----------------------------------------------------*/
static unsigned int matrix_map_streak[6][22] =
{
    {   1,   NA,    7,   13,   19,   25,   31,   37,   43,   49,  120,   55,   67,   73,   79,   90,   93,   98,   91,   97,   92,  118 },
    {   2,    8,   14,   20,   26,   32,   38,   44,   50,   56,   61,   62,   68,   74,   80,   89,   94,   99,  100,  108,  109,  116 },
    {   3,    9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,   75,   NA,   81,   88,   95,   96,  101,  107,  110,  115 },
    {   4,   NA,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,   76,   82,   NA,   NA,   NA,  102,  106,  111,   NA },
    {   5,   11,   17,   23,   29,   35,   41,   47,   53,   59,   65,   66,   71,   77,   NA,   NA,   87,   NA,  103,  105,  112,  114 },
    {   6,   12,   NA,   18,   24,   30,   36,   42,   48,   54,   60,   72,   NA,   78,   83,   84,   85,   86,  104,   NA,  113,   NA }
};

/*-----------------------------------------------------*\
| miniStreak matrix map (6 rows x 18 cols)             |
\*-----------------------------------------------------*/
static unsigned int matrix_map_ministreak[6][18] =
{
    {   1,   NA,    7,   13,   19,   25,   31,   37,   43,   49,  103,   55,   67,   73,   79,   90,   93,   98 },
    {   2,    8,   14,   20,   26,   32,   38,   44,   50,   56,   61,   62,   68,   74,   80,   89,   94,   99 },
    {   3,    9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,   75,   NA,   81,   88,   95,   96 },
    {   4,   NA,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,   76,   82,   NA,   NA,   NA },
    {   5,   11,   17,   23,   29,   35,   41,   47,   53,   59,   65,   66,   71,   77,   NA,   NA,   87,   NA },
    {   6,   12,   NA,   18,   24,   30,   36,   42,   48,   54,   60,   72,   NA,   78,   83,   84,   85,   86 }
};

static const char* zone_names[] =
{
    ZONE_EN_KEYBOARD
};

static zone_type zone_types[] =
{
    ZONE_TYPE_MATRIX,
};

/*-----------------------------------------------------*\
| LED names for full Streak (124 LEDs)                 |
\*-----------------------------------------------------*/
static const char *led_names_streak[] =
{
    KEY_EN_UNUSED,                  // 0 - FN Lock indicator
    KEY_EN_ESCAPE,                  // 1
    KEY_EN_BACK_TICK,               // 2
    KEY_EN_TAB,                     // 3
    KEY_EN_CAPS_LOCK,               // 4
    KEY_EN_LEFT_SHIFT,              // 5
    KEY_EN_LEFT_CONTROL,            // 6
    KEY_EN_F1,                      // 7
    KEY_EN_1,                       // 8
    KEY_EN_Q,                       // 9
    KEY_EN_A,                       // 10
    KEY_EN_ISO_BACK_SLASH,          // 11
    KEY_EN_LEFT_WINDOWS,            // 12
    KEY_EN_F2,                      // 13
    KEY_EN_2,                       // 14
    KEY_EN_W,                       // 15
    KEY_EN_S,                       // 16
    KEY_EN_Z,                       // 17
    KEY_EN_LEFT_ALT,                // 18
    KEY_EN_F3,                      // 19
    KEY_EN_3,                       // 20
    KEY_EN_E,                       // 21
    KEY_EN_D,                       // 22
    KEY_EN_X,                       // 23
    KEY_EN_UNUSED,                  // 24
    KEY_EN_F4,                      // 25
    KEY_EN_4,                       // 26
    KEY_EN_R,                       // 27
    KEY_EN_F,                       // 28
    KEY_EN_C,                       // 29
    KEY_EN_UNUSED,                  // 30
    KEY_EN_F5,                      // 31
    KEY_EN_5,                       // 32
    KEY_EN_T,                       // 33
    KEY_EN_G,                       // 34
    KEY_EN_V,                       // 35
    KEY_EN_SPACE,                   // 36
    KEY_EN_F6,                      // 37
    KEY_EN_6,                       // 38
    KEY_EN_Y,                       // 39
    KEY_EN_H,                       // 40
    KEY_EN_B,                       // 41
    KEY_EN_UNUSED,                  // 42
    KEY_EN_F7,                      // 43
    KEY_EN_7,                       // 44
    KEY_EN_U,                       // 45
    KEY_EN_J,                       // 46
    KEY_EN_N,                       // 47
    KEY_EN_UNUSED,                  // 48
    KEY_EN_F8,                      // 49
    KEY_EN_8,                       // 50
    KEY_EN_I,                       // 51
    KEY_EN_K,                       // 52
    KEY_EN_M,                       // 53
    KEY_EN_UNUSED,                  // 54
    KEY_EN_F9,                      // 55
    KEY_EN_9,                       // 56
    KEY_EN_O,                       // 57
    KEY_EN_L,                       // 58
    KEY_EN_COMMA,                   // 59
    KEY_EN_RIGHT_ALT,               // 60
    KEY_EN_0,                       // 61
    KEY_EN_MINUS,                   // 62
    KEY_EN_P,                       // 63
    KEY_EN_SEMICOLON,               // 64
    KEY_EN_PERIOD,                  // 65
    KEY_EN_FORWARD_SLASH,           // 66
    KEY_EN_F10,                     // 67
    KEY_EN_EQUALS,                  // 68
    KEY_EN_LEFT_BRACKET,            // 69
    KEY_EN_QUOTE,                   // 70
    KEY_EN_UNUSED,                  // 71
    KEY_EN_RIGHT_FUNCTION,          // 72
    KEY_EN_F11,                     // 73
    KEY_EN_UNUSED,                  // 74
    KEY_EN_RIGHT_BRACKET,           // 75
    KEY_EN_POUND,                   // 76
    KEY_EN_RIGHT_SHIFT,             // 77
    KEY_EN_MENU,                    // 78
    KEY_EN_F12,                     // 79
    KEY_EN_BACKSPACE,               // 80
    KEY_EN_ANSI_BACK_SLASH,         // 81
    KEY_EN_ANSI_ENTER,              // 82
    KEY_EN_RIGHT_CONTROL,           // 83
    KEY_EN_LEFT_ARROW,              // 84
    KEY_EN_DOWN_ARROW,              // 85
    KEY_EN_RIGHT_ARROW,             // 86
    KEY_EN_UP_ARROW,                // 87
    KEY_EN_DELETE,                  // 88
    KEY_EN_INSERT,                  // 89
    KEY_EN_PRINT_SCREEN,            // 90
    "Key: Mute Mic",                // 91
    "Key: Mute Speaker",            // 92
    KEY_EN_SCROLL_LOCK,             // 93
    KEY_EN_HOME,                    // 94
    KEY_EN_END,                     // 95
    KEY_EN_PAGE_DOWN,               // 96
    "Key: Gaming Mode",             // 97
    KEY_EN_PAUSE_BREAK,             // 98
    KEY_EN_PAGE_UP,                 // 99
    KEY_EN_NUMPAD_LOCK,             // 100
    KEY_EN_NUMPAD_7,                // 101
    KEY_EN_NUMPAD_4,                // 102
    KEY_EN_NUMPAD_1,                // 103
    KEY_EN_NUMPAD_0,                // 104
    KEY_EN_NUMPAD_2,                // 105
    KEY_EN_NUMPAD_5,                // 106
    KEY_EN_NUMPAD_8,                // 107
    KEY_EN_NUMPAD_DIVIDE,           // 108
    KEY_EN_NUMPAD_TIMES,            // 109
    KEY_EN_NUMPAD_9,                // 110
    KEY_EN_NUMPAD_6,                // 111
    KEY_EN_NUMPAD_3,                // 112
    KEY_EN_NUMPAD_PERIOD,           // 113
    KEY_EN_NUMPAD_ENTER,            // 114
    KEY_EN_NUMPAD_PLUS,             // 115
    KEY_EN_NUMPAD_MINUS,            // 116
    KEY_EN_UNUSED,                  // 117
    "Key: Volume Knob",             // 118
    KEY_EN_UNUSED,                  // 119
    "LED: Logo",                    // 120
    KEY_EN_UNUSED,                  // 121
    KEY_EN_UNUSED,                  // 122
    KEY_EN_UNUSED,                  // 123
};

/*-----------------------------------------------------*\
| LED names for miniStreak (106 LEDs)                  |
\*-----------------------------------------------------*/
static const char *led_names_ministreak[] =
{
    KEY_EN_UNUSED,                  // 0 - FN Lock indicator
    KEY_EN_ESCAPE,                  // 1
    KEY_EN_BACK_TICK,               // 2
    KEY_EN_TAB,                     // 3
    KEY_EN_CAPS_LOCK,               // 4
    KEY_EN_LEFT_SHIFT,              // 5
    KEY_EN_LEFT_CONTROL,            // 6
    KEY_EN_F1,                      // 7
    KEY_EN_1,                       // 8
    KEY_EN_Q,                       // 9
    KEY_EN_A,                       // 10
    KEY_EN_ISO_BACK_SLASH,          // 11
    KEY_EN_LEFT_WINDOWS,            // 12
    KEY_EN_F2,                      // 13
    KEY_EN_2,                       // 14
    KEY_EN_W,                       // 15
    KEY_EN_S,                       // 16
    KEY_EN_Z,                       // 17
    KEY_EN_LEFT_ALT,                // 18
    KEY_EN_F3,                      // 19
    KEY_EN_3,                       // 20
    KEY_EN_E,                       // 21
    KEY_EN_D,                       // 22
    KEY_EN_X,                       // 23
    KEY_EN_UNUSED,                  // 24
    KEY_EN_F4,                      // 25
    KEY_EN_4,                       // 26
    KEY_EN_R,                       // 27
    KEY_EN_F,                       // 28
    KEY_EN_C,                       // 29
    KEY_EN_UNUSED,                  // 30
    KEY_EN_F5,                      // 31
    KEY_EN_5,                       // 32
    KEY_EN_T,                       // 33
    KEY_EN_G,                       // 34
    KEY_EN_V,                       // 35
    KEY_EN_SPACE,                   // 36
    KEY_EN_F6,                      // 37
    KEY_EN_6,                       // 38
    KEY_EN_Y,                       // 39
    KEY_EN_H,                       // 40
    KEY_EN_B,                       // 41
    KEY_EN_UNUSED,                  // 42
    KEY_EN_F7,                      // 43
    KEY_EN_7,                       // 44
    KEY_EN_U,                       // 45
    KEY_EN_J,                       // 46
    KEY_EN_N,                       // 47
    KEY_EN_UNUSED,                  // 48
    KEY_EN_F8,                      // 49
    KEY_EN_8,                       // 50
    KEY_EN_I,                       // 51
    KEY_EN_K,                       // 52
    KEY_EN_M,                       // 53
    KEY_EN_UNUSED,                  // 54
    KEY_EN_F9,                      // 55
    KEY_EN_9,                       // 56
    KEY_EN_O,                       // 57
    KEY_EN_L,                       // 58
    KEY_EN_COMMA,                   // 59
    KEY_EN_RIGHT_ALT,               // 60
    KEY_EN_0,                       // 61
    KEY_EN_MINUS,                   // 62
    KEY_EN_P,                       // 63
    KEY_EN_SEMICOLON,               // 64
    KEY_EN_PERIOD,                  // 65
    KEY_EN_FORWARD_SLASH,           // 66
    KEY_EN_F10,                     // 67
    KEY_EN_EQUALS,                  // 68
    KEY_EN_LEFT_BRACKET,            // 69
    KEY_EN_QUOTE,                   // 70
    KEY_EN_UNUSED,                  // 71
    KEY_EN_RIGHT_FUNCTION,          // 72
    KEY_EN_F11,                     // 73
    KEY_EN_UNUSED,                  // 74
    KEY_EN_RIGHT_BRACKET,           // 75
    KEY_EN_POUND,                   // 76
    KEY_EN_RIGHT_SHIFT,             // 77
    KEY_EN_MENU,                    // 78
    KEY_EN_F12,                     // 79
    KEY_EN_BACKSPACE,               // 80
    KEY_EN_ANSI_BACK_SLASH,         // 81
    KEY_EN_ANSI_ENTER,              // 82
    KEY_EN_RIGHT_CONTROL,           // 83
    KEY_EN_LEFT_ARROW,              // 84
    KEY_EN_DOWN_ARROW,              // 85
    KEY_EN_RIGHT_ARROW,             // 86
    KEY_EN_UP_ARROW,                // 87
    KEY_EN_DELETE,                  // 88
    KEY_EN_INSERT,                  // 89
    KEY_EN_PRINT_SCREEN,            // 90
    KEY_EN_UNUSED,                  // 91
    KEY_EN_UNUSED,                  // 92
    KEY_EN_SCROLL_LOCK,             // 93
    KEY_EN_HOME,                    // 94
    KEY_EN_END,                     // 95
    KEY_EN_PAGE_DOWN,               // 96
    KEY_EN_UNUSED,                  // 97
    KEY_EN_PAUSE_BREAK,             // 98
    KEY_EN_PAGE_UP,                 // 99
    KEY_EN_UNUSED,                  // 100
    KEY_EN_UNUSED,                  // 101
    KEY_EN_UNUSED,                  // 102
    "LED: Logo",                    // 103
    KEY_EN_UNUSED,                  // 104
    KEY_EN_UNUSED,                  // 105
};

/**------------------------------------------------------------------*\
    @name Fnatic Streak Keyboard
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectFnaticStreakControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_FnaticStreak::RGBController_FnaticStreak(FnaticStreakController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetNameString();
    vendor      = "Fnatic";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Fnatic Streak Keyboard Device";
    location    = controller->GetLocationString();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = 0xFFFF;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_FnaticStreak::~RGBController_FnaticStreak()
{
    /*---------------------------------------------------------*\
    | Delete the matrix map                                     |
    \*---------------------------------------------------------*/
    for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        if(zones[zone_index].matrix_map != NULL)
        {
            delete zones[zone_index].matrix_map;
        }
    }

    delete controller;
}

void RGBController_FnaticStreak::SetupZones()
{
    /*---------------------------------------------------------*\
    | Set up zones                                              |
    \*---------------------------------------------------------*/
    unsigned int    zone_size;
    unsigned int    matrix_width;
    unsigned int*   matrix_map_ptr;
    const char**    led_names_ptr;

    if(controller->IsMini())
    {
        zone_size       = 106;
        matrix_width    = 18;
        matrix_map_ptr  = (unsigned int *)&matrix_map_ministreak;
        led_names_ptr   = led_names_ministreak;
    }
    else
    {
        zone_size       = 124;
        matrix_width    = 22;
        matrix_map_ptr  = (unsigned int *)&matrix_map_streak;
        led_names_ptr   = led_names_streak;
    }

    zone new_zone;
    new_zone.name                   = zone_names[0];
    new_zone.type                   = zone_types[0];
    new_zone.leds_min               = zone_size;
    new_zone.leds_max               = zone_size;
    new_zone.leds_count             = zone_size;
    new_zone.matrix_map             = new matrix_map_type;
    new_zone.matrix_map->height     = 6;
    new_zone.matrix_map->width      = matrix_width;
    new_zone.matrix_map->map        = matrix_map_ptr;
    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < zone_size; led_idx++)
    {
        led new_led;
        new_led.name = led_names_ptr[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_FnaticStreak::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_FnaticStreak::DeviceUpdateLEDs()
{
    unsigned int led_count = controller->GetLEDCount();
    std::vector<unsigned char> colordata(led_count * 3);

    for(std::size_t color_idx = 0; color_idx < colors.size(); color_idx++)
    {
        colordata[(color_idx*3)+0] = RGBGetRValue(colors[color_idx]);
        colordata[(color_idx*3)+1] = RGBGetGValue(colors[color_idx]);
        colordata[(color_idx*3)+2] = RGBGetBValue(colors[color_idx]);
    }

    controller->SendColors(colordata.data(), colordata.size());
}

void RGBController_FnaticStreak::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_FnaticStreak::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_FnaticStreak::DeviceUpdateMode()
{
    /* Direct mode only for now */
}

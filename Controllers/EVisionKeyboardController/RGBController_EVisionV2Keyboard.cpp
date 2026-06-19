/*---------------------------------------------------------*\
| RGBController_EVisionV2Keyboard.cpp                       |
|                                                           |
|   RGBController for EVision V2 keyboard                   |
|                                                           |
|   Le Philousophe                              25 Dec 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#define NA 0xFFFFFFFF

#include <chrono>
#include "hsv.h"
#include "RGBControllerKeyNames.h"
#include "RGBController_EVisionV2Keyboard.h"

using namespace std::chrono_literals;

/*
 * KOORUI MK104 (Skillkorp K5, VID 320F PID 505B) - German QWERTZ 104-key layout
 *
 * The firmware scans the keyboard in 16 column-groups of 8 LEDs each (128 total).
 * LED index formula: index = column * 8 + row
 *
 * Row 0: Function row  (Esc, F1-F12, PrtSc, ScrLk, Break)
 * Row 1: Number row    (^, 1-0, ß, dead-acute, Backspace, Ins, Home)
 * Row 2: Tab row       (Tab, Q-+, #, Del, End)
 * Row 3: Home row      (Caps, A-Ä, Enter, PgUp, PgDn)
 * Row 4: Shift row     (LShift, <, Y--, -, RShift, Up, Num1)
 * Row 5: Control row   (LCtrl, LWin, LAlt, Space, RAlt, Fn, RMenu, RCtrl, Left, Down, Right)
 * Row 6: Numpad row    (NumLk, Num/, Num*, Num-, Num7-9, Num4-6, Num+, Num2-3, Num0, NumDel, NumEnter)
 * Row 7: Unused        (all NONE)
 */
static unsigned int koorui_mk104_matrix[KOORUI_MK104_MATRIX_HEIGHT][KOORUI_MK104_MATRIX_WIDTH] =
{
    /*  col: 0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15 */
    {   0,   8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96, 104, 112, 120 }, /* F row   */
    {   1,   9,  17,  25,  33,  41,  49,  57,  65,  73,  81,  89,  97, 105, 113, 121 }, /* Num row */
    {   2,  10,  18,  26,  34,  42,  50,  58,  66,  74,  82,  90,  98, 106, 114, 122 }, /* Tab row */
    {   3,  11,  19,  27,  35,  43,  51,  59,  67,  75,  83,  91,  NA, 107, 115, 123 }, /* Home row (col 12 row 3 = ISO Enter upper half - no LED) */
    {   4,  12,  20,  28,  36,  44,  52,  60,  68,  76,  84,  92,  NA, 108, 116, 124 }, /* Shift row */
    {   5,  13,  21,  NA,  NA,  45,  NA,  NA,  NA,  77,  85,  93, 101, 109, 117, 125 }, /* Ctrl row (cols 3-4 and 6-8 = Space area) */
    {   6,  14,  22,  30,  38,  46,  54,  62,  70,  78,  86,  94, 102, 110, 118, 126 }, /* Numpad  */
    {  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA,  NA }, /* Unused  */
};

static const char *led_koorui_mk104[] =
{
    /* Group 0, col 0: Esc, ^, Tab, Caps, LShift, LCtrl, NumLk, Unused */
    KEY_EN_ESCAPE,              /* 0   */
    "^",                        /* 1   back-tick / caret */
    KEY_EN_TAB,                 /* 2   */
    KEY_EN_CAPS_LOCK,           /* 3   */
    KEY_EN_LEFT_SHIFT,          /* 4   */
    KEY_EN_LEFT_CONTROL,        /* 5   */
    KEY_EN_NUMPAD_LOCK,         /* 6   */
    "",                         /* 7   NONE */

    /* Group 1, col 1: F1, 1, Q, A, <, LWin, Num/, Unused */
    KEY_EN_F1,                  /* 8   */
    KEY_EN_1,                   /* 9   */
    KEY_EN_Q,                   /* 10  */
    KEY_EN_A,                   /* 11  */
    KEY_EN_ISO_BACK_SLASH,      /* 12  < (ISO extra key) */
    KEY_EN_LEFT_WINDOWS,        /* 13  */
    KEY_EN_NUMPAD_DIVIDE,       /* 14  */
    "",                         /* 15  NONE */

    /* Group 2, col 2: F2, 2, W, S, Y, LAlt, Num*, Unused */
    KEY_EN_F2,                  /* 16  */
    KEY_EN_2,                   /* 17  */
    KEY_EN_W,                   /* 18  */
    KEY_EN_S,                   /* 19  */
    KEY_EN_Y,                   /* 20  QWERTZ Y (labeled Y, scancodes as Z) */
    KEY_EN_LEFT_ALT,            /* 21  */
    KEY_EN_NUMPAD_TIMES,        /* 22  */
    "",                         /* 23  NONE */

    /* Group 3, col 3: F3, 3, E, D, X, Unused, Num-, Unused */
    KEY_EN_F3,                  /* 24  */
    KEY_EN_3,                   /* 25  */
    KEY_EN_E,                   /* 26  */
    KEY_EN_D,                   /* 27  */
    KEY_EN_X,                   /* 28  */
    "",                         /* 29  NONE */
    KEY_EN_NUMPAD_MINUS,        /* 30  */
    "",                         /* 31  NONE */

    /* Group 4, col 4: F4, 4, R, F, C, Unused, Num7, Unused */
    KEY_EN_F4,                  /* 32  */
    KEY_EN_4,                   /* 33  */
    KEY_EN_R,                   /* 34  */
    KEY_EN_F,                   /* 35  */
    KEY_EN_C,                   /* 36  */
    "",                         /* 37  NONE */
    KEY_EN_NUMPAD_7,            /* 38  */
    "",                         /* 39  NONE */

    /* Group 5, col 5: F5, 5, T, G, V, Space, Num8, Unused */
    KEY_EN_F5,                  /* 40  */
    KEY_EN_5,                   /* 41  */
    KEY_EN_T,                   /* 42  */
    KEY_EN_G,                   /* 43  */
    KEY_EN_V,                   /* 44  */
    KEY_EN_SPACE,               /* 45  */
    KEY_EN_NUMPAD_8,            /* 46  */
    "",                         /* 47  NONE */

    /* Group 6, col 6: F6, 6, Z, H, B, Unused, Num9, Unused */
    KEY_EN_F6,                  /* 48  */
    KEY_EN_6,                   /* 49  */
    KEY_EN_Z,                   /* 50  QWERTZ Z (labeled Z, scancodes as Y) */
    KEY_EN_H,                   /* 51  */
    KEY_EN_B,                   /* 52  */
    "",                         /* 53  NONE */
    KEY_EN_NUMPAD_9,            /* 54  */
    "",                         /* 55  NONE */

    /* Group 7, col 7: F7, 7, U, J, N, Unused, Num4, Unused */
    KEY_EN_F7,                  /* 56  */
    KEY_EN_7,                   /* 57  */
    KEY_EN_U,                   /* 58  */
    KEY_EN_J,                   /* 59  */
    KEY_EN_N,                   /* 60  */
    "",                         /* 61  NONE */
    KEY_EN_NUMPAD_4,            /* 62  */
    "",                         /* 63  NONE */

    /* Group 8, col 8: F8, 8, I, K, M, Unused, Num5, Unused */
    KEY_EN_F8,                  /* 64  */
    KEY_EN_8,                   /* 65  */
    KEY_EN_I,                   /* 66  */
    KEY_EN_K,                   /* 67  */
    KEY_EN_M,                   /* 68  */
    "",                         /* 69  NONE */
    KEY_EN_NUMPAD_5,            /* 70  */
    "",                         /* 71  NONE */

    /* Group 9, col 9: F9, 9, O, L, ',', RAlt, Num6, Unused */
    KEY_EN_F9,                  /* 72  */
    KEY_EN_9,                   /* 73  */
    KEY_EN_O,                   /* 74  */
    KEY_EN_L,                   /* 75  */
    KEY_EN_COMMA,               /* 76  , key */
    KEY_EN_RIGHT_ALT,           /* 77  */
    KEY_EN_NUMPAD_6,            /* 78  */
    "",                         /* 79  NONE */

    /* Group 10, col 10: F10, 0, P, Ö, '.', Fn, Num+, Unused */
    KEY_EN_F10,                 /* 80  */
    KEY_EN_0,                   /* 81  */
    KEY_EN_P,                   /* 82  */
    "\xC3\x96",                 /* 83  Ö (UTF-8) */
    KEY_EN_PERIOD,              /* 84  . key */
    KEY_EN_RIGHT_FUNCTION,      /* 85  Fn */
    KEY_EN_NUMPAD_PLUS,         /* 86  */
    "",                         /* 87  NONE */

    /* Group 11, col 11: F11, ß, Ü, Ä, -(shift row), RMenu, Num2, Unused */
    KEY_EN_F11,                 /* 88  */
    "\xC3\x9F",                 /* 89  ß (UTF-8) */
    "\xC3\x9C",                 /* 90  Ü (UTF-8) */
    "\xC3\x84",                 /* 91  Ä (UTF-8) */
    KEY_EN_MINUS,               /* 92  - key (shift row, before RShift) */
    KEY_EN_MENU,                /* 93  right context menu */
    KEY_EN_NUMPAD_2,            /* 94  */
    "",                         /* 95  NONE */

    /* Group 12, col 12: F12, dead-acute, +, Unused, Unused, RCtrl, Num3, Unused */
    KEY_EN_F12,                 /* 96  */
    "Dead Acute",               /* 97  ´ key (dead acute) */
    "+",                        /* 98  + key (after Ü in Q row) */
    "",                         /* 99  NONE (ISO Enter area) */
    "",                         /* 100 NONE (ISO Enter area) */
    KEY_EN_RIGHT_CONTROL,       /* 101 */
    KEY_EN_NUMPAD_3,            /* 102 */
    "",                         /* 103 NONE */

    /* Group 13, col 13: PrtSc, Backspace, #, Enter, RShift, LeftArrow, Num0, Unused */
    KEY_EN_PRINT_SCREEN,        /* 104 */
    KEY_EN_BACKSPACE,           /* 105 */
    KEY_EN_POUND,               /* 106 # key (ISO, right of Enter) */
    KEY_EN_ISO_ENTER,           /* 107 */
    KEY_EN_RIGHT_SHIFT,         /* 108 */
    KEY_EN_LEFT_ARROW,          /* 109 */
    KEY_EN_NUMPAD_0,            /* 110 */
    "",                         /* 111 NONE */

    /* Group 14, col 14: ScrLk, Insert, Del, PgUp, Up, Down, NumDel, Unused */
    KEY_EN_SCROLL_LOCK,         /* 112 */
    KEY_EN_INSERT,              /* 113 */
    KEY_EN_DELETE,              /* 114 */
    KEY_EN_PAGE_UP,             /* 115 */
    KEY_EN_UP_ARROW,            /* 116 */
    KEY_EN_DOWN_ARROW,          /* 117 */
    KEY_EN_NUMPAD_PERIOD,       /* 118 Num Del */
    "",                         /* 119 NONE */

    /* Group 15, col 15: Break, Home, End, PgDn, Num1, RightArrow, NumEnter, Unused */
    KEY_EN_PAUSE_BREAK,         /* 120 */
    KEY_EN_HOME,                /* 121 */
    KEY_EN_END,                 /* 122 */
    KEY_EN_PAGE_DOWN,           /* 123 */
    KEY_EN_NUMPAD_1,            /* 124 */
    KEY_EN_RIGHT_ARROW,         /* 125 */
    KEY_EN_NUMPAD_ENTER,        /* 126 */
    "",                         /* 127 NONE */
};

static unsigned int evisionv2_matrix[EVISION_V2_MATRIX_HEIGHT][EVISION_V2_MATRIX_WIDTH] =
{
   /*   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   19   19   20   */
    {   0,  NA,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  NA,  NA,  NA,  NA },
    {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36 },
    {  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57 },
    {  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  NA,  NA,  NA,  72,  73,  74,  NA },
    {  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  NA,  87,  NA,  88,  NA,  89,  90,  91,  92 },
    {  93,  94,  95,  NA,  NA,  NA,  96,  NA,  NA,  NA,  97,  98,  99, 100, 101, 102, 103,  NA, 104, 105,  NA }
};

static unsigned int endorfy_matrix[EVISION_V2_MATRIX_HEIGHT][EVISION_V2_MATRIX_WIDTH] =
{
   /*   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   19   19   20   */
    {   0,  NA,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  NA,  NA,  NA,  NA },
    {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36 },
    {  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57 },
    {  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  NA,  70,  NA,  NA,  NA,  71,  72,  73,  NA },
    {  74,  NA,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  NA,  NA,  86,  NA,  87,  88,  89,  90 },
    {  91,  92,  93,  NA,  NA,  NA,  94,  NA,  NA,  NA,  95,  96,  97,  98,  99, 100, 101, 102,  NA,  103, NA }
};

static const char *led_evisionv2[] =
{
    KEY_EN_ESCAPE,              //00
    KEY_EN_F1,
    KEY_EN_F2,
    KEY_EN_F3,
    KEY_EN_F4,
    KEY_EN_F5,
    KEY_EN_F6,
    KEY_EN_F7,
    KEY_EN_F8,
    KEY_EN_F9,
    KEY_EN_F10,                 //10
    KEY_EN_F11,
    KEY_EN_F12,
    KEY_EN_PRINT_SCREEN,
    KEY_EN_SCROLL_LOCK,
    KEY_EN_PAUSE_BREAK,

    KEY_EN_BACK_TICK,           //16
    KEY_EN_1,
    KEY_EN_2,
    KEY_EN_3,
    KEY_EN_4,                   //20
    KEY_EN_5,
    KEY_EN_6,
    KEY_EN_7,
    KEY_EN_8,
    KEY_EN_9,
    KEY_EN_0,
    KEY_EN_MINUS,
    KEY_EN_EQUALS,
    KEY_EN_BACKSPACE,
    KEY_EN_INSERT,
    KEY_EN_HOME,               //30
    KEY_EN_PAGE_UP,
    KEY_EN_NUMPAD_LOCK,
    KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_NUMPAD_TIMES,
    KEY_EN_NUMPAD_MINUS,

    KEY_EN_TAB,               //37
    KEY_EN_Q,
    KEY_EN_W,
    KEY_EN_E,                 //40
    KEY_EN_R,
    KEY_EN_T,
    KEY_EN_Y,
    KEY_EN_U,
    KEY_EN_I,
    KEY_EN_O,
    KEY_EN_P,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_ANSI_BACK_SLASH,   //50
    KEY_EN_DELETE,
    KEY_EN_END,
    KEY_EN_PAGE_DOWN,
    KEY_EN_NUMPAD_7,
    KEY_EN_NUMPAD_8,
    KEY_EN_NUMPAD_9,
    KEY_EN_NUMPAD_PLUS,

    KEY_EN_CAPS_LOCK,         //58
    KEY_EN_A,
    KEY_EN_S,
    KEY_EN_D,
    KEY_EN_F,
    KEY_EN_G,
    KEY_EN_H,
    KEY_EN_J,
    KEY_EN_K,
    KEY_EN_L,
    KEY_EN_SEMICOLON,
    KEY_EN_QUOTE,
    KEY_EN_POUND,                //70
    KEY_EN_ISO_ENTER,
    KEY_EN_NUMPAD_4,
    KEY_EN_NUMPAD_5,
    KEY_EN_NUMPAD_6,

    KEY_EN_LEFT_SHIFT,          //75
    KEY_EN_ISO_BACK_SLASH,
    KEY_EN_Z,
    KEY_EN_X,
    KEY_EN_C,
    KEY_EN_V,                   //80
    KEY_EN_B,
    KEY_EN_N,
    KEY_EN_M,
    KEY_EN_COMMA,
    KEY_EN_PERIOD,
    KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_UP_ARROW,
    KEY_EN_NUMPAD_1,
    KEY_EN_NUMPAD_2,            //90
    KEY_EN_NUMPAD_3,
    KEY_EN_NUMPAD_ENTER,

    KEY_EN_LEFT_CONTROL,        //93
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_LEFT_ALT,
    KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT,
    KEY_EN_RIGHT_WINDOWS,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_RIGHT_CONTROL,       //100
    KEY_EN_LEFT_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW,
    KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_PERIOD,
};

static const char *led_endorfy[] =
{
    KEY_EN_ESCAPE,              //00
    KEY_EN_F1,
    KEY_EN_F2,
    KEY_EN_F3,
    KEY_EN_F4,
    KEY_EN_F5,
    KEY_EN_F6,
    KEY_EN_F7,
    KEY_EN_F8,
    KEY_EN_F9,
    KEY_EN_F10,                 //10
    KEY_EN_F11,
    KEY_EN_F12,
    KEY_EN_PRINT_SCREEN,
    KEY_EN_SCROLL_LOCK,
    KEY_EN_PAUSE_BREAK,

    KEY_EN_BACK_TICK,           //16
    KEY_EN_1,
    KEY_EN_2,
    KEY_EN_3,
    KEY_EN_4,                   //20
    KEY_EN_5,
    KEY_EN_6,
    KEY_EN_7,
    KEY_EN_8,
    KEY_EN_9,
    KEY_EN_0,
    KEY_EN_MINUS,
    KEY_EN_EQUALS,
    KEY_EN_BACKSPACE,
    KEY_EN_INSERT,
    KEY_EN_HOME,               //30
    KEY_EN_PAGE_UP,
    KEY_EN_NUMPAD_LOCK,
    KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_NUMPAD_TIMES,
    KEY_EN_NUMPAD_MINUS,

    KEY_EN_TAB,               //37
    KEY_EN_Q,
    KEY_EN_W,
    KEY_EN_E,                 //40
    KEY_EN_R,
    KEY_EN_T,
    KEY_EN_Y,
    KEY_EN_U,
    KEY_EN_I,
    KEY_EN_O,
    KEY_EN_P,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_ANSI_BACK_SLASH,   //50
    KEY_EN_DELETE,
    KEY_EN_END,
    KEY_EN_PAGE_DOWN,
    KEY_EN_NUMPAD_7,
    KEY_EN_NUMPAD_8,
    KEY_EN_NUMPAD_9,
    KEY_EN_NUMPAD_PLUS,

    KEY_EN_CAPS_LOCK,         //58
    KEY_EN_A,
    KEY_EN_S,
    KEY_EN_D,
    KEY_EN_F,
    KEY_EN_G,
    KEY_EN_H,
    KEY_EN_J,
    KEY_EN_K,
    KEY_EN_L,
    KEY_EN_SEMICOLON,
    KEY_EN_QUOTE,
    KEY_EN_ANSI_ENTER,          //70
    KEY_EN_NUMPAD_4,
    KEY_EN_NUMPAD_5,
    KEY_EN_NUMPAD_6,

    KEY_EN_LEFT_SHIFT,
    KEY_EN_Z,          //75
    KEY_EN_X,
    KEY_EN_C,
    KEY_EN_V,
    KEY_EN_B,                   //80
    KEY_EN_N,
    KEY_EN_M,
    KEY_EN_COMMA,
    KEY_EN_PERIOD,
    KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_UP_ARROW,
    KEY_EN_NUMPAD_1,
    KEY_EN_NUMPAD_2,
    KEY_EN_NUMPAD_3,            //90
    KEY_EN_NUMPAD_ENTER,

    KEY_EN_LEFT_CONTROL,
    KEY_EN_LEFT_WINDOWS,        //93
    KEY_EN_LEFT_ALT,
    KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_MENU,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_ARROW,       //100
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW,
    KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_PERIOD,
};

/**------------------------------------------------------------------*\
    @name EVision V2 Keyboard
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectEVisionV2Keyboard
    @comment The Evision V2 controller implements all hardware modes
      found in the OEM software. Some options may not be named correctly
      like directions for some modes.
\*-------------------------------------------------------------------*/

RGBController_EVisionV2Keyboard::RGBController_EVisionV2Keyboard(EVisionV2KeyboardController* controller_ptr, EVisionV2KeyboardPart kb_part)
{
    controller  = controller_ptr;
    part        = kb_part;

    name        = controller->GetName();
    vendor      = "Evision";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "EVision Keyboard Device";
    location    = controller->GetLocation();
    serial      = controller->GetSerial();
    layout      = controller->layout;

    switch(part)
    {
        case EVISION_V2_KEYBOARD_PART_KEYBOARD:
            SetupKeyboardModes();
            break;

        case EVISION_V2_KEYBOARD_PART_LOGO:
            name += " Logo";
            SetupLogoEdgeModes();
            break;

        case EVISION_V2_KEYBOARD_PART_EDGE:
            name += " Edge";
            SetupLogoEdgeModes();
            break;

        case ENDORFY_KEYBOARD_PART_EDGE:
            SetupEdgeModes();
            break;
    }

    SetupZones();

    LoadConfig();

    keepalive_thread_run = false;
    keepalive_thread = nullptr;
    if(part == EVISION_V2_KEYBOARD_PART_KEYBOARD)
    {
        keepalive_thread_run = true;
        keepalive_thread = new std::thread(&RGBController_EVisionV2Keyboard::KeepaliveThread, this);
    }
}

RGBController_EVisionV2Keyboard::~RGBController_EVisionV2Keyboard()
{
    if(keepalive_thread)
    {
        keepalive_thread_run = false;
        keepalive_thread->join();
        delete keepalive_thread;
    }

    delete controller;
}

void RGBController_EVisionV2Keyboard::SetupKeyboardModes()
{
    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = EVISION_V2_MODE_DIRECT;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode ColorWave;
    ColorWave.name           = "Color Wave short";
    ColorWave.value          = EVISION_V2_MODE_COLOR_WAVE_SHORT;
    ColorWave.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWave.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ColorWave.speed_max      = EVISION_V2_SPEED_FASTEST;
    ColorWave.speed          = EVISION_V2_SPEED_NORMAL;
    ColorWave.colors_min     = 1;
    ColorWave.colors_max     = 1;
    ColorWave.color_mode     = MODE_COLORS_RANDOM;
    ColorWave.colors.resize(1);
    ColorWave.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ColorWave.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ColorWave.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ColorWave);

    mode ColorWaveLong;
    ColorWaveLong.name           = "Color Wave long";
    ColorWaveLong.value          = EVISION_V2_MODE_COLOR_WAVE_LONG;
    ColorWaveLong.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWaveLong.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ColorWaveLong.speed_max      = EVISION_V2_SPEED_FASTEST;
    ColorWaveLong.speed          = EVISION_V2_SPEED_NORMAL;
    ColorWaveLong.colors_min     = 1;
    ColorWaveLong.colors_max     = 1;
    ColorWaveLong.color_mode     = MODE_COLORS_RANDOM;
    ColorWaveLong.colors.resize(1);
    ColorWaveLong.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ColorWaveLong.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ColorWaveLong.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ColorWaveLong);

    mode ColorWheel;
    ColorWheel.name           = "Color Wheel";
    ColorWheel.value          = EVISION_V2_MODE_COLOR_WHEEL;
    ColorWheel.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWheel.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ColorWheel.speed_max      = EVISION_V2_SPEED_FASTEST;
    ColorWheel.speed          = EVISION_V2_SPEED_NORMAL;
    ColorWheel.colors_min     = 1;
    ColorWheel.colors_max     = 1;
    ColorWheel.color_mode     = MODE_COLORS_RANDOM;
    ColorWheel.colors.resize(1);
    ColorWheel.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ColorWheel.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ColorWheel.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ColorWheel);

    mode SpectrumCycle;
    SpectrumCycle.name           = "Spectrum Cycle";
    SpectrumCycle.value          = EVISION_V2_MODE_SPECTRUM_CYCLE;
    SpectrumCycle.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumCycle.speed_min      = EVISION_V2_SPEED_SLOWEST;
    SpectrumCycle.speed_max      = EVISION_V2_SPEED_FASTEST;
    SpectrumCycle.speed          = EVISION_V2_SPEED_NORMAL;
    SpectrumCycle.color_mode     = MODE_COLORS_NONE;
    SpectrumCycle.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    SpectrumCycle.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    SpectrumCycle.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(SpectrumCycle);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = EVISION_V2_MODE_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Breathing.speed_max      = EVISION_V2_SPEED_FASTEST;
    Breathing.speed          = EVISION_V2_SPEED_NORMAL;
    Breathing.colors_min     = 1;
    Breathing.colors_max     = 1;
    Breathing.color_mode     = MODE_COLORS_RANDOM;
    Breathing.colors.resize(1);
    Breathing.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Breathing.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Breathing.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Breathing);

    mode Static;
    Static.name           = "Static";
    Static.value          = EVISION_V2_MODE_STATIC;
    Static.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.colors_min     = 1;
    Static.colors_max     = 1;
    Static.color_mode     = MODE_COLORS_RANDOM;
    Static.colors.resize(1);
    Static.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Static.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Static.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Static);

    mode Reactive;
    Reactive.name           = "Reactive";
    Reactive.value          = EVISION_V2_MODE_REACTIVE;
    Reactive.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Reactive.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Reactive.speed_max      = EVISION_V2_SPEED_FASTEST;
    Reactive.speed          = EVISION_V2_SPEED_NORMAL;
    Reactive.colors_min     = 1;
    Reactive.colors_max     = 1;
    Reactive.color_mode     = MODE_COLORS_RANDOM;
    Reactive.colors.resize(1);
    Reactive.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Reactive.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Reactive.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Reactive);

    mode ReactiveRipple;
    ReactiveRipple.name           = "Reactive Ripple";
    ReactiveRipple.value          = EVISION_V2_MODE_REACTIVE_RIPPLE;
    ReactiveRipple.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ReactiveRipple.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ReactiveRipple.speed_max      = EVISION_V2_SPEED_FASTEST;
    ReactiveRipple.speed          = EVISION_V2_SPEED_NORMAL;
    ReactiveRipple.colors_min     = 1;
    ReactiveRipple.colors_max     = 1;
    ReactiveRipple.color_mode     = MODE_COLORS_RANDOM;
    ReactiveRipple.colors.resize(1);
    ReactiveRipple.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ReactiveRipple.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ReactiveRipple.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ReactiveRipple);

    mode ReactiveLine;
    ReactiveLine.name           = "Reactive Line";
    ReactiveLine.value          = EVISION_V2_MODE_REACTIVE_LINE;
    ReactiveLine.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ReactiveLine.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ReactiveLine.speed_max      = EVISION_V2_SPEED_FASTEST;
    ReactiveLine.speed          = EVISION_V2_SPEED_NORMAL;
    ReactiveLine.colors_min     = 1;
    ReactiveLine.colors_max     = 1;
    ReactiveLine.color_mode     = MODE_COLORS_RANDOM;
    ReactiveLine.colors.resize(1);
    ReactiveLine.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ReactiveLine.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ReactiveLine.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ReactiveLine);

    mode Starlight;
    Starlight.name           = "Starlight";
    Starlight.value          = EVISION_V2_MODE_STARLIGHT_FAST;
    Starlight.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Starlight.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Starlight.speed_max      = EVISION_V2_SPEED_FASTEST;
    Starlight.speed          = EVISION_V2_SPEED_NORMAL;
    Starlight.colors_min     = 1;
    Starlight.colors_max     = 1;
    Starlight.color_mode     = MODE_COLORS_RANDOM;
    Starlight.colors.resize(1);
    Starlight.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Starlight.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Starlight.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Starlight);

    mode Blooming;
    Blooming.name           = "Blooming";
    Blooming.value          = EVISION_V2_MODE_BLOOMING;
    Blooming.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Blooming.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Blooming.speed_max      = EVISION_V2_SPEED_FASTEST;
    Blooming.speed          = EVISION_V2_SPEED_NORMAL;
    Blooming.color_mode     = MODE_COLORS_NONE;
    Blooming.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Blooming.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Blooming.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Blooming);

    mode RainbowWaveVertical;
    RainbowWaveVertical.name           = "Rainbow Wave vertical";
    RainbowWaveVertical.value          = EVISION_V2_MODE_RAINBOW_WAVE_VERTICAL;
    RainbowWaveVertical.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    RainbowWaveVertical.speed_min      = EVISION_V2_SPEED_SLOWEST;
    RainbowWaveVertical.speed_max      = EVISION_V2_SPEED_FASTEST;
    RainbowWaveVertical.speed          = EVISION_V2_SPEED_NORMAL;
    RainbowWaveVertical.color_mode     = MODE_COLORS_NONE;
    RainbowWaveVertical.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    RainbowWaveVertical.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    RainbowWaveVertical.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(RainbowWaveVertical);

    mode Hurricane;
    Hurricane.name           = "Hurricane";
    Hurricane.value          = EVISION_V2_MODE_HURRICANE;
    Hurricane.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Hurricane.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Hurricane.speed_max      = EVISION_V2_SPEED_FASTEST;
    Hurricane.speed          = EVISION_V2_SPEED_NORMAL;
    Hurricane.colors_min     = 1;
    Hurricane.colors_max     = 1;
    Hurricane.color_mode     = MODE_COLORS_RANDOM;
    Hurricane.colors.resize(1);
    Hurricane.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Hurricane.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Hurricane.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Hurricane);

    mode Accumulate;
    Accumulate.name           = "Accumulate";
    Accumulate.value          = EVISION_V2_MODE_ACCUMULATE;
    Accumulate.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Accumulate.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Accumulate.speed_max      = EVISION_V2_SPEED_FASTEST;
    Accumulate.speed          = EVISION_V2_SPEED_NORMAL;
    Accumulate.colors_min     = 1;
    Accumulate.colors_max     = 1;
    Accumulate.color_mode     = MODE_COLORS_RANDOM;
    Accumulate.colors.resize(1);
    Accumulate.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Accumulate.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Accumulate.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Accumulate);

    mode StarlightSlow;
    StarlightSlow.name           = "Starlight slow";
    StarlightSlow.value          = EVISION_V2_MODE_STARLIGHT_SLOW;
    StarlightSlow.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    StarlightSlow.speed_min      = EVISION_V2_SPEED_SLOWEST;
    StarlightSlow.speed_max      = EVISION_V2_SPEED_FASTEST;
    StarlightSlow.speed          = EVISION_V2_SPEED_NORMAL;
    StarlightSlow.colors_min     = 1;
    StarlightSlow.colors_max     = 1;
    StarlightSlow.color_mode     = MODE_COLORS_RANDOM;
    StarlightSlow.colors.resize(1);
    StarlightSlow.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    StarlightSlow.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    StarlightSlow.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(StarlightSlow);

    mode Visor;
    Visor.name           = "Visor";
    Visor.value          = EVISION_V2_MODE_VISOR;
    Visor.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Visor.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Visor.speed_max      = EVISION_V2_SPEED_FASTEST;
    Visor.speed          = EVISION_V2_SPEED_NORMAL;
    Visor.colors_min     = 1;
    Visor.colors_max     = 1;
    Visor.color_mode     = MODE_COLORS_RANDOM;
    Visor.colors.resize(1);
    Visor.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Visor.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Visor.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Visor);

    mode Surmount;
    Surmount.name           = "Surmount";
    Surmount.value          = EVISION_V2_MODE_SURMOUNT;
    Surmount.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Surmount.colors_min     = 1;
    Surmount.colors_max     = 1;
    Surmount.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Surmount.colors.resize(1);
    Surmount.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Surmount.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Surmount.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Surmount);

    mode RainbowCircle;
    RainbowCircle.name           = "Rainbow Circle";
    RainbowCircle.value          = EVISION_V2_MODE_RAINBOW_WAVE_CIRCLE;
    RainbowCircle.flags          = MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    RainbowCircle.color_mode     = MODE_COLORS_NONE;
    RainbowCircle.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    RainbowCircle.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    RainbowCircle.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(RainbowCircle);

    for(unsigned int i = 0; i < 10; i++)
    {
        mode Custom;
        Custom.name           = "Custom ";
        Custom.name          += std::to_string(i+1);
        Custom.value          = EVISION_V2_MODE_CUSTOM | i << 8;
        Custom.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
        Custom.color_mode     = MODE_COLORS_PER_LED;
        Custom.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
        Custom.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
        Custom.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
        modes.push_back(Custom);
    }
}

void RGBController_EVisionV2Keyboard::SetupLogoEdgeModes()
{
    mode ColorWave;
    ColorWave.name           = "Color Wave";
    ColorWave.value          = EVISION_V2_MODE2_COLOR_WAVE;
    ColorWave.flags          = MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ColorWave.speed_min      = EVISION_V2_SPEED_SLOWEST;
    ColorWave.speed_max      = EVISION_V2_SPEED_FASTEST;
    ColorWave.speed          = EVISION_V2_SPEED_NORMAL;
    ColorWave.colors_min     = 1;
    ColorWave.colors_max     = 1;
    ColorWave.color_mode     = MODE_COLORS_RANDOM;
    ColorWave.colors.resize(1);
    ColorWave.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    ColorWave.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    ColorWave.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(ColorWave);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = EVISION_V2_MODE2_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Breathing.speed_max      = EVISION_V2_SPEED_FASTEST;
    Breathing.speed          = EVISION_V2_SPEED_NORMAL;
    Breathing.colors_min     = 1;
    Breathing.colors_max     = 1;
    Breathing.color_mode     = MODE_COLORS_RANDOM;
    Breathing.colors.resize(1);
    Breathing.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Breathing.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Breathing.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Breathing);

    mode Yoyo;
    Yoyo.name           = "Yoyo";
    Yoyo.value          = EVISION_V2_MODE2_YOYO;
    Yoyo.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Yoyo.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Yoyo.speed_max      = EVISION_V2_SPEED_FASTEST;
    Yoyo.speed          = EVISION_V2_SPEED_NORMAL;
    Yoyo.colors_min     = 1;
    Yoyo.colors_max     = 1;
    Yoyo.color_mode     = MODE_COLORS_RANDOM;
    Yoyo.colors.resize(1);
    Yoyo.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Yoyo.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Yoyo.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Yoyo);

    mode Blink;
    Blink.name           = "Blink";
    Blink.value          = EVISION_V2_MODE2_BLINK;
    Blink.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Blink.speed_min      = EVISION_V2_SPEED_SLOWEST;
    Blink.speed_max      = EVISION_V2_SPEED_FASTEST;
    Blink.speed          = EVISION_V2_SPEED_NORMAL;
    Blink.colors_min     = 1;
    Blink.colors_max     = 1;
    Blink.color_mode     = MODE_COLORS_RANDOM;
    Blink.colors.resize(1);
    Blink.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Blink.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Blink.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Blink);

    mode Static;
    Static.name           = "Static";
    Static.value          = EVISION_V2_MODE2_STATIC;
    Static.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.colors_min     = 1;
    Static.colors_max     = 1;
    Static.color_mode     = MODE_COLORS_RANDOM;
    Static.colors.resize(1);
    Static.brightness_min = EVISION_V2_BRIGHTNESS_LOWEST;
    Static.brightness_max = EVISION_V2_BRIGHTNESS_HIGHEST;
    Static.brightness     = EVISION_V2_BRIGHTNESS_HIGHEST;
    modes.push_back(Static);

    mode Off;
    Off.name  = "Off";
    Off.value = EVISION_V2_MODE2_OFF;
    Off.flags = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Off);
}

void RGBController_EVisionV2Keyboard::SetupEdgeModes()
{
    mode Freeze;
    Freeze.name           = "Freeze";
    Freeze.value          = ENDORFY_MODE2_FREEZE;
    Freeze.flags          = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Freeze);

    mode ColorWave;
    ColorWave.name           = "Color Wave";
    ColorWave.value          = ENDORFY_MODE2_COLOR_WAVE;
    ColorWave.flags          = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(ColorWave);

    mode Spectrum;
    Spectrum.name           = "Spectrum Cycle";
    Spectrum.value          = ENDORFY_MODE2_SPECTRUM_CYCLE;
    Spectrum.flags          = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Spectrum);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = ENDORFY_MODE2_BREATHING;
    Breathing.flags          = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Breathing);

    mode Static;
    Static.name           = "Static";
    Static.value          = ENDORFY_MODE2_STATIC;
    Static.flags          = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Static);

    mode Off;
    Off.name  = "Off";
    Off.value = ENDORFY_MODE2_OFF;
    Off.flags = MODE_FLAG_AUTOMATIC_SAVE;
    modes.push_back(Off);
}

void RGBController_EVisionV2Keyboard::SetupZones()
{
    unsigned short leds_count;
    unsigned int *matrix_map;
    const char **led_names;

    unsigned int matrix_height;
    unsigned int matrix_width;

    switch(layout)
    {
        default:
        case EVISION_V2_KEYBOARD_LAYOUT:
            led_names = led_evisionv2;
            matrix_map = (unsigned int *)evisionv2_matrix;
            leds_count = 106;
            matrix_height = EVISION_V2_MATRIX_HEIGHT;
            matrix_width  = EVISION_V2_MATRIX_WIDTH;
            break;
        case ENDORFY_KEYBOARD_LAYOUT:
            led_names = led_endorfy;
            matrix_map = (unsigned int *)endorfy_matrix;
            leds_count = 104;
            matrix_height = EVISION_V2_MATRIX_HEIGHT;
            matrix_width  = EVISION_V2_MATRIX_WIDTH;
            break;
        case KOORUI_MK104_KEYBOARD_LAYOUT:
            led_names     = led_koorui_mk104;
            matrix_map    = (unsigned int *)koorui_mk104_matrix;
            leds_count    = 128;
            matrix_height = KOORUI_MK104_MATRIX_HEIGHT;
            matrix_width  = KOORUI_MK104_MATRIX_WIDTH;
            break;
    }

    /*-------------------------------------------------*\
    | Clear any existing color/LED configuration        |
    \*-------------------------------------------------*/
    zone KB_zone;
    KB_zone.name                = ZONE_EN_KEYBOARD;
    KB_zone.type                = ZONE_TYPE_MATRIX;
    KB_zone.leds_count          = leds_count;
    KB_zone.leds_min            = KB_zone.leds_count;
    KB_zone.leds_max            = KB_zone.leds_count;

    KB_zone.matrix_map          = new matrix_map_type;
    KB_zone.matrix_map->height = matrix_height;
    KB_zone.matrix_map->width = matrix_width;
    KB_zone.matrix_map->map     = matrix_map;
    zones.push_back(KB_zone);

    /*-------------------------------------------------*\
    | Clear any existing color/LED configuration        |
    \*-------------------------------------------------*/
    leds.clear();
    colors.clear();

    /*---------------------------------------------------------*\
    | Set up zones                                              |
    \*---------------------------------------------------------*/
    for(std::size_t zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        for(unsigned int led_index = 0; led_index < zones[zone_index].leds_count; led_index++)
        {
            led new_led;
            new_led.name  = led_names[led_index];
            new_led.value = led_index;
            leds.push_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_EVisionV2Keyboard::LoadConfig()
{
    EvisionV2ModeConfig config;

    controller->GetMode(part, config);

    int mode = config.mode;
    if((part == EVISION_V2_KEYBOARD_PART_KEYBOARD) && (mode == EVISION_V2_MODE_CUSTOM))
    {
        mode |= config.ledmode << 8;
    }
    config.direction = (config.direction == 0) ? 0 : 1;

    // Define default colors
    for(int mode_index = 0; mode_index < (int)modes.size(); mode_index++)
    {
        if(config.random_colours)
        {
            if(modes[mode_index].flags & MODE_FLAG_HAS_RANDOM_COLOR)
            {
                modes[mode_index].color_mode = MODE_COLORS_RANDOM;
            }
            if(modes[mode_index].colors.size() > 0)
            {
                modes[mode_index].colors[0] = 0xffffff;
            }
        }
        else
        {
            if(modes[mode_index].flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
            {
                modes[mode_index].color_mode = MODE_COLORS_MODE_SPECIFIC;
            }
            if(modes[mode_index].colors.size() > 0)
            {
                modes[mode_index].colors[0] = config.colour;
            }
        }
        if(modes[mode_index].value == mode)
        {
            modes[mode_index].brightness = config.brightness;
            modes[mode_index].speed = config.speed;
            if(modes[mode_index].flags & MODE_FLAG_HAS_DIRECTION_LR)
            {
                modes[mode_index].direction = (1 - config.direction) + MODE_DIRECTION_LEFT;
            }
            else if(modes[mode_index].flags & MODE_FLAG_HAS_DIRECTION_UD)
            {
                modes[mode_index].direction = (1 - config.direction) + MODE_DIRECTION_UP;
            }
            else if(modes[mode_index].flags & MODE_FLAG_HAS_DIRECTION_HV)
            {
                modes[mode_index].direction = (1 - config.direction) + MODE_DIRECTION_HORIZONTAL;
            }

            if(part == EVISION_V2_KEYBOARD_PART_KEYBOARD)
            {
                if(mode == EVISION_V2_MODE_SURMOUNT)
                {
                    switch(config.ledmode)
                    {
                        case EVISION_V2_SURMOUNT_MODE_COLOR_RED:
                            modes[mode_index].colors[0] = ToRGBColor(0xff, 0, 0);
                            break;
                        case EVISION_V2_SURMOUNT_MODE_COLOR_YELLOW:
                            modes[mode_index].colors[0] = ToRGBColor(0xff, 0xff, 0);
                            break;
                        case EVISION_V2_SURMOUNT_MODE_COLOR_GREEN:
                            modes[mode_index].colors[0] = ToRGBColor(0, 0xff, 0);
                            break;
                        case EVISION_V2_SURMOUNT_MODE_COLOR_CYAN:
                            modes[mode_index].colors[0] = ToRGBColor(0, 0xff, 0xff);
                            break;
                        default:
                            break;
                    }
                }
                else if(config.mode == EVISION_V2_MODE_CUSTOM)
                {
                    controller->GetLedsCustom(config.ledmode, colors);
                }
            }

            active_mode = mode_index;
        }
    }
}

void RGBController_EVisionV2Keyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_EVisionV2Keyboard::DeviceUpdateLEDs()
{
    if(part != EVISION_V2_KEYBOARD_PART_KEYBOARD)
    {
        return;
    }

    controller->SetLedsDirect(colors);
    has_color_set = true;
    last_update_time = std::chrono::steady_clock::now();
}

void RGBController_EVisionV2Keyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EVisionV2Keyboard::UpdateSingleLED(int led)
{
    if(part != EVISION_V2_KEYBOARD_PART_KEYBOARD)
    {
        return;
    }

    controller->SetLedDirect(led, colors[led]);
    has_color_set = true;
    last_update_time = std::chrono::steady_clock::now();
}

void RGBController_EVisionV2Keyboard::DeviceUpdateMode()
{
    mode set_mode = modes[active_mode];

    // No mode set packets required for Direct mode
    if((part == EVISION_V2_KEYBOARD_PART_KEYBOARD) && (set_mode.value == EVISION_V2_MODE_DIRECT))
    {
        return;
    }

    EvisionV2ModeConfig config;

    config.mode           = set_mode.value & 0xff;
    config.brightness     = set_mode.brightness;
    config.speed          = set_mode.speed;
    config.direction      = 1 - (set_mode.direction & 0x1);
    config.random_colours = (set_mode.color_mode == MODE_COLORS_RANDOM);
    config.colour         = 0;
    if(modes[active_mode].colors.size() > 0)
    {
        config.colour = set_mode.colors[0];
    }

    config.ledmode = 0;
    if(part == EVISION_V2_KEYBOARD_PART_KEYBOARD)
    {
        if(config.mode == EVISION_V2_MODE_SURMOUNT)
        {
            hsv_t temp;
            rgb2hsv(config.colour, &temp);

            if(temp.hue <= 30 || temp.hue > 300)
            {
                config.ledmode = EVISION_V2_SURMOUNT_MODE_COLOR_RED;
            }
            else if(temp.hue > 30 && temp.hue <= 90)
            {
                config.ledmode = EVISION_V2_SURMOUNT_MODE_COLOR_YELLOW;
            }
            else if(temp.hue > 90 && temp.hue <= 150)
            {
                config.ledmode = EVISION_V2_SURMOUNT_MODE_COLOR_GREEN;
            }
            else if(temp.hue > 150 && temp.hue <= 300)
            {
                config.ledmode = EVISION_V2_SURMOUNT_MODE_COLOR_CYAN;
            }
        }
        else if(config.mode == EVISION_V2_MODE_CUSTOM)
        {
            config.ledmode = (set_mode.value >> 8) & 0xff;
        }
    }

    controller->SetMode(part, config);

    if((part == EVISION_V2_KEYBOARD_PART_KEYBOARD) && (config.mode == EVISION_V2_MODE_CUSTOM))
    {
        controller->GetLedsCustom(config.ledmode, colors);
        SignalUpdate();
    }
}

void RGBController_EVisionV2Keyboard::DeviceSaveMode()
{
    if(part != EVISION_V2_KEYBOARD_PART_KEYBOARD)
    {
        return;
    }
    int value = modes[active_mode].value;
    if((value & 0xff) == EVISION_V2_MODE_CUSTOM)
    {
        controller->SetLedsCustom((value >> 8) & 0xff, colors);
    }
}

void RGBController_EVisionV2Keyboard::KeepaliveThread()
{
    bool was_active = false;
    while(keepalive_thread_run.load())
    {
        if(modes[active_mode].value == EVISION_V2_MODE_DIRECT && has_color_set)
        {
            if((std::chrono::steady_clock::now() - last_update_time) > std::chrono::milliseconds(200))
            {
                controller->RefreshLedDirect();
                last_update_time = std::chrono::steady_clock::now();
                was_active = true;
            }
        }
        else if(was_active)
        {
            controller->EndLedsDirect();
            was_active = false;
        }
        std::this_thread::sleep_for(100ms);
    }
    controller->EndLedsDirect();
}

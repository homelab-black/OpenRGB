
/*---------------------------------------------------------*\
|   Clevo8910Keys.h                                         |
|                                                           |
|   Driver for Clevo Color per key keyboard                 |
|                                                           |
|   EzyUser     9 Jan 2026                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "RGBControllerKeyNames.h"
#include "RGBController.h"

#define NA                      0xFFFFFFFF


static const unsigned int clevo8910_leds_map[] =
    {    0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
        20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  NA,  35,  36,  37,  38,
        39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  NA,  54,  55,  56,  57,
        58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  NA,  73,  74,  75,  76,
        77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  NA,  92,  93,  94,  95,
        96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,  NA, 112, 113, 114 };
#define CLEVO8910_NUM_LEDS       (sizeof (clevo8910_leds_map) / sizeof (unsigned int))
#define CLEVO8910_KEYROWS       6
#define CLEVO8910_KEYSINROW     20

const led clevo8910_leds[]
{
    //row 1
    {KEY_EN_ESCAPE,             0x00},//0
    {KEY_EN_F1,                 0x01},//1
    {KEY_EN_F2,                 0x02},//2
    {KEY_EN_F3,                 0x03},//3
    {KEY_EN_F4,                 0x04},//4
    {KEY_EN_F5,                 0x05},//5
    {KEY_EN_F6,                 0x06},//6
    {KEY_EN_F7,                 0x07},//7
    {KEY_EN_F8,                 0x08},//8
    {KEY_EN_F9,                 0x09},//9
    {KEY_EN_F10,                0x0A},//10
    {KEY_EN_F11,                0x0B},//11
    {KEY_EN_F12,                0x0C},//12
    {KEY_EN_PRINT_SCREEN,       0x0D},//15
    {KEY_EN_INSERT,             0x0E},//14
    {KEY_EN_DELETE,             0x0F},//15
    {KEY_EN_HOME,               0x10},//16
    {KEY_EN_END,                0x11},//17
    {KEY_EN_PAGE_UP,            0x12},//18
    {KEY_EN_PAGE_DOWN,          0x13},//19

    //row 2
    {KEY_EN_BACK_TICK,          0x20},//20
    {KEY_EN_1,                  0x21},//21
    {KEY_EN_2,                  0x22},//22
    {KEY_EN_3,                  0x23},//23
    {KEY_EN_4,                  0x24},//24
    {KEY_EN_5,                  0x25},//25
    {KEY_EN_6,                  0x26},//26
    {KEY_EN_7,                  0x27},//27
    {KEY_EN_8,                  0x28},//28
    {KEY_EN_9,                  0x29},//29
    {KEY_EN_0,                  0x2A},//30
    {KEY_EN_MINUS,              0x2B},//31
    {KEY_EN_EQUALS,             0x2D},//32
    {KEY_EN_BACKSPACE,          0x2E},//33
    {KEY_EN_BACKSPACE,          0x2F},//34
    {KEY_EN_NUMPAD_LOCK,        0x30},//35
    {KEY_EN_NUMPAD_DIVIDE,      0x31},//36
    {KEY_EN_NUMPAD_TIMES,       0x32},//37
    {KEY_EN_NUMPAD_MINUS,       0x33},//38

    //row 3
    {KEY_EN_TAB,                0x40},//39
    {KEY_EN_TAB,                0x41},//40
    {KEY_EN_Q,                  0x42},//41
    {KEY_EN_W,                  0x43},//42
    {KEY_EN_E,                  0x44},//43
    {KEY_EN_R,                  0x45},//44
    {KEY_EN_T,                  0x46},//45
    {KEY_EN_Y,                  0x47},//46
    {KEY_EN_U,                  0x48},//47
    {KEY_EN_I,                  0x49},//48
    {KEY_EN_O,                  0x4A},//49
    {KEY_EN_P,                  0x4B},//50
    {KEY_EN_LEFT_BRACKET,       0x4C},//51
    {KEY_EN_RIGHT_BRACKET,      0x4D},//52
    {KEY_EN_ANSI_BACK_SLASH,    0x4E},//53
    {KEY_EN_NUMPAD_7,           0x50},//54
    {KEY_EN_NUMPAD_8,           0x51},//55
    {KEY_EN_NUMPAD_9,           0x52},//56
    {KEY_EN_NUMPAD_PLUS,        0x53},//57

    //row 4
    {KEY_EN_CAPS_LOCK,          0x60},//58
    {KEY_EN_CAPS_LOCK,          0x61},//59
    {KEY_EN_A,                  0x62},//60
    {KEY_EN_S,                  0x63},//61
    {KEY_EN_D,                  0x64},//62
    {KEY_EN_F,                  0x65},//63
    {KEY_EN_G,                  0x66},//64
    {KEY_EN_H,                  0x67},//65
    {KEY_EN_J,                  0x68},//66
    {KEY_EN_K,                  0x69},//67
    {KEY_EN_L,                  0x6A},//68
    {KEY_EN_SEMICOLON,          0x6B},//69
    {KEY_EN_QUOTE,              0x6C},//70
    {KEY_EN_ANSI_ENTER,         0x6E},//71
    {KEY_EN_ANSI_ENTER,         0x6F},//72
    {KEY_EN_NUMPAD_4,           0x70},//73
    {KEY_EN_NUMPAD_5,           0x71},//74
    {KEY_EN_NUMPAD_6,           0x72},//75
    {KEY_EN_NUMPAD_PLUS,        0x73},//76

    //row 5
    {KEY_EN_LEFT_SHIFT,         0x80},//77
    {KEY_EN_LEFT_SHIFT,         0x82},//78
    {KEY_EN_Z,                  0x83},//79
    {KEY_EN_X,                  0x84},//80
    {KEY_EN_C,                  0x85},//81
    {KEY_EN_V,                  0x86},//82
    {KEY_EN_B,                  0x87},//83
    {KEY_EN_N,                  0x88},//84
    {KEY_EN_M,                  0x89},//85
    {KEY_EN_COMMA,              0x8A},//86
    {KEY_EN_PERIOD,             0x8B},//87
    {KEY_EN_FORWARD_SLASH,      0x8C},//88
    {KEY_EN_RIGHT_SHIFT,        0x8D},//89
    {KEY_EN_RIGHT_SHIFT,        0x8E},//90
    {KEY_EN_UP_ARROW,           0x8F},//91
    {KEY_EN_NUMPAD_1,           0x90},//92
    {KEY_EN_NUMPAD_2,           0x91},//93
    {KEY_EN_NUMPAD_3,           0x92},//94
    {KEY_EN_NUMPAD_ENTER,       0x93},//95

    //row 6
    {KEY_EN_LEFT_CONTROL,       0xA0},//96
    {KEY_EN_LEFT_CONTROL,       0xA1},//97
    {KEY_EN_LEFT_FUNCTION,      0xA2},//98
    {KEY_EN_LEFT_WINDOWS,       0xA3},//99
    {KEY_EN_LEFT_ALT,           0xA4},//100
    {KEY_EN_SPACE,              0xA5},//101
    {KEY_EN_SPACE,              0xA6},//102
    {KEY_EN_SPACE,              0xA8},//103
    {KEY_EN_SPACE,              0xA9},//104
    {KEY_EN_RIGHT_ALT,          0xAA},//105
    {KEY_EN_MENU,               0xAB},//106
    {KEY_EN_RIGHT_CONTROL,      0xAC},//107
    {KEY_EN_RIGHT_CONTROL,      0xAD},//108
    {KEY_EN_LEFT_ARROW,         0xAE},//109
    {KEY_EN_DOWN_ARROW,         0xAF},//110
    {KEY_EN_RIGHT_ARROW,        0xB0},//111
    {KEY_EN_NUMPAD_0,           0xB1},//112
    {KEY_EN_NUMPAD_PERIOD,      0xB2},//113
    {KEY_EN_NUMPAD_ENTER,       0xB3},//114
};
#define CLEVO8910_KEYBOARD_LED_COUNT  (sizeof (clevo8910_leds) / sizeof (led))

/*---------------------------------------------------------*\
| RGBController_RedragonK652.cpp                           |
|                                                          |
|   RGBController for the wired Redragon K652               |
|                                                          |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_RedragonK652.h"

#include "RGBControllerKeyNames.h"

#define NA 0xFFFFFFFF

namespace
{
    static unsigned int k652_matrix_map[6][16] =
    {
        {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
        { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, NA, 30 },
        { 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, NA, 45 },
        { 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, NA, NA, 59 },
        { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, NA, NA, 72, 73 },
        /* Fn occupies the NA cell between Right Alt (78) and Right Control (79). */
        { 74, 75, 76, NA, NA, NA, 77, NA, NA, NA, 78, NA, 79, 80, 81, 82 }
    };

    static const char* k652_led_names[redragon_k652::LED_COUNT] =
    {
        KEY_EN_ESCAPE,
        KEY_EN_F1,
        KEY_EN_F2,
        KEY_EN_F3,
        KEY_EN_F4,
        KEY_EN_F5,
        KEY_EN_F6,
        KEY_EN_F7,
        KEY_EN_F8,
        KEY_EN_F9,
        KEY_EN_F10,
        KEY_EN_F11,
        KEY_EN_F12,
        KEY_EN_PRINT_SCREEN,
        KEY_EN_PAUSE_BREAK,
        KEY_EN_DELETE,

        KEY_EN_BACK_TICK,
        KEY_EN_1,
        KEY_EN_2,
        KEY_EN_3,
        KEY_EN_4,
        KEY_EN_5,
        KEY_EN_6,
        KEY_EN_7,
        KEY_EN_8,
        KEY_EN_9,
        KEY_EN_0,
        KEY_EN_MINUS,
        KEY_EN_EQUALS,
        KEY_EN_BACKSPACE,
        KEY_EN_HOME,

        KEY_EN_TAB,
        KEY_EN_Q,
        KEY_EN_W,
        KEY_EN_E,
        KEY_EN_R,
        KEY_EN_T,
        KEY_EN_Y,
        KEY_EN_U,
        KEY_EN_I,
        KEY_EN_O,
        KEY_EN_P,
        KEY_EN_LEFT_BRACKET,
        KEY_EN_RIGHT_BRACKET,
        KEY_EN_ANSI_BACK_SLASH,
        KEY_EN_END,

        KEY_EN_CAPS_LOCK,
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
        KEY_EN_ANSI_ENTER,
        KEY_EN_PAGE_UP,

        KEY_EN_LEFT_SHIFT,
        KEY_EN_Z,
        KEY_EN_X,
        KEY_EN_C,
        KEY_EN_V,
        KEY_EN_B,
        KEY_EN_N,
        KEY_EN_M,
        KEY_EN_COMMA,
        KEY_EN_PERIOD,
        KEY_EN_FORWARD_SLASH,
        KEY_EN_RIGHT_SHIFT,
        KEY_EN_UP_ARROW,
        KEY_EN_PAGE_DOWN,

        KEY_EN_LEFT_CONTROL,
        KEY_EN_LEFT_WINDOWS,
        KEY_EN_LEFT_ALT,
        KEY_EN_SPACE,
        KEY_EN_RIGHT_ALT,
        KEY_EN_RIGHT_CONTROL,
        KEY_EN_LEFT_ARROW,
        KEY_EN_DOWN_ARROW,
        KEY_EN_RIGHT_ARROW
    };
}

/**------------------------------------------------------------------*\
    @name Redragon K652
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectRedragonK652
    @comment Experimental wired support; Static and Direct only
\*-------------------------------------------------------------------*/

RGBController_RedragonK652::RGBController_RedragonK652(RedragonK652Controller* controller_ptr)
    : controller(controller_ptr)
{
    name        = controller->GetName();
    vendor      = "Redragon";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Redragon K652 Wired Keyboard";
    location    = controller->GetLocation();

    /*-----------------------------------------------------*\
    | Do not let saved profiles write lighting, whether they  |
    | are loaded automatically or manually.  Profile apply is |
    | intentionally unsupported by this experimental device. |
    \*-----------------------------------------------------*/
    flags      |= CONTROLLER_FLAG_NO_PROFILE_APPLY;

    mode Static;
    Static.name         = "Static";
    Static.value        = redragon_k652::MODE_STATIC;
    Static.flags        = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Static.color_mode   = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min   = 1;
    Static.colors_max   = 1;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = redragon_k652::MODE_DIRECT;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    active_mode = 0;

    SetupZones();
}

RGBController_RedragonK652::~RGBController_RedragonK652()
{
    Shutdown();
    delete controller;
}

void RGBController_RedragonK652::SetupZones()
{
    zone new_zone;
    new_zone.name       = ZONE_EN_KEYBOARD;
    new_zone.type       = ZONE_TYPE_MATRIX;
    new_zone.leds_count = controller->GetLEDCount();
    new_zone.leds_min   = new_zone.leds_count;
    new_zone.leds_max   = new_zone.leds_count;
    new_zone.matrix_map.Set(6, 16, reinterpret_cast<unsigned int*>(&k652_matrix_map));

    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < redragon_k652::LED_COUNT; led_idx++)
    {
        led new_led;
        new_led.name  = k652_led_names[led_idx];
        new_led.value = redragon_k652::KEY_INDICES[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_RedragonK652::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == redragon_k652::MODE_DIRECT)
    {
        controller->ApplyDirect(colors);
    }
}

void RGBController_RedragonK652::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RedragonK652::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RedragonK652::DeviceUpdateMode()
{
    if(modes[active_mode].value == redragon_k652::MODE_STATIC)
    {
        controller->ApplyStatic(modes[active_mode].colors[0]);
    }
    else if(modes[active_mode].value == redragon_k652::MODE_DIRECT)
    {
        controller->ApplyDirect(colors);
    }
}

void RGBController_RedragonK652::DeviceEnableUserLightingControl()
{
    /*-------------------------------------------------------------*\
    | Detection and registration leave the low-level controller     |
    | locked.  Only an explicit UI, CLI, or SDK request reaches this |
    | hook and permanently enables lighting writes for this instance.|
    \*-------------------------------------------------------------*/
    controller->EnableUserLightingControl();
}

#undef NA

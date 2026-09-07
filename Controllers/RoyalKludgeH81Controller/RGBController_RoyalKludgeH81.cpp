/*---------------------------------------------------------*\
| RGBController_RoyalKludgeH81.cpp                          |
|                                                           |
|   RGBController for Royal Kludge RK-H81 keyboard          |
|                                                           |
|   eclecticBelletrist                          04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_RoyalKludgeH81.h"
#include "RGBControllerKeyNames.h"
#include "LogManager.h"

using namespace std::chrono_literals;

/*---------------------------------------------------------------*\
| the H81 physical layout is 6 rows by 16 columns.  values are    |
| indexes into the leds vector, NA marks a matrix position with   |
| no physical key                                                 |
\*---------------------------------------------------------------*/
#define NA  0xFFFFFFFF

static const unsigned int matrix_map[6][16] =
{
    {  0,  6, 12, 18, 23, 28, 34, 39, 44, 49, 55, 60, 64, NA, 73, NA },
    {  1,  7, 13, 19, 24, 29, 35, 40, 45, 50, 56, 61, 65, 68, NA, 76 },
    {  2,  8, 14, 20, 25, 30, 36, 41, 46, 51, 57, 62, 66, 69, NA, 77 },
    {  3,  9, 15, 21, 26, 31, 37, 42, 47, 52, 58, 63, NA, 70, NA, 78 },
    {  4, 10, 16, 22, 27, 32, 38, 43, 48, 53, 59, NA, NA, 71, 74, NA },
    {  5, 11, 17, NA, NA, 33, NA, NA, NA, 54, NA, NA, 67, 72, 75, 79 },
};

/*---------------------------------------------------------------*\
| LED names in leds vector order (matches rkh81_led_offsets in    |
| RoyalKludgeH81Controller.cpp)                                   |
\*---------------------------------------------------------------*/
static const char* const led_names[RKH81_LED_COUNT] =
{
    KEY_EN_ESCAPE,      KEY_EN_BACK_TICK,   KEY_EN_TAB,         KEY_EN_CAPS_LOCK,
    KEY_EN_LEFT_SHIFT,  KEY_EN_LEFT_CONTROL,KEY_EN_F1,          KEY_EN_1,
    KEY_EN_Q,           KEY_EN_A,           KEY_EN_Z,           KEY_EN_LEFT_WINDOWS,
    KEY_EN_F2,          KEY_EN_2,           KEY_EN_W,           KEY_EN_S,
    KEY_EN_X,           KEY_EN_LEFT_ALT,    KEY_EN_F3,          KEY_EN_3,
    KEY_EN_E,           KEY_EN_D,           KEY_EN_C,           KEY_EN_F4,
    KEY_EN_4,           KEY_EN_R,           KEY_EN_F,           KEY_EN_V,
    KEY_EN_F5,          KEY_EN_5,           KEY_EN_T,           KEY_EN_G,
    KEY_EN_B,           KEY_EN_SPACE,       KEY_EN_F6,          KEY_EN_6,
    KEY_EN_Y,           KEY_EN_H,           KEY_EN_N,           KEY_EN_F7,
    KEY_EN_7,           KEY_EN_U,           KEY_EN_J,           KEY_EN_M,
    KEY_EN_F8,          KEY_EN_8,           KEY_EN_I,           KEY_EN_K,
    KEY_EN_COMMA,       KEY_EN_F9,          KEY_EN_9,           KEY_EN_O,
    KEY_EN_L,           KEY_EN_PERIOD,      KEY_EN_RIGHT_ALT,   KEY_EN_F10,
    KEY_EN_0,           KEY_EN_P,           KEY_EN_SEMICOLON,   KEY_EN_FORWARD_SLASH,
    KEY_EN_F11,         KEY_EN_MINUS,       KEY_EN_LEFT_BRACKET,KEY_EN_QUOTE,
    KEY_EN_F12,         KEY_EN_EQUALS,      KEY_EN_RIGHT_BRACKET,KEY_EN_RIGHT_CONTROL,
    KEY_EN_BACKSPACE,   KEY_EN_ANSI_BACK_SLASH, KEY_EN_ANSI_ENTER, KEY_EN_RIGHT_SHIFT,
    KEY_EN_LEFT_ARROW,  KEY_EN_DELETE,      KEY_EN_UP_ARROW,    KEY_EN_DOWN_ARROW,
    KEY_EN_HOME,        KEY_EN_PAGE_UP,     KEY_EN_PAGE_DOWN,   KEY_EN_RIGHT_ARROW,
};

/**------------------------------------------------------------------*\
    @name Royal Kludge H81 Keyboard
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectRoyalKludgeH81Controllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_RoyalKludgeH81::RGBController_RoyalKludgeH81(RoyalKludgeH81Controller* controller_ptr)
{
    controller = controller_ptr;

    name        = "Royal Kludge H81 Keyboard";
    type        = DEVICE_TYPE_KEYBOARD;
    vendor      = "Royal Kludge";
    description = "Royal Kludge / BY Tech H81 Keyboard, per-key RGB via vendor HID interface";
    location    = controller->GetLocation();
    serial      = controller->GetSerialString();

    mode Off;
    Off.name       = "Off";
    Off.flags      = 0;
    Off.color_mode = MODE_COLORS_NONE;
    Off.value      = rkh81::MODE_OFF;
    modes.push_back(Off);

    mode Direct;
    Direct.name       = "Direct";
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    Direct.value      = rkh81::MODE_DIRECT;
    modes.push_back(Direct);

    active_mode = rkh81::MODE_DIRECT;

    SetupZones();

    // keeps the device from falling back to its stored profile, see KeepaliveThreadFunction()
    last_update_time_ns.store(std::chrono::steady_clock::now().time_since_epoch().count());

    keepalive_thread_run = true;
    keepalive_thread     = new std::thread(&RGBController_RoyalKludgeH81::KeepaliveThreadFunction, this);
}

RGBController_RoyalKludgeH81::~RGBController_RoyalKludgeH81()
{
    {
        std::lock_guard<std::mutex> lock(keepalive_wake_mutex);
        keepalive_thread_run = false;
    }
    keepalive_wake_cv.notify_all();
    keepalive_thread->join();
    delete keepalive_thread;

    Shutdown();

    RKH81UnclaimPath(controller->GetPath());

    delete controller;
}

void RGBController_RoyalKludgeH81::SetupZones()
{
    zone new_zone;
    new_zone.name       = ZONE_EN_KEYBOARD;
    new_zone.type       = ZONE_TYPE_MATRIX;
    new_zone.leds_min   = RKH81_LED_COUNT;
    new_zone.leds_max   = RKH81_LED_COUNT;
    new_zone.leds_count = RKH81_LED_COUNT;
    new_zone.matrix_map.Set(6, 16, const_cast<unsigned int*>(&matrix_map[0][0]));

    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < RKH81_LED_COUNT; led_idx++)
    {
        led new_led;
        new_led.name = led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_RoyalKludgeH81::DeviceUpdateLEDs()
{
    last_update_time_ns.store(std::chrono::steady_clock::now().time_since_epoch().count());
    controller->SetLEDsDirect(colors);
}

void RGBController_RoyalKludgeH81::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludgeH81::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludgeH81::DeviceUpdateMode()
{
    // called from DeviceCallThreadFunction(), which already holds AccessMutex.lock_shared()
    int mode_value = modes[active_mode].value;

    last_update_time_ns.store(std::chrono::steady_clock::now().time_since_epoch().count());

    // Off is the one-shot hardware command, doesn't need the keepalive Direct relies on
    if(mode_value == rkh81::MODE_OFF)
    {
        controller->SetLightsOff();
    }
    else if(mode_value == rkh81::MODE_DIRECT)
    {
        controller->SetLEDsDirect(colors);
    }
}

void RGBController_RoyalKludgeH81::KeepaliveThreadFunction()
{
    // Direct mode needs a refresh roughly every second or the device falls back
    // to its stored profile, Off is a persistent hardware state and is left alone here
    std::unique_lock<std::mutex> wake_lock(keepalive_wake_mutex);

    while(keepalive_thread_run.load())
    {
        AccessMutex.lock_shared();
        int          mode_value = modes[active_mode].value;
        AccessMutex.unlock_shared();

        int64_t now_ns  = std::chrono::steady_clock::now().time_since_epoch().count();
        int64_t last_ns = last_update_time_ns.load();

        if((mode_value == rkh81::MODE_DIRECT) && ((now_ns - last_ns) > std::chrono::nanoseconds(500ms).count()))
        {
            UpdateLEDsInternal();
        }

        keepalive_wake_cv.wait_for(wake_lock, 400ms, [this]{ return(!keepalive_thread_run.load()); });
    }
}

/*---------------------------------------------------------*\
| RGBController_RoyalKludgeS98.cpp                          |
|                                                           |
|   RGBController for Royal Kludge S98 keyboard             |
|                                                           |
|   manumyx                                     03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_RoyalKludgeS98.h"

#define NA 0xFFFFFFFF

#define ROYAL_KLUDGE_S98_MATRIX_ROWS                        6
#define ROYAL_KLUDGE_S98_MATRIX_COLS                        19
#define ROYAL_KLUDGE_S98_LED_COUNT                          100

static unsigned int matrix_map[ROYAL_KLUDGE_S98_MATRIX_ROWS][ROYAL_KLUDGE_S98_MATRIX_COLS] =
{
    {  0, NA,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, NA, NA, NA, NA },
    { 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 },
    { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 },
    { 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, NA },
    { 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, NA, 82, 83, 84, 85, 86, 87 },
    { 88, 89, 90, NA, NA, 91, NA, NA, 92, 93, 94, NA, NA, 95, 96, 97, 98, 99, NA }
};

struct RoyalKludgeS98KeyMapEntry
{
    const char*     name;
    unsigned char   matrix_index;
};

static const RoyalKludgeS98KeyMapEntry led_names[ROYAL_KLUDGE_S98_LED_COUNT] =
{
    /* Row 0 */
    { KEY_EN_ESCAPE,            0   },
    { KEY_EN_F1,                12  },
    { KEY_EN_F2,                18  },
    { KEY_EN_F3,                24  },
    { KEY_EN_F4,                30  },
    { KEY_EN_F5,                36  },
    { KEY_EN_F6,                42  },
    { KEY_EN_F7,                48  },
    { KEY_EN_F8,                54  },
    { KEY_EN_F9,                60  },
    { KEY_EN_F10,               66  },
    { KEY_EN_F11,               72  },
    { KEY_EN_F12,               78  },
    { KEY_EN_DELETE,            84  },

    /* Row 1 */
    { KEY_EN_BACK_TICK,         1   },
    { KEY_EN_1,                 7   },
    { KEY_EN_2,                 13  },
    { KEY_EN_3,                 19  },
    { KEY_EN_4,                 25  },
    { KEY_EN_5,                 31  },
    { KEY_EN_6,                 37  },
    { KEY_EN_7,                 43  },
    { KEY_EN_8,                 49  },
    { KEY_EN_9,                 55  },
    { KEY_EN_0,                 61  },
    { KEY_EN_MINUS,             67  },
    { KEY_EN_EQUALS,            73  },
    { KEY_EN_BACKSPACE,         79  },
    { KEY_EN_HOME,              85  },
    { KEY_EN_NUMPAD_LOCK,       91  },
    { KEY_EN_NUMPAD_DIVIDE,     97  },
    { KEY_EN_NUMPAD_TIMES,      103 },
    { KEY_EN_NUMPAD_MINUS,      109 },

    /* Row 2 */
    { KEY_EN_TAB,               2   },
    { KEY_EN_Q,                 8   },
    { KEY_EN_W,                 14  },
    { KEY_EN_E,                 20  },
    { KEY_EN_R,                 26  },
    { KEY_EN_T,                 32  },
    { KEY_EN_Y,                 38  },
    { KEY_EN_U,                 44  },
    { KEY_EN_I,                 50  },
    { KEY_EN_O,                 56  },
    { KEY_EN_P,                 62  },
    { KEY_EN_LEFT_BRACKET,      68  },
    { KEY_EN_RIGHT_BRACKET,     74  },
    { KEY_EN_ANSI_BACK_SLASH,   80  },
    { KEY_EN_PAGE_UP,           86  },
    { KEY_EN_NUMPAD_7,          92  },
    { KEY_EN_NUMPAD_8,          98  },
    { KEY_EN_NUMPAD_9,          104 },
    { KEY_EN_NUMPAD_PLUS,       110 },

    /* Row 3 */
    { KEY_EN_CAPS_LOCK,         3   },
    { KEY_EN_A,                 9   },
    { KEY_EN_S,                 15  },
    { KEY_EN_D,                 21  },
    { KEY_EN_F,                 27  },
    { KEY_EN_G,                 33  },
    { KEY_EN_H,                 39  },
    { KEY_EN_J,                 45  },
    { KEY_EN_K,                 51  },
    { KEY_EN_L,                 57  },
    { KEY_EN_SEMICOLON,         63  },
    { KEY_EN_QUOTE,             69  },
    { KEY_EN_POUND,             75  },
    { KEY_EN_ANSI_ENTER,        81  },
    { KEY_EN_PAGE_DOWN,         87  },
    { KEY_EN_NUMPAD_4,          93  },
    { KEY_EN_NUMPAD_5,          99  },
    { KEY_EN_NUMPAD_6,          105 },

    /* Row 4 */
    { KEY_EN_LEFT_SHIFT,        4   },
    { KEY_EN_ISO_BACK_SLASH,    76  },
    { KEY_EN_Z,                 10  },
    { KEY_EN_X,                 16  },
    { KEY_EN_C,                 22  },
    { KEY_EN_V,                 28  },
    { KEY_EN_B,                 34  },
    { KEY_EN_N,                 40  },
    { KEY_EN_M,                 46  },
    { KEY_EN_COMMA,             52  },
    { KEY_EN_PERIOD,            58  },
    { KEY_EN_FORWARD_SLASH,     64  },
    { KEY_EN_RIGHT_SHIFT,       82  },
    { KEY_EN_UP_ARROW,          88  },
    { KEY_EN_NUMPAD_1,          94  },
    { KEY_EN_NUMPAD_2,          100 },
    { KEY_EN_NUMPAD_3,          106 },
    { KEY_EN_NUMPAD_ENTER,      112 },

    /* Row 5 */
    { KEY_EN_LEFT_CONTROL,      5   },
    { KEY_EN_LEFT_WINDOWS,      11  },
    { KEY_EN_LEFT_ALT,          17  },
    { KEY_EN_SPACE,             35  },
    { KEY_EN_RIGHT_ALT,         53  },
    { KEY_EN_RIGHT_FUNCTION,    59  },
    { KEY_EN_RIGHT_CONTROL,     65  },
    { KEY_EN_LEFT_ARROW,        83  },
    { KEY_EN_DOWN_ARROW,        89  },
    { KEY_EN_RIGHT_ARROW,       95  },
    { KEY_EN_NUMPAD_0,          101 },
    { KEY_EN_NUMPAD_PERIOD,     107 }
};

/**------------------------------------------------------------------*\
    @name Royal Kludge S98
    @category Keyboard
    @type USB
    @save :robot:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectRoyalKludgeS98WiredControllers,DetectRoyalKludgeS98WirelessControllers
    @comment Supports real-time direct streaming over USB wired connection, per-key custom lighting, and built-in hardware effects over both USB wired and 2.4GHz wireless connections.
\*-------------------------------------------------------------------*/

RGBController_RoyalKludgeS98::RGBController_RoyalKludgeS98(RoyalKludgeS98Controller* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = controller->GetNameString();
    vendor                      = "Royal Kludge";
    type                        = DEVICE_TYPE_KEYBOARD;
    description                 = controller->IsWireless() ? "Royal Kludge S98 Keyboard Device (2.4GHz Wireless)" : "Royal Kludge S98 Keyboard Device (Wired USB)";
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();
    version                     = controller->GetFirmwareVersion();

    if(!controller->IsWireless())
    {
        mode Direct;
        Direct.name             = "Direct";
        Direct.value            = ROYAL_KLUDGE_S98_MODE_DIRECT;
        Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
        Direct.color_mode       = MODE_COLORS_PER_LED;
        modes.push_back(Direct);
    }

    mode Custom;
    Custom.name                 = "Custom";
    Custom.value                = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
    Custom.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Custom.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    AddEffectMode("Static",             ROYAL_KLUDGE_S98_MODE_STATIC,               false);
    AddEffectMode("Breathing",          ROYAL_KLUDGE_S98_MODE_BREATHING,            true);
    AddEffectMode("Rainbow Wave",       ROYAL_KLUDGE_S98_MODE_RAINBOW_WAVE,         true);
    AddEffectMode("Flash Away",         ROYAL_KLUDGE_S98_MODE_FLASH_AWAY,           true);
    AddEffectMode("Raindrops",          ROYAL_KLUDGE_S98_MODE_RAINDROPS,            true);
    AddEffectMode("Rainbow Wheel",      ROYAL_KLUDGE_S98_MODE_RAINBOW_WHEEL,        true);
    AddEffectMode("Ripple Shining",     ROYAL_KLUDGE_S98_MODE_RIPPLE_SHINING,       true);
    AddEffectMode("Stars Twinkle",      ROYAL_KLUDGE_S98_MODE_STARS_TWINKLE,        true);
    AddEffectMode("Shadow Disappear",   ROYAL_KLUDGE_S98_MODE_SHADOW_DISAPPEAR,     true);
    AddEffectMode("Retro Snake",        ROYAL_KLUDGE_S98_MODE_RETRO_SNAKE,          true);
    AddEffectMode("Neon Stream",        ROYAL_KLUDGE_S98_MODE_NEON_STREAM,          true);
    AddEffectMode("Reactive",           ROYAL_KLUDGE_S98_MODE_REACTIVE,             true);
    AddEffectMode("Sine Wave",          ROYAL_KLUDGE_S98_MODE_SINE_WAVE,            true);
    AddEffectMode("Side Scan",          ROYAL_KLUDGE_S98_MODE_SIDE_SCAN,            true);
    AddEffectMode("Rotating Windmill",  ROYAL_KLUDGE_S98_MODE_ROTATING_WINDMILL,    true);
    AddEffectMode("Colorful Waterfall", ROYAL_KLUDGE_S98_MODE_COLORFUL_WATERFALL,   true);
    AddEffectMode("Blossoming",         ROYAL_KLUDGE_S98_MODE_BLOSSOMING,           true);

    mode Off;
    Off.name                    = "Off";
    Off.value                   = ROYAL_KLUDGE_S98_MODE_OFF;
    Off.flags                   = MODE_FLAG_AUTOMATIC_SAVE;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    active_mode                 = 0;

    if(controller->IsWireless())
    {
        unsigned char current_hw_mode = controller->GetCurrentMode();

        for(unsigned int i = 0; i < modes.size(); i++)
        {
            if(modes[i].value == current_hw_mode)
            {
                active_mode = i;
                break;
            }
        }
    }

    SetupZones();

    direct_active               = false;
    frame_pending               = false;
    frame_direct                = false;
    frame_generation            = 0;
    generation                  = 0;
    mode_pending                = false;

    writer_thread_run           = true;
    writer_thread               = new std::thread(&RGBController_RoyalKludgeS98::WriterThreadFunction, this);
}

RGBController_RoyalKludgeS98::~RGBController_RoyalKludgeS98()
{
    Shutdown();

    frame_mutex.lock();
    writer_thread_run = false;
    frame_mutex.unlock();
    frame_cv.notify_all();
    writer_thread->join();
    delete writer_thread;

    controller->RestoreMode();
    delete controller;
}

void RGBController_RoyalKludgeS98::AddEffectMode(std::string mode_name, unsigned char mode_value, bool has_speed)
{
    mode Effect;
    Effect.name                 = mode_name;
    Effect.value                = mode_value;
    Effect.flags                = MODE_FLAG_HAS_BRIGHTNESS
                                | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR
                                | MODE_FLAG_HAS_RANDOM_COLOR
                                | MODE_FLAG_AUTOMATIC_SAVE;

    if(has_speed)
    {
        Effect.flags           |= MODE_FLAG_HAS_SPEED;
        Effect.speed_min        = ROYAL_KLUDGE_S98_SPEED_MIN;
        Effect.speed_max        = ROYAL_KLUDGE_S98_SPEED_MAX;
        Effect.speed            = controller->GetEffectSpeed(mode_value);
    }

    Effect.brightness_min       = ROYAL_KLUDGE_S98_BRIGHTNESS_MIN;
    Effect.brightness_max       = ROYAL_KLUDGE_S98_BRIGHTNESS_MAX;
    Effect.brightness           = controller->GetEffectBrightness(mode_value);
    Effect.colors_min           = 1;
    Effect.colors_max           = 1;
    Effect.colors.resize(1);
    Effect.colors[0]            = controller->GetEffectColor(mode_value);

    if(mode_value != ROYAL_KLUDGE_S98_MODE_STATIC)
    {
        Effect.color_mode       = MODE_COLORS_RANDOM;
    }
    else
    {
        Effect.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    }

    modes.push_back(Effect);
}

void RGBController_RoyalKludgeS98::SetupZones()
{
    zone keyboard_zone;
    keyboard_zone.name          = ZONE_EN_KEYBOARD;
    keyboard_zone.type          = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min      = ROYAL_KLUDGE_S98_LED_COUNT;
    keyboard_zone.leds_max      = ROYAL_KLUDGE_S98_LED_COUNT;
    keyboard_zone.leds_count    = ROYAL_KLUDGE_S98_LED_COUNT;
    keyboard_zone.matrix_map.Set(ROYAL_KLUDGE_S98_MATRIX_ROWS, ROYAL_KLUDGE_S98_MATRIX_COLS, (unsigned int*)&matrix_map);
    zones.push_back(keyboard_zone);

    for(unsigned int led_idx = 0; led_idx < ROYAL_KLUDGE_S98_LED_COUNT; led_idx++)
    {
        led new_led;
        new_led.name            = led_names[led_idx].name;
        new_led.value           = led_names[led_idx].matrix_index;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_RoyalKludgeS98::DiscardPendingFrame()
{
    /*-----------------------------------------------------*\
    | Must be called with writer_mutex held.  Invalidates   |
    | any queued frame so that a stale frame cannot switch  |
    | the keyboard back after a mode change.                |
    \*-----------------------------------------------------*/
    generation++;

    std::lock_guard<std::mutex> lock(frame_mutex);
    frame_pending = false;
}

void RGBController_RoyalKludgeS98::QueueFrame(const std::vector<RoyalKludgeS98LedSlot>& led_slots, bool direct)
{
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame_slots      = led_slots;
        frame_direct     = direct;
        frame_generation = generation.load();
        frame_pending    = true;
    }

    frame_cv.notify_one();
}

void RGBController_RoyalKludgeS98::QueueModeUpdate
    (
    unsigned char mode_value,
    unsigned char brightness,
    unsigned char speed,
    unsigned char color_flag,
    RGBColor      color
    )
{
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        pending_mode_update.mode_value = mode_value;
        pending_mode_update.brightness = brightness;
        pending_mode_update.speed      = speed;
        pending_mode_update.color_flag = color_flag;
        pending_mode_update.color      = color;
        mode_pending                   = true;
    }

    frame_cv.notify_one();
}

void RGBController_RoyalKludgeS98::DeviceUpdateLEDs()
{
    std::vector<RoyalKludgeS98LedSlot> led_slots(leds.size());
    bool all_black = true;

    for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        led_slots[led_idx].matrix_index = (unsigned char)leds[led_idx].value;
        led_slots[led_idx].color        = colors[led_idx];

        if(colors[led_idx] != 0)
        {
            all_black = false;
        }
    }

    if(modes[active_mode].value == ROYAL_KLUDGE_S98_MODE_SELF_DEFINE)
    {
        direct_active = false;
        QueueFrame(led_slots, false);
    }
    else if(modes[active_mode].value == ROYAL_KLUDGE_S98_MODE_DIRECT && !controller->IsWireless())
    {
        if(!direct_active.load() && all_black)
        {
            return;
        }

        direct_active = true;
        QueueFrame(led_slots, true);
    }
}

void RGBController_RoyalKludgeS98::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludgeS98::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludgeS98::DeviceUpdateMode()
{
    if(modes[active_mode].value == ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        if(direct_active.load())
        {
            DiscardPendingFrame();
            QueueModeUpdate(ROYAL_KLUDGE_S98_MODE_DIRECT, 0, 0, 0, 0);
            DeviceUpdateLEDs();
        }
        return;
    }

    direct_active = false;

    if(modes[active_mode].value == ROYAL_KLUDGE_S98_MODE_SELF_DEFINE)
    {
        DiscardPendingFrame();
        QueueModeUpdate(ROYAL_KLUDGE_S98_MODE_SELF_DEFINE, 0, 0, 0, 0);
        DeviceUpdateLEDs();
        return;
    }

    unsigned char color_flag = ROYAL_KLUDGE_S98_COLOR_FLAG_RANDOM;
    RGBColor      mode_color = 0;

    if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC && modes[active_mode].colors.size() > 0)
    {
        color_flag = ROYAL_KLUDGE_S98_COLOR_FLAG_SPECIFIC;
        mode_color = modes[active_mode].colors[0];
    }

    DiscardPendingFrame();
    QueueModeUpdate
        (
        (unsigned char)modes[active_mode].value,
        (unsigned char)modes[active_mode].brightness,
        (unsigned char)modes[active_mode].speed,
        color_flag,
        mode_color
        );
}

void RGBController_RoyalKludgeS98::WriterThreadFunction()
{
    std::chrono::time_point<std::chrono::steady_clock> last_send_time = std::chrono::steady_clock::now();

    while(true)
    {
        {
            std::unique_lock<std::mutex> lock(frame_mutex);
            frame_cv.wait(lock, [this]{ return(frame_pending || mode_pending || !writer_thread_run.load()); });

            if(!writer_thread_run.load())
            {
                break;
            }
        }

        /*-----------------------------------------------------*\
        | Handle pending mode / slider changes first            |
        \*-----------------------------------------------------*/
        bool                     has_mode_update = false;
        RoyalKludgeS98ModeUpdate mode_update;

        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if(mode_pending)
            {
                mode_update     = pending_mode_update;
                mode_pending    = false;
                has_mode_update = true;
            }
        }

        if(has_mode_update)
        {
            std::lock_guard<std::mutex> lock(writer_mutex);
            controller->SetMode
                (
                mode_update.mode_value,
                mode_update.brightness,
                mode_update.speed,
                mode_update.color_flag,
                mode_update.color
                );
            last_send_time = std::chrono::steady_clock::now();
            continue;
        }

        /*-----------------------------------------------------*\
        | Pace transfers to protect the keyboard MCU USB buffer |
        | and prevent endpoint stalls and watchdog resets.      |
        | 23ms target interval (~43.5 FPS cap) with a minimum   |
        | 5ms recovery rest delivers peak physical throughput   |
        | with 100% stability.                                  |
        \*-----------------------------------------------------*/
        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
        long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_send_time).count();
        if(elapsed < 23)
        {
            long long wait_time = std::max(5LL, 23LL - elapsed);
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
        }

        std::vector<RoyalKludgeS98LedSlot> led_slots;
        bool                               direct;
        unsigned int                       frame_gen;

        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if(!frame_pending || !writer_thread_run.load())
            {
                continue;
            }

            led_slots.swap(frame_slots);
            direct        = frame_direct;
            frame_gen     = frame_generation;
            frame_pending = false;
        }

        std::lock_guard<std::mutex> lock(writer_mutex);

        if(frame_gen != generation.load())
        {
            continue;
        }

        if(direct)
        {
            controller->SetLedsDirect(led_slots);
        }
        else
        {
            controller->SetLedColors(led_slots);
        }

        last_send_time = std::chrono::steady_clock::now();
    }
}
/*---------------------------------------------------------*\
| RGBController_RoccatVulcanKeyboard.cpp                    |
|                                                           |
|   RGBController for Roccat Vulcan keyboard                |
|                                                           |
|   Mola19                                      17 Dec 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "RGBControllerKeyNames.h"
#include "RGBController_RoccatVulcanKeyboard.h"

#define NA  0xFFFFFFFF

/**------------------------------------------------------------------*\
    @name Roccat Vulcan Keyboard
    @category Keyboard
    @type USB
    @save :robot:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectRoccatVulcanKeyboardControllers
    @comment The mode "Default" differs from device to device and
    and sometimes also based on which profile you are on.
    Often it is very close to the rainbow mode.
\*-------------------------------------------------------------------*/

RGBController_RoccatVulcanKeyboard::RGBController_RoccatVulcanKeyboard(RoccatVulcanKeyboardController* controller_ptr)
{
    controller                  = controller_ptr;

    pid                         = controller->device_pid;

    controller->InitDeviceInfo();

    name                        = controller->GetName();
    vendor                      = "Roccat";
    type                        = DEVICE_TYPE_KEYBOARD;
    description                 = "Roccat Vulcan Keyboard Device";
    version                     = controller->GetDeviceInfo().version;
    location                    = controller->GetLocation();
    serial                      = controller->GetSerial();

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = ROCCAT_VULCAN_MODE_DIRECT;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode           = MODE_COLORS_PER_LED;

    if(pid != ROCCAT_VULCAN_120_AIMO_PID && pid != ROCCAT_VULCAN_100_AIMO_PID)
    {
        Direct.flags           |= MODE_FLAG_HAS_BRIGHTNESS;
        Direct.brightness_min   = ROCCAT_VULCAN_BRIGHTNESS_MIN;
        Direct.brightness_max   = ROCCAT_VULCAN_BRIGHTNESS_MAX;
        Direct.brightness       = ROCCAT_VULCAN_BRIGHTNESS_DEFAULT;
    }

    modes.push_back(Direct);

    mode Static;
    Static.name                 = "Static";
    Static.value                = ROCCAT_VULCAN_MODE_STATIC;
    Static.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        // Direct streaming is not stored in keyboard onboard memory.
        Static.flags &= ~MODE_FLAG_AUTOMATIC_SAVE;
    }
    Static.brightness_min       = ROCCAT_VULCAN_BRIGHTNESS_MIN;
    Static.brightness_max       = ROCCAT_VULCAN_BRIGHTNESS_MAX;
    Static.brightness           = ROCCAT_VULCAN_BRIGHTNESS_DEFAULT;
    Static.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Wave;
    Wave.name                   = "Rainbow Wave";
    Wave.value                  = ROCCAT_VULCAN_MODE_WAVE;
    Wave.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Wave.speed_min              = ROCCAT_VULCAN_SPEED_MIN;
    Wave.speed_max              = ROCCAT_VULCAN_SPEED_MAX;
    Wave.speed                  = ROCCAT_VULCAN_SPEED_DEFAULT;
    Wave.brightness_min         = ROCCAT_VULCAN_BRIGHTNESS_MIN;
    Wave.brightness_max         = ROCCAT_VULCAN_BRIGHTNESS_MAX;
    Wave.brightness             = ROCCAT_VULCAN_BRIGHTNESS_DEFAULT;
    Wave.color_mode             = MODE_COLORS_NONE;
    if(pid != TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        modes.push_back(Wave);
    }

    mode Default;
    Default.name                = "Default";
    Default.value               = ROCCAT_VULCAN_MODE_DEFAULT;
    Default.flags               = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Default.brightness_min      = ROCCAT_VULCAN_BRIGHTNESS_MIN;
    Default.brightness_max      = ROCCAT_VULCAN_BRIGHTNESS_MAX;
    Default.brightness          = ROCCAT_VULCAN_BRIGHTNESS_DEFAULT;
    Default.color_mode          = MODE_COLORS_NONE;
    if(pid != TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        modes.push_back(Default);
    }

    if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        // Software effects: the 5009 firmware's native mode commands are not verified.
        mode Cycle;
        Cycle.name = "Color Cycle (Software)";
        Cycle.value = 0x80;
        Cycle.flags = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
        Cycle.color_mode = MODE_COLORS_NONE;
        Cycle.speed_min = ROCCAT_VULCAN_SPEED_MIN;
        Cycle.speed_max = ROCCAT_VULCAN_SPEED_MAX;
        Cycle.speed = ROCCAT_VULCAN_SPEED_DEFAULT;
        Cycle.brightness_min = ROCCAT_VULCAN_BRIGHTNESS_MIN;
        Cycle.brightness_max = ROCCAT_VULCAN_BRIGHTNESS_MAX;
        Cycle.brightness = ROCCAT_VULCAN_BRIGHTNESS_DEFAULT;
        modes.push_back(Cycle);

        mode Rainbow = Cycle;
        Rainbow.name = "Rainbow (Software)";
        Rainbow.value = 0x81;
        modes.push_back(Rainbow);

        mode RainbowWave = Cycle;
        RainbowWave.name = "Rainbow Wave (Software)";
        RainbowWave.value = 0x82;
        modes.push_back(RainbowWave);

        mode Breathing = Cycle;
        Breathing.name = "Breathing (Software)";
        Breathing.value = 0x83;
        Breathing.color_mode = MODE_COLORS_PER_LED;
        Breathing.flags |= MODE_FLAG_HAS_PER_LED_COLOR;
        modes.push_back(Breathing);
    }

    SetupZones();
    if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        effect_led_ids.reserve(leds.size());
        for(const led &key : leds)
        {
            effect_led_ids.push_back(key.value);
        }
        effect_thread = std::thread(&RGBController_RoccatVulcanKeyboard::EffectLoop, this);
    }
}

RGBController_RoccatVulcanKeyboard::~RGBController_RoccatVulcanKeyboard()
{
    Shutdown();
    effect_stop.store(true);
    effect_cv.notify_all();
    if(effect_thread.joinable()) effect_thread.join();
    if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        controller->EnableDirect(false);
    }

    delete controller;
}

void RGBController_RoccatVulcanKeyboard::SetupZones()
{
    std::map<int,layout_info> * keyboard_ptr;

    switch(pid)
    {
        case ROCCAT_VULCAN_100_AIMO_PID:
        case ROCCAT_VULCAN_120_AIMO_PID:
            keyboard_ptr = &RoccatVulcan120AimoLayouts;
            break;
        case ROCCAT_VULCAN_TKL_PID:
        case ROCCAT_VULCAN_TKL_PRO_PID:
        case TURTLE_BEACH_VULCAN_II_TKL_PID:
            keyboard_ptr = &RoccatVulcanTKLLayouts;
            break;
        case TURTLE_BEACH_VULCAN_II_TKL_PRO_PID:
            keyboard_ptr = &TurtleBeachVulcanIITKLProLayouts;
            break;
        case TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID:
            keyboard_ptr = &TurtleBeachVulcanIITKLPro5009Layouts;
            break;
        case ROCCAT_VULCAN_PRO_PID:
        case ROCCAT_PYRO_PID:
            keyboard_ptr = &RoccatPyroLayouts;
            break;
        case ROCCAT_VULCAN_II_PID:
        case TURTLE_BEACH_VULCAN_II_PID:
            keyboard_ptr = &RoccatVulcanIILayouts;
            break;
        case ROCCAT_VULCAN_II_MAX_PID:
            keyboard_ptr = &RoccatVulcanIIMaxLayouts;
            break;
        case ROCCAT_VULCAN_II_MINI_PID:
            keyboard_ptr = &RoccatVulcanIIMiniLayouts;
            break;
        case ROCCAT_MAGMA_PID:
        case ROCCAT_MAGMA_MINI_PID:
            keyboard_ptr = &RoccatMagmaLayouts;
            break;
        default:
            keyboard_ptr = &RoccatVulcan120AimoLayouts;
    }

    std::map<int,layout_info> & keyboard = *keyboard_ptr;

    unsigned char layout;

    switch(controller->GetDeviceInfo().layout_type)
    {
        case ROCCAT_VULCAN_LAYOUT_DE:
        case ROCCAT_VULCAN_LAYOUT_UK:
        case ROCCAT_VULCAN_LAYOUT_FR:
            layout = ROCCAT_VULCAN_LAYOUT_UK;
            break;
        case ROCCAT_VULCAN_LAYOUT_JP:
            layout = ROCCAT_VULCAN_LAYOUT_JP;
            break;
        case ROCCAT_VULCAN_LAYOUT_US:
        default:
            layout = ROCCAT_VULCAN_LAYOUT_US;
    }
    /* Use the verified LED matrix for the Turtle Beach 5009. */
    if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID &&
       keyboard.find(layout) == keyboard.end())
    {
        layout = ROCCAT_VULCAN_LAYOUT_UK;
    }

    /*---------------------------------------------------------*\
    | Determine zone sizes                                      |
    | Vulcan II MAX has 108 keyboard keys + 24 secondary LEDs   |
    | (physically under parent keys) + 16 palm rest LEDs.       |
    | Secondary LEDs are split into labeled linear zones.       |
    \*---------------------------------------------------------*/
    int keyboard_size = keyboard[layout].size;

    if(pid == ROCCAT_VULCAN_II_MAX_PID)
    {
        /*---------------------------------------------------------*\
        | Dynamically calculate the main keyboard zone size by      |
        | subtracting the 40 secondary/palm rest LEDs from the      |
        | total layout size. This prevents JIS keys from overflow.  |
        \*---------------------------------------------------------*/
        keyboard_size = keyboard[layout].size - 40;

    }

    zone keyboard_zone;
    keyboard_zone.name               = "Keyboard";
    keyboard_zone.type               = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min           = keyboard_size;
    keyboard_zone.leds_max           = keyboard_size;
    keyboard_zone.leds_count         = keyboard_size;
    keyboard_zone.matrix_map.Set(keyboard[layout].rows, keyboard[layout].cols, keyboard[layout].matrix_map);
    zones.push_back(keyboard_zone);

    if(pid == ROCCAT_VULCAN_II_MAX_PID)
    {
        zone fkey_ind_zone;
        fkey_ind_zone.name               = "F-Key Indicators";
        fkey_ind_zone.type               = ZONE_TYPE_LINEAR;
        fkey_ind_zone.leds_min           = 15;
        fkey_ind_zone.leds_max           = 15;
        fkey_ind_zone.leds_count         = 15;
        zones.push_back(fkey_ind_zone);

        zone nav1_zone;
        nav1_zone.name               = "Nav Cluster Indicators";
        nav1_zone.type               = ZONE_TYPE_LINEAR;
        nav1_zone.leds_min           = 3;
        nav1_zone.leds_max           = 3;
        nav1_zone.leds_count         = 3;
        zones.push_back(nav1_zone);

        zone numlock_zone;
        numlock_zone.name               = "Num Lock Indicator";
        numlock_zone.type               = ZONE_TYPE_LINEAR;
        numlock_zone.leds_min           = 1;
        numlock_zone.leds_max           = 1;
        numlock_zone.leds_count         = 1;
        zones.push_back(numlock_zone);

        zone nav2_zone;
        nav2_zone.name               = "Nav Cluster Indicators 2";
        nav2_zone.type               = ZONE_TYPE_LINEAR;
        nav2_zone.leds_min           = 3;
        nav2_zone.leds_max           = 3;
        nav2_zone.leds_count         = 3;
        zones.push_back(nav2_zone);

        zone caps_zone;
        caps_zone.name               = "Caps Lock Indicator";
        caps_zone.type               = ZONE_TYPE_LINEAR;
        caps_zone.leds_min           = 1;
        caps_zone.leds_max           = 1;
        caps_zone.leds_count         = 1;
        zones.push_back(caps_zone);

        zone win_zone;
        win_zone.name               = "Win Key Indicator";
        win_zone.type               = ZONE_TYPE_LINEAR;
        win_zone.leds_min           = 1;
        win_zone.leds_max           = 1;
        win_zone.leds_count         = 1;
        zones.push_back(win_zone);

        zone palmrest_zone;
        palmrest_zone.name               = "Palm Rest";
        palmrest_zone.type               = ZONE_TYPE_LINEAR;
        palmrest_zone.leds_min           = 16;
        palmrest_zone.leds_max           = 16;
        palmrest_zone.leds_count         = 16;
        zones.push_back(palmrest_zone);
    }

    for(int led_id = 0; led_id < keyboard[layout].size; led_id++)
    {
        led new_led;
        new_led.name  = keyboard[layout].led_names[led_id].name;
        new_led.value = keyboard[layout].led_names[led_id].id;
        leds.push_back(new_led);
    }

 SetupColors();

// PID 5009: enable direct mode without sending the older model's SendMode.
if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
{
    controller->EnableDirect(true);
    DeviceUpdateLEDs();
}
else
{
    DeviceUpdateMode();
    DeviceUpdateLEDs();
}
}

void RGBController_RoccatVulcanKeyboard::DeviceUpdateLEDs()
{
if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        const mode &selected = modes[active_mode];
        if(selected.value >= 0x80 && selected.value <= 0x83)
        {
            // Snapshot UI-owned mode state; worker never accesses modes/colors.
            effect_speed.store(selected.speed);
            effect_brightness.store(selected.brightness);
            if(selected.value == 0x83)
            {
                std::lock_guard<std::mutex> lock(effect_data_mutex);
                effect_base_colors.clear();
                for(unsigned int i = 0; i < colors.size() && i < leds.size(); ++i)
                    effect_base_colors.push_back({leds[i].value, colors[i]});
            }
            effect_mode.store(selected.value);
            effect_cv.notify_all();
            return;
        }
        effect_mode.store(0);
        effect_cv.notify_all();
        std::vector<led_color> frame_colors;
        frame_colors.reserve(colors.size());
        for(unsigned int i = 0; i < colors.size() && i < leds.size(); ++i)
            frame_colors.push_back({leds[i].value, Scale5009(colors[i], selected.brightness)});
        std::lock_guard<std::mutex> io_lock(effect_io_mutex);
        controller->SendColors(frame_colors);
        return;
    }

    if(modes[active_mode].value == ROCCAT_VULCAN_MODE_DIRECT)
    {
        std::vector<led_color> led_color_list = {};

        for(unsigned int i = 0; i < colors.size(); i++)
        {
            led_color_list.push_back({ leds[i].value, colors[i] });
        }

        controller->SendColors(led_color_list);
    }
    else
    {
        DeviceUpdateMode();
    }
}

void RGBController_RoccatVulcanKeyboard::DeviceUpdateZoneLEDs(int /*zone_idx*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoccatVulcanKeyboard::DeviceUpdateSingleLED(int /*led_idx*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoccatVulcanKeyboard::DeviceUpdateMode()
{
if(pid == TURTLE_BEACH_VULCAN_II_TKL_PRO_REV2_PID)
    {
        // Software effects use the same verified direct RGB protocol.
        DeviceUpdateLEDs();
        return;
    }

    std::vector<led_color> led_color_list = {};

    if(modes[active_mode].value == ROCCAT_VULCAN_MODE_STATIC)
    {
        for(unsigned int i = 0; i < colors.size(); i++)
        {
            led_color_list.push_back({ leds[i].value, colors[i] });
        }
    }

    controller->SendMode(modes[active_mode].value, modes[active_mode].speed, modes[active_mode].brightness, led_color_list);
    controller->WaitUntilReady();

    controller->EnableDirect(modes[active_mode].value == ROCCAT_VULCAN_MODE_DIRECT);
    controller->WaitUntilReady();
}

// Only used by 10F5:5009. The old Roccat controller path is unchanged.
RGBColor RGBController_RoccatVulcanKeyboard::Scale5009(RGBColor color, unsigned int brightness)
{
    const unsigned int lo = ROCCAT_VULCAN_BRIGHTNESS_MIN;
    const unsigned int hi = ROCCAT_VULCAN_BRIGHTNESS_MAX;
    const unsigned int b = std::max(lo, std::min(brightness, hi)) - lo;
    const unsigned int span = hi - lo;
    return ToRGBColor((RGBGetRValue(color) * b + span / 2) / span,
                      (RGBGetGValue(color) * b + span / 2) / span,
                      (RGBGetBValue(color) * b + span / 2) / span);
}

RGBColor RGBController_RoccatVulcanKeyboard::Hue5009(double hue)
{
    hue -= std::floor(hue);
    const double h = hue * 6.0;
    const double x = 1.0 - std::abs(std::fmod(h, 2.0) - 1.0);
    double r = 0, g = 0, b = 0;
    if(h < 1)      { r = 1; g = x; }
    else if(h < 2) { r = x; g = 1; }
    else if(h < 3) { g = 1; b = x; }
    else if(h < 4) { g = x; b = 1; }
    else if(h < 5) { r = x; b = 1; }
    else           { r = 1; b = x; }
    return ToRGBColor(static_cast<unsigned int>(r * 255 + .5),
                      static_cast<unsigned int>(g * 255 + .5),
                      static_cast<unsigned int>(b * 255 + .5));
}

void RGBController_RoccatVulcanKeyboard::EffectLoop()
{
    using clock = std::chrono::steady_clock;
    const auto origin = clock::now();
    while(!effect_stop.load())
    {
        const unsigned int kind = effect_mode.load();
        if(kind < 0x80 || kind > 0x83)
        {
            std::unique_lock<std::mutex> lock(effect_wait_mutex);
            effect_cv.wait_for(lock, std::chrono::milliseconds(250), [this] {
                return effect_stop.load() || (effect_mode.load() >= 0x80 && effect_mode.load() <= 0x83);
            });
            continue;
        }

        const unsigned int speed = std::max(1u, std::min(effect_speed.load(), 11u));
        const unsigned int brightness = effect_brightness.load();
        const double t = std::chrono::duration<double>(clock::now() - origin).count();
        const double phase = t * (0.035 + speed * 0.025);
        std::vector<led_color> frame;
        if(kind == 0x83)
        {
            std::lock_guard<std::mutex> lock(effect_data_mutex);
            frame = effect_base_colors;
        }
        else
        {
            frame.reserve(effect_led_ids.size());
            for(unsigned int led_id : effect_led_ids)
            {
                frame.push_back({led_id, 0});
            }
        }

        const double breath = 0.5 - 0.5 * std::cos(phase * 6.283185307179586);
        for(unsigned int i = 0; i < frame.size(); ++i)
        {
            RGBColor c = frame[i].color;
            if(kind == 0x80) c = Hue5009(phase);
            if(kind == 0x81) c = Hue5009(phase + frame[i].value / 91.0);
            if(kind == 0x82) c = Hue5009(phase + frame[i].value / 28.0);
            if(kind == 0x83)
                c = ToRGBColor(static_cast<unsigned int>(RGBGetRValue(c) * breath),
                               static_cast<unsigned int>(RGBGetGValue(c) * breath),
                               static_cast<unsigned int>(RGBGetBValue(c) * breath));
            frame[i].color = Scale5009(c, brightness);
        }

        {
            std::lock_guard<std::mutex> io_lock(effect_io_mutex);
            if(!effect_stop.load() && effect_mode.load() == kind)
                controller->SendColors(frame);
        }
        // Wait between frames to avoid busy-looping; USB writes are synchronous.
        std::unique_lock<std::mutex> wait_lock(effect_wait_mutex);
        effect_cv.wait_for(wait_lock, std::chrono::milliseconds(8), [this, kind] {
            return effect_stop.load() || effect_mode.load() != kind;
        });
    }
}

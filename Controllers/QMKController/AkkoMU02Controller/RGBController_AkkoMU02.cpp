/*---------------------------------------------------------*\
| RGBController_AkkoMU02.cpp                                |
|                                                           |
|   RGBController for Akko MU02 VIA keyboards               |
|                                                           |
|   OpenRGB contributors                        11 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AkkoMU02.h"
#include "LogManager.h"
#include "hsv.h"

/**------------------------------------------------------------------*\
    @name Akko MU02 ISO
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectAkkoMU02Controllers
    @comment Hardware effects, global brightness and static per-key colours via VIA 12.
\*-------------------------------------------------------------------*/

static const char* mu02_effects[] =
{
    "All Off", "Solid Color", "Alphas Mods", "Gradient Up Down", "Gradient Left Right",
    "Breathing", "Band Sat", "Band Val", "Band Pinwheel Sat", "Band Pinwheel Val",
    "Band Spiral Sat", "Band Spiral Val", "Cycle All", "Cycle Left Right", "Cycle Up Down",
    "Rainbow Moving Chevron", "Cycle Out In", "Cycle Out In Dual", "Cycle Pinwheel",
    "Cycle Spiral", "Dual Beacon", "Rainbow Beacon", "Rainbow Pinwheels", "Raindrops",
    "Jellybean Raindrops", "Hue Breathing", "Hue Pendulum", "Hue Wave", "Pixel Rain",
    "Pixel Flow", "Pixel Fractal", "Typing Heatmap", "Digital Rain", "Solid Reactive Simple",
    "Solid Reactive", "Solid Reactive Wide", "Solid Reactive Multiwide", "Solid Reactive Cross",
    "Solid Reactive Multicross", "Solid Reactive Nexus", "Solid Reactive Multinexus", "Splash",
    "Multisplash", "Solid Splash", "Solid Multisplash", "Custom"
};

// ISO matrix coordinates, not physical key widths; the unlit rotary knob is omitted.
static const char* mu02_keys[6][15] =
{
    {"Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Delete", nullptr},
    {"Grave", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equals", "Backspace", "Home"},
    {"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Left Bracket", "Right Bracket", nullptr, "Page Up"},
    {"Caps Lock", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Semicolon", "Apostrophe", "ISO Hash", "Enter", "Page Down"},
    {"Left Shift", "ISO Backslash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Period", "Slash", "Right Shift", "Up", "End"},
    {"Left Ctrl", "Left Super", "Left Alt", nullptr, nullptr, nullptr, "Space", nullptr, nullptr, "Right Alt", "Fn", "Right Ctrl", "Left", "Down", "Right"}
};

RGBController_AkkoMU02::RGBController_AkkoMU02(AkkoMU02Controller* controller_ptr)
    : controller(controller_ptr)
{
    name        = "Akko MU02 ISO";
    vendor      = "Akko";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Akko MU02 ISO VIA RGB Matrix";
    location    = controller->GetLocation();
    version     = "VIA 12";

    const AkkoMU02State& state = controller->GetState();
    last_speed          = state.speed;
    last_brightness     = state.brightness;
    hsv_t hsv_color;
    hsv_color.hue           = (state.hue * 360u) / 256u;
    hsv_color.saturation    = state.saturation;
    hsv_color.value         = 255;
    last_color              = hsv2rgb(&hsv_color);

    for(unsigned int effect = 0; effect < sizeof(mu02_effects) / sizeof(mu02_effects[0]); effect++)
    {
        mode m;
        m.name              = mu02_effects[effect];
        m.value             = effect;
        m.speed             = state.speed;
        m.speed_min         = 0;
        m.speed_max         = 255;
        m.brightness        = state.brightness;
        m.brightness_min    = 0;
        m.brightness_max    = 255;
        if(effect == 45)
        {
            m.flags         = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
            m.color_mode    = MODE_COLORS_PER_LED;
        }
        else if(effect != 0)
        {
            m.flags |= MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
            if(effect != 24 && effect != 28 && effect != 29 && effect != 32)
            {
                m.flags         |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
                m.color_mode    = MODE_COLORS_MODE_SPECIFIC;
                m.colors_min    = m.colors_max = 1;
                m.colors.push_back(last_color);
            }
        }
        modes.push_back(m);
    }
    active_mode = state.effect;
    SetupZones();
}

RGBController_AkkoMU02::~RGBController_AkkoMU02()
{
    Shutdown();
    delete controller;
}

void RGBController_AkkoMU02::SetupZones()
{
    zone keyboard;
    keyboard.name   = "Whole Keyboard";
    keyboard.type   = ZONE_TYPE_MATRIX;
    unsigned int* matrix = new unsigned int[90];
    for(unsigned int index = 0; index < 90; index++)
    {
        matrix[index] = 0xFFFFFFFF;
        if(mu02_keys[index / 15][index % 15])
        {
            matrix[index] = leds.size();
            led key;
            key.name    = mu02_keys[index / 15][index % 15];
            key.value   = index;
            leds.push_back(key);
        }
    }
    keyboard.leds_min = keyboard.leds_max = keyboard.leds_count = leds.size();
    keyboard.matrix_map.Set(6, 15, matrix);
    zones.push_back(keyboard);
    SetupColors();
    for(unsigned int index = 0; index < leds.size(); index++)
    {
        const AkkoMU02KeyColor& key = controller->GetKeyColors()[leds[index].value];
        hsv_t hsv_color;
        hsv_color.hue           = (key.hue * 360u) / 256u;
        hsv_color.saturation    = key.saturation;
        hsv_color.value         = key.hue == 0 && key.saturation == 0 ? 0 : 255;
        colors[index]           = hsv2rgb(&hsv_color);
    }
    last_key_colors = colors;
    key_color_valid.assign(colors.size(), true);
}

void RGBController_AkkoMU02::DeviceUpdateMode()
{
    if(active_mode < 0 || static_cast<unsigned int>(active_mode) >= modes.size())
    {
        return;
    }
    mode& m = modes[active_mode];
    if(m.value < 0 || m.value > 45 || m.speed > 255 || m.brightness > 255
    || ((m.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR) && m.colors.size() != 1))
    {
        return;
    }
    unsigned char speed         = m.speed;
    unsigned char brightness    = m.brightness;
    unsigned char color[2];
    bool set_color = (m.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR) && m.colors[0] != last_color;
    if(set_color)
    {
        hsv_t hsv_color;
        rgb2hsv(m.colors[0], &hsv_color);
        if(hsv_color.value == 0)
        {
            LOG_WARNING("[%s] Black requires a brightness change; use the All Off effect instead", name.c_str());
            set_color = false;
        }
        else
        {
            color[0] = ((hsv_color.hue * 256u + 180u) / 360u) % 256u;
            color[1] = hsv_color.saturation;
            // RGB value stays separate: brightness is controlled only by the brightness slider.
        }
    }
    const bool set_speed        = (m.flags & MODE_FLAG_HAS_SPEED) && speed != last_speed;
    const bool set_brightness   = (m.flags & MODE_FLAG_HAS_BRIGHTNESS) && brightness != last_brightness;
    if(!controller->Apply(m.value, set_speed ? &speed : nullptr, set_color ? color : nullptr,
                          set_brightness ? &brightness : nullptr))
    {
        LOG_WARNING("[%s] VIA lighting update/readback failed", name.c_str());
        return;
    }
    const AkkoMU02State& actual = controller->GetState();
    for(unsigned int index = 0; index < modes.size(); index++)
    {
        mode& other = modes[index];
        // Global controls must not revert when selecting an untouched mode.
        if(other.speed == last_speed)
        {
            other.speed = actual.speed;
        }
        if(other.brightness == last_brightness)
        {
            other.brightness = actual.brightness;
        }
    }
    last_speed      = actual.speed;
    m.speed         = last_speed;
    last_brightness = actual.brightness;
    m.brightness    = last_brightness;
    if(set_color)
    {
        last_color = m.colors[0];
    }
    if(m.value == 45)
    {
        DeviceUpdateLEDs();
    }
}

void RGBController_AkkoMU02::DeviceSaveMode()
{
    if(active_mode != 45)
    {
        return;
    }
    if(!controller->Apply(45, nullptr, nullptr))
    {
        LOG_WARNING("[%s] Cannot apply palette: Custom mode update/readback failed", name.c_str());
        return;
    }
    DeviceUpdateLEDs();
    for(unsigned int index = 0; index < key_color_valid.size(); index++)
    {
        if(!key_color_valid[index])
        {
            LOG_WARNING("[%s] Palette not applied: a key colour has unverified readback", name.c_str());
            return;
        }
    }
    // Explicit only: the vendor save/apply may persist data and is needed for previously unlit keys.
    if(!controller->SaveKeyColors())
    {
        LOG_WARNING("[%s] Palette save/apply acknowledgement failed", name.c_str());
    }
}

void RGBController_AkkoMU02::DeviceUpdateLEDs()
{
    for(unsigned int index = 0; index < leds.size(); index++)
    {
        DeviceUpdateSingleLED(index);
    }
}

void RGBController_AkkoMU02::DeviceUpdateZoneLEDs(int zone)
{
    if(zone == 0)
    {
        DeviceUpdateLEDs();
    }
}

void RGBController_AkkoMU02::DeviceUpdateSingleLED(int index)
{
    if(active_mode != 45 || controller->GetState().effect != 45 || index < 0
    || static_cast<unsigned int>(index) >= colors.size()
    || (key_color_valid[index] && colors[index] == last_key_colors[index]))
    {
        return;
    }
    hsv_t hsv_color;
    rgb2hsv(colors[index], &hsv_color);
    unsigned char hue           = 0;
    unsigned char saturation    = 0;
    if(hsv_color.value != 0)
    {
        saturation = hsv_color.saturation;
        // H=0,S=0 means off; grayscale hue is undefined, so use H=1 for white.
        hue = saturation == 0 ? 1 : ((hsv_color.hue * 256u + 180u) / 360u) % 256u;
        // shortcut: nonblack per-key RGB value is ignored; use global brightness for dimming.
    }
    const unsigned int position = leds[index].value;
    if(controller->SetKeyColor(position / 15, position % 15, hue, saturation))
    {
        last_key_colors[index]  = colors[index];
        key_color_valid[index]  = true;
    }
    else
    {
        key_color_valid[index] = false; // A failed reply does not prove the SET was unapplied.
        LOG_WARNING("[%s] Per-key colour update/readback failed for %s", name.c_str(), leds[index].name.c_str());
    }
}

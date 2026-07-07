/*---------------------------------------------------------*\
|  RGBController_RoccatSuoraFX.cpp                          |
|                                                           |
|  OpenRGB RGBController for Roccat Suora FX                |
|                                                           |
|  AirPe                                         12 Feb 2026|
|                                                           |
|  This file is part of the OpenRGB project                 |
|  SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#include "RGBController_RoccatSuoraFX.h"
#include <vector>
#include <algorithm>

RGBController_RoccatSuoraFX::RGBController_RoccatSuoraFX(RoccatSuoraFXController* controller_ptr)
{
    controller  = controller_ptr;

    vendor      = "Roccat";
    name        = "Roccat Suora FX";
    description = "Roccat Suora FX Keyboard";
    version     = "1.0";
    location    = controller->GetDeviceLocation();
    type        = DEVICE_TYPE_KEYBOARD;

    SetupZones();
}

RGBController_RoccatSuoraFX::~RGBController_RoccatSuoraFX()
{
    delete controller;
}

void RGBController_RoccatSuoraFX::SetupZones()
{
    /* Key Map based on User Verification */
    const char* key_names[128] = {
        "Key: Left Ctrl",   "Key: Left Shift",  "Key: Tab",         "Key: Caps Lock",   "Key: Tilde",       "Key: Escape",
        "Key: Left Win",    "Key: Z",           "Key: A",           "Key: Q",           "Key: 1",           "Key: Backslash (ISO)",
        "Key: Left Alt",    "Key: X",           "Key: S",           "Key: W",           "Key: 2",           "Key: F1",
        "Key: Unused 18",   "Key: C",           "Key: D",           "Key: E",           "Key: 3",           "Key: F2",
        "Key: Unused 24",   "Key: V",           "Key: F",           "Key: R",           "Key: 4",           "Key: F3",
        "Key: Space",       "Key: B",           "Key: G",           "Key: T",           "Key: 5",           "Key: F4",
        "Key: Unused 36",   "Key: N",           "Key: H",           "Key: Y",           "Key: 6",           "Key: F5",
        "Key: Unused 42",   "Key: M",           "Key: J",           "Key: U",           "Key: 7",           "Key: F6",
        "Key: Unused 48",   "Key: Comma",       "Key: K",           "Key: I",           "Key: 8",           "Key: F7",
        "Key: Right Alt",   "Key: Period",      "Key: L",           "Key: O",           "Key: 9",           "Key: F8",
        "Key: Fn",          "Key: Slash",       "Key: Semicolon",   "Key: P",           "Key: 0",           "Key: F9",
        "Key: Menu",        "Key: Unused 67",   "Key: Quote",       "Key: Left Bracket","Key: Minus",       "Key: F10",
        "Key: Unused 72",   "Key: Right Shift", "Key: Unused 74",   "Key: Right Bracket","Key: Equals",     "Key: F11",
        "Key: Right Ctrl",  "Key: Unused 79",   "Key: Hash (ISO)",  "Key: Enter",       "Key: Backspace",   "Key: F12",
        "Key: Left Arrow",  "Key: Unused 85",   "Key: Unused 86",   "Key: Delete",      "Key: Insert",      "Key: Print Screen",
        "Key: Down Arrow",  "Key: Up Arrow",    "Key: Unused 92",   "Key: End",         "Key: Home",        "Key: Scroll Lock",
        "Key: Right Arrow", "Key: Unused 97",   "Key: Unused 98",   "Key: Page Down",   "Key: Page Up",     "Key: Pause Break",
        "Key: Num 0",       "Key: Num 1",       "Key: Num 4",       "Key: Num 7",       "Key: Num Lock",    "Key: Mute",
        "Key: Unused 108",  "Key: Num 2",       "Key: Num 5",       "Key: Num 8",       "Key: Num Divide",  "Key: Volume Down",
        "Key: Num Decimal", "Key: Num 3",       "Key: Num 6",       "Key: Num 9",       "Key: Num Asterisk","Key: Volume Up",
        "Key: Num Enter",   "Key: Unused 121",  "Key: Num Plus",    "Key: Unused 123",  "Key: Num Minus",   "Key: Game Mode",
        "Key: Unused 126",  "Key: Unused 127"
    };

    /* 1. Populate LEDs list and build hardware-to-logical map */
    std::vector<int> hw_to_logical_map(128, -1);
    
    for(unsigned int i = 0; i < 128; i++)
    {
        /* Filter out Unused keys */
        if(key_names[i] != nullptr && strstr(key_names[i], "Unused") == nullptr)
        {
            led new_led;
            
            /* Clean up "Key: " prefix for cleaner UI */
            std::string name_str = key_names[i];
            size_t prefix_pos = name_str.find("Key: ");
            if(prefix_pos != std::string::npos)
            {
                new_led.name = name_str.substr(prefix_pos + 5);
            }
            else
            {
                new_led.name = name_str;
            }

            new_led.value = i; /* Store hardware index */
            leds.push_back(new_led);
            
            /* Map hardware index to new logical index */
            hw_to_logical_map[i] = leds.size() - 1;
        }
    }
    
    /* 2. Configure Zone */
    zone new_zone;
    new_zone.name       = "Keyboard";
    new_zone.type       = ZONE_TYPE_MATRIX;
    new_zone.leds_count = leds.size(); 
    new_zone.leds_min   = leds.size();
    new_zone.leds_max   = leds.size();
    new_zone.matrix_map = new matrix_map_type;
    new_zone.matrix_map->height = 6;
    new_zone.matrix_map->width  = 22;
    new_zone.matrix_map->map    = new unsigned int[new_zone.matrix_map->height * new_zone.matrix_map->width];

    /* 3. Build Matrix Map using Logical Indices */
    for(unsigned int col = 0; col < 22; col++)
    {
        for(unsigned int row = 0; row < 6; row++)
        {
            unsigned int hw_idx = col * 6 + (5 - row);
            unsigned int logical_idx = 0xFFFFFFFF; // Default to empty
            
            if(hw_idx < 128)
            {
                int mapped = hw_to_logical_map[hw_idx];
                if(mapped != -1)
                {
                    logical_idx = (unsigned int)mapped;
                }
            }
            
            new_zone.matrix_map->map[row * new_zone.matrix_map->width + col] = logical_idx;
        }
    }

    zones.push_back(new_zone);

    /* Direct Mode */
    modes.clear();
    mode direct_mode;
    direct_mode.name       = "Direct";
    direct_mode.value      = ROCCAT_SUORA_FX_EFFECT_DIRECT;
    direct_mode.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE;
    direct_mode.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(direct_mode);

    /* Static / Fully Lit */
    mode static_mode;
    static_mode.name       = "Static";
    static_mode.value      = ROCCAT_SUORA_FX_EFFECT_FULLY_LIT;
    static_mode.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    static_mode.colors_min = 1;
    static_mode.colors_max = 1;
    static_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    static_mode.brightness_min = 0;
    static_mode.brightness_max = 50;
    static_mode.brightness = 50;
    static_mode.colors.resize(1);
    static_mode.colors[0] = 0x00FFFFFF; // White
    modes.push_back(static_mode);

    /* Breathing */
    mode breathing_mode;
    breathing_mode.name       = "Breathing";
    breathing_mode.value      = ROCCAT_SUORA_FX_EFFECT_BREATHING;
    breathing_mode.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    breathing_mode.colors_min = 1;
    breathing_mode.colors_max = 1;
    breathing_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    breathing_mode.speed_min  = 0;
    breathing_mode.speed_max  = 10;
    breathing_mode.speed      = 5;
    breathing_mode.brightness_min = 0;
    breathing_mode.brightness_max = 50;
    breathing_mode.brightness = 50;
    breathing_mode.colors.resize(1);
    breathing_mode.colors[0] = 0x00FF0000; // Red
    modes.push_back(breathing_mode);

    /* Wave */
    mode wave_mode;
    wave_mode.name       = "Wave";
    wave_mode.value      = ROCCAT_SUORA_FX_EFFECT_WAVE;
    wave_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_AUTOMATIC_SAVE;
    wave_mode.color_mode = MODE_COLORS_NONE;
    wave_mode.speed_min  = 0;
    wave_mode.speed_max  = 10;
    wave_mode.speed      = 5;
    wave_mode.brightness_min = 0;
    wave_mode.brightness_max = 50;
    wave_mode.brightness = 50;
    wave_mode.direction  = MODE_DIRECTION_RIGHT;
    modes.push_back(wave_mode);

    /* Fade */
    mode fade_mode;
    fade_mode.name       = "Fade";
    fade_mode.value      = ROCCAT_SUORA_FX_EFFECT_FADE;
    fade_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    fade_mode.color_mode = MODE_COLORS_NONE;
    fade_mode.speed_min  = 0;
    fade_mode.speed_max  = 10;
    fade_mode.speed      = 5;
    fade_mode.brightness_min = 0;
    fade_mode.brightness_max = 50;
    fade_mode.brightness = 50;
    modes.push_back(fade_mode);

    /* Ripple */
    mode ripple_mode;
    ripple_mode.name       = "Ripple";
    ripple_mode.value      = ROCCAT_SUORA_FX_EFFECT_RIPPLE;
    ripple_mode.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    ripple_mode.colors_min = 1;
    ripple_mode.colors_max = 1;
    ripple_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    ripple_mode.speed_min  = 0;
    ripple_mode.speed_max  = 10;
    ripple_mode.speed      = 5;
    ripple_mode.brightness_min = 0;
    ripple_mode.brightness_max = 50;
    ripple_mode.brightness = 50;
    ripple_mode.colors.resize(1);
    ripple_mode.colors[0] = 0x000000FF; // Blue
    modes.push_back(ripple_mode);

    /* Rain */
    mode rain_mode;
    rain_mode.name       = "Rain";
    rain_mode.value      = ROCCAT_SUORA_FX_EFFECT_RAIN;
    rain_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    rain_mode.color_mode = MODE_COLORS_NONE;
    rain_mode.speed_min  = 0;
    rain_mode.speed_max  = 10;
    rain_mode.speed      = 5;
    rain_mode.brightness_min = 0;
    rain_mode.brightness_max = 50;
    rain_mode.brightness = 50;
    modes.push_back(rain_mode);

    SetupColors();
}

void RGBController_RoccatSuoraFX::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /* Not supported */
}

void RGBController_RoccatSuoraFX::DeviceUpdateLEDs()
{
    unsigned char buffer[512]; // 128 keys * 4 bytes (00 R G B)
    std::memset(buffer, 0, sizeof(buffer));

    /* 
       We only iterate through the active/visible LEDs.
       Using leds[i].value guarantees we write to the correct
       hardware index in the buffer.
    */
    for(unsigned int i = 0; i < leds.size(); i++)
    {
        RGBColor color = colors[i];
        unsigned int hw_index = leds[i].value;
        
        if(hw_index < 128)
        {
            buffer[hw_index * 4 + 1] = RGBGetRValue(color);
            buffer[hw_index * 4 + 2] = RGBGetGValue(color);
            buffer[hw_index * 4 + 3] = RGBGetBValue(color);
        }
    }

    /* 
       Optimization: Do NOT call InitializeCustomMode() here.
       It contains a 50ms sleep that causes massive flickering.
       Initialization is now handled in DeviceUpdateMode().
    */
    // controller->InitializeCustomMode(); 
    
    /* 
       Send only the color data. 
       We must call ApplyLighting() to trigger the update.
    */
    controller->SendLightingData(buffer);
    controller->ApplyLighting();
}

void RGBController_RoccatSuoraFX::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoccatSuoraFX::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoccatSuoraFX::DeviceUpdateMode()
{
    mode curr_mode = modes[active_mode];

    if(curr_mode.value == ROCCAT_SUORA_FX_EFFECT_DIRECT)
    {
        /* Initialize hardware for Direct Mode (Unlock + Header) */
        controller->InitializeCustomMode();
        return;
    }

    unsigned char effect     = curr_mode.value;
    
    /* Invert Speed: Hardware is likely delay-based (0=Fast) while OpenRGB is Speed (0=Slow) */
    unsigned char speed      = 10 - curr_mode.speed;
    
    unsigned char brightness = curr_mode.brightness;
    unsigned char color      = ROCCAT_SUORA_FX_COLOR_AQUA; // Default
    unsigned char variant    = ROCCAT_SUORA_FX_VARIANT_NONE;

    /* Map Directions */
    if(curr_mode.direction == MODE_DIRECTION_RIGHT)
    {
        variant = ROCCAT_SUORA_FX_VARIANT_RIGHT;
    }
    else if(curr_mode.direction == MODE_DIRECTION_LEFT)
    {
        variant = ROCCAT_SUORA_FX_VARIANT_LEFT;
    }
    else if(curr_mode.direction == MODE_DIRECTION_UP)
    {
        variant = ROCCAT_SUORA_FX_VARIANT_UP;
    }
    else if(curr_mode.direction == MODE_DIRECTION_DOWN)
    {
        variant = ROCCAT_SUORA_FX_VARIANT_DOWN;
    }

    /* Map Colors */
    if(curr_mode.colors.size() > 0)
    {
        RGBColor rgb = curr_mode.colors[0];
        unsigned char r = RGBGetRValue(rgb);
        unsigned char g = RGBGetGValue(rgb);
        unsigned char b = RGBGetBValue(rgb);

        if(r > 128 && g < 128 && b < 128)      color = ROCCAT_SUORA_FX_COLOR_RED;
        else if(r < 128 && g > 128 && b < 128) color = ROCCAT_SUORA_FX_COLOR_GREEN;
        else if(r < 128 && g < 128 && b > 128) color = ROCCAT_SUORA_FX_COLOR_BLUE;
        else if(r > 128 && g > 128 && b < 128) color = ROCCAT_SUORA_FX_COLOR_YELLOW;
        else if(r < 128 && g > 128 && b > 128) color = ROCCAT_SUORA_FX_COLOR_AQUA;
        else if(r > 128 && g < 128 && b > 128) color = ROCCAT_SUORA_FX_COLOR_PURPLE;
        else if(r > 128 && g > 128 && b > 128) color = ROCCAT_SUORA_FX_COLOR_WHITE;
        else                                   color = ROCCAT_SUORA_FX_COLOR_WHITE;
    }

    controller->SetEffect(effect, speed, brightness, color, variant);
}

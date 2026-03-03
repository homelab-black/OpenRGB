/*---------------------------------------------------------*\
| RGBController_Akko5075BController.cpp                     |
|                                                           |
|   RGBController for Akko 5075B keyboard                   |
|                                                           |
|   Bizzotto Nicola (nicolaa02nb)               05 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AkkoController.h"
#include "KeyboardLayoutManager.h"

/**------------------------------------------------------------------*\
    @name Akko TH80 Pro
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectAkkoControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_AkkoController::RGBController_AkkoController(AkkoController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "Akko";
    type        = DEVICE_TYPE_KEYBOARD;
    description = controller->GetDeviceName();
    serial      = controller->GetSerial();
    location    = controller->GetLocation();

    mode Direct; // Same as Light Shadow;
    Direct.name     = "Direct";
    Direct.value    = AKKO_MODE::LIGHT_SHADOW;
    Direct.flags    = MODE_FLAG_HAS_PER_LED_COLOR;
    modes.push_back(Direct);

    mode Always_on;
    Always_on.name  = "Static";
    Always_on.value = AKKO_MODE::ALWAYS_ON;
    Always_on.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Always_on);

    mode Custom; // Same as User Picture, but with max brightness;
    Custom.name     = "Custom";
    Custom.value    = AKKO_MODE::USER_PICTURE;
    Custom.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_PER_LED_COLOR;
    modes.push_back(Custom);

    mode Breath;
    Breath.name     = "Breathing";
    Breath.value    = AKKO_MODE::BREATH;
    Breath.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Breath);

    mode Meteor;
    Meteor.name     = "Meteor";
    Meteor.value    = AKKO_MODE::METEOR;
    Meteor.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Meteor);

    mode Ripple;
    Ripple.name     = "Ripple";
    Ripple.value    = AKKO_MODE::RIPPLE;
    Ripple.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Ripple);

    mode Aurora;
    Aurora.name     = "Aurora";
    Aurora.value    = AKKO_MODE::AURORA;
    Aurora.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Aurora);

    mode Raindrop;
    Raindrop.name   = "Raindrop";
    Raindrop.value  = AKKO_MODE::RAINDROP;
    Raindrop.flags  = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Raindrop);

    mode Coverage;
    Coverage.name   = "Coverage";
    Coverage.value  = AKKO_MODE::COVERAGE;
    Coverage.flags  = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Coverage);

    mode Steady_stream;
    Steady_stream.name  = "Steady Stream";
    Steady_stream.value = AKKO_MODE::STEADY_STREAM;
    Steady_stream.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Steady_stream);

    mode Neon;
    Neon.name   = "Neon";
    Neon.value  = AKKO_MODE::NEON;
    Neon.flags  = MODE_FLAG_AUTOMATIC_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_RANDOM_COLOR ;
    modes.push_back(Neon);

    mode Wave;
    Wave.name  = "Wave";//Rainbow Wave with customizable color
    Wave.value = AKKO_MODE::WAVE;
    Wave.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    modes.push_back(Wave);

    mode Stars_twinkle;
    Stars_twinkle.name  = "Stars twinkle";
    Stars_twinkle.value = AKKO_MODE::STARS_TWINKLE;
    Stars_twinkle.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Stars_twinkle);

    mode Like_shadows;
    Like_shadows.name   = "Like Shadows";
    Like_shadows.value  = AKKO_MODE::LIKE_SHADOWS;
    Like_shadows.flags  = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Like_shadows);

    mode Sine_wave;
    Sine_wave.name  = "Sine wave";
    Sine_wave.value = AKKO_MODE::SINE_WAVE;
    Sine_wave.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Sine_wave);

    mode Flowing_spring;
    Flowing_spring.name     = "Flowing Spring";
    Flowing_spring.value    = AKKO_MODE::FLOWING_SPRING;
    Flowing_spring.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Flowing_spring);

    mode Flowers_blooming;
    Flowers_blooming.name   = "Flowers blooming";
    Flowers_blooming.value  = AKKO_MODE::FLOWERS_BLOOMING;
    Flowers_blooming.flags  = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Flowers_blooming);

    mode Laser;
    Laser.name  = "Laser";
    Laser.value = AKKO_MODE::LASER;
    Laser.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Laser);

    mode Peak_turn;
    Peak_turn.name  = "Peak turn";
    Peak_turn.value = AKKO_MODE::PEAK_TURN;
    Peak_turn.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Peak_turn);

    mode Inclined_rain;
    Inclined_rain.name  = "Inclined Rain";
    Inclined_rain.value = AKKO_MODE::INCLINED_RAIN;
    Inclined_rain.flags = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Inclined_rain);

    mode Light_trace;
    Light_trace.name    = "Light Trace";
    Light_trace.value   = AKKO_MODE::LIGHT_TRACE;
    Light_trace.flags   = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Light_trace);

    mode Endless;
    Endless.name    = "Endless";
    Endless.value   = AKKO_MODE::ENDLESS;
    Endless.flags   = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Endless);

    // mode User_picture;
    // User_picture.name       = "User Picture";
    // User_picture.value      = AKKO_MODE::USER_PICTURE;
    // User_picture.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    // User_picture.color_mode = MODE_COLORS_PER_LED;
    // modes.push_back(User_picture);

    mode Music_follow;
    Music_follow.name   = "Music Follow (Not Fully implemented)";
    Music_follow.value  = AKKO_MODE::MUSIC_FOLLOW;
    Music_follow.flags  = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    modes.push_back(Music_follow);

    mode Music_follow_2;
    Music_follow_2.name     = "Music Follow 2 (Not Fully implemented)";
    Music_follow_2.value    = AKKO_MODE::MUSIC_FOLLOW_2;
    Music_follow_2.flags    = MODE_FLAG_MANUAL_SAVE | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    modes.push_back(Music_follow_2);

    // mode Light_shadow;
    // Light_shadow.name   = "Light Shadow (Not Fully implemented)";
    // Light_shadow.value  = AKKO_MODE::LIGHT_SHADOW;
    // Light_shadow.flags  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    // modes.push_back(Light_shadow);

    mode Off;
    Off.name        = "Off";
    Off.value       = AKKO_MODE::OFF;
    Off.flags       = 0;
    Off.color_mode  = MODE_COLORS_NONE;
    modes.push_back(Off);


    for(std::vector<mode>::iterator it = modes.begin(); it != modes.end(); it++)
    {
        if (it->flags & MODE_FLAG_HAS_PER_LED_COLOR)
        {
            it->color_mode = MODE_COLORS_PER_LED;
        }
        else if(it->flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
        {
            // it->colors_min = 0; // colors_min is 0 when constructed
            it->colors_max = 1;
            it->colors.resize(1);
            it->color_mode = MODE_COLORS_MODE_SPECIFIC;
        }
        else  if(it->flags & MODE_FLAG_HAS_RANDOM_COLOR)
        {
            it->color_mode = MODE_COLORS_RANDOM;
        }

        if(it->flags & MODE_FLAG_HAS_SPEED)
        {
            // it->speed_min = AKKO_SPEED::S_MIN; // speed_min is 0 when constructed
            it->speed_max = AKKO_SPEED::S_MAX;
            it->speed     = AKKO_SPEED::S_DEFAULT;
        }

        if(it->flags & MODE_FLAG_HAS_BRIGHTNESS)
        {
            // it->brightness_min    = AKKO_BRIGHTNESS::B_MIN; // brightness_min is 0 when constructed
            it->brightness_max    = AKKO_BRIGHTNESS::B_MAX;
            it->brightness        = AKKO_BRIGHTNESS::B_MAX;
        }
    }

    SetupZones();
}

RGBController_AkkoController::~RGBController_AkkoController()
{
    delete controller;
}

void RGBController_AkkoController::SetupZones()
{
    AkkoZoneInfo keyboard_zone_info = controller->GetZoneInfo(AKKO_ZONE::Z_KEYBOARD);
    zones_info.insert({AKKO_ZONE::Z_KEYBOARD, keyboard_zone_info});
    if(controller->HasLightStrips())
    {
        zones_info.insert({AKKO_ZONE::Z_LIGHT_STRIPS, controller->GetZoneInfo(AKKO_ZONE::Z_LIGHT_STRIPS)});
    }

    keyboard_keymap_overlay_values keyboard_layout = controller->GetKeyboardLayoutValues();
    KeyboardLayoutManager new_kb(controller->GetKeyboardLayout(), controller->GetKeyboardSize(), keyboard_layout.key_values);
    new_kb.ChangeKeys(keyboard_layout);

    for(std::vector<mode>::iterator it = modes.begin(); it != modes.end(); it++)
    {
        if (it->value == keyboard_zone_info.mode)
        {
            active_mode = std::distance(modes.begin(), it);

            it->speed      = keyboard_zone_info.speed;
            it->brightness = keyboard_zone_info.brightness;
            it->color_mode = GetColorModeFromFlags(keyboard_zone_info.mode, keyboard_zone_info.flags);
            it->direction  = GetDirectionFromFlags(keyboard_zone_info.flags);
            if (it->color_mode == MODE_COLORS_MODE_SPECIFIC && keyboard_zone_info.colors.size() > 0)
            {
                it->colors[0] = keyboard_zone_info.color;
            }
            break;
        }
    }

    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        const AKKO_ZONE& key    = it->first;
        AkkoZoneInfo& zone_info   = it->second;
        if(key == AKKO_ZONE::Z_KEYBOARD)
        {
            zone_info.leds_count = new_kb.GetKeyCount();

            zone keyboard;
            keyboard.name               = "Keyboard";
            keyboard.type               = ZONE_TYPE_MATRIX;
            keyboard.leds_count         = zone_info.leds_count;
            keyboard.leds_min           = keyboard.leds_count;
            keyboard.leds_max           = keyboard.leds_count;
            keyboard.matrix_map         = new matrix_map_type;
            keyboard.matrix_map->height = new_kb.GetRowCount();
            keyboard.matrix_map->width  = new_kb.GetColumnCount();
            keyboard.matrix_map->map    = new unsigned int[keyboard.matrix_map->height * keyboard.matrix_map->width];
            zones.push_back(keyboard);

            new_kb.GetKeyMap(keyboard.matrix_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT, keyboard.matrix_map->height, keyboard.matrix_map->width);

            for(unsigned int led_idx  = 0; led_idx  < keyboard.leds_count; led_idx ++)
            {
                led keyboard_led;
                keyboard_led.name = new_kb.GetKeyNameAt(led_idx);
                keyboard_led.value = new_kb.GetKeyValueAt(led_idx);
                leds.push_back(keyboard_led);
            }
        }
        else if(key == AKKO_ZONE::Z_LIGHT_STRIPS && controller->HasLightStrips())
        {
            zone_info.leds_count = 1;

            zone light_strips;
            light_strips.name       = "Light Strips";
            light_strips.type       = ZONE_TYPE_SINGLE;
            light_strips.leds_min   = 1;
            light_strips.leds_max   = 1;
            light_strips.leds_count = 1;
            light_strips.leds       = NULL;
            zones.push_back(light_strips);

            led light_strips_led;
            light_strips_led.name = "Light Strips LEDs";
            light_strips_led.value = 0x00;
            leds.push_back(light_strips_led);
        }
    }

    SetupColors();

    size_t led_index = 0;
    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        AkkoZoneInfo& zone_info   = it->second;
        for(int i = 0; i < zone_info.leds_count; i++)
        {
            if(zone_info.mode == AKKO_MODE::USER_PICTURE)
            {
                unsigned int device_idx = leds[led_index + i].value;
                if(i + led_index < colors.size() && device_idx < zone_info.colors.size())
                {
                    colors[i + led_index] = zone_info.colors[device_idx];
                }
            }
            else
            {
                colors[i + led_index] = zone_info.color;
            }
        }

        led_index += zone_info.leds_count;
    }
}

void RGBController_AkkoController::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | Not implemented                                           |
    \*---------------------------------------------------------*/
}

void RGBController_AkkoController::DeviceUpdateLEDs()
{
    size_t led_index = 0;
    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        AkkoZoneInfo& zone_info   = it->second;

        if(zone_info.mode == AKKO_MODE::LIGHT_SHADOW)
        {
            for(std::size_t i = 0; i < zone_info.leds_count; i++)
            {
                unsigned int device_idx = leds[led_index + i].value;

                if(device_idx < zone_info.colors.size())
                {
                    if(zone_info.colors[device_idx] != colors[led_index + i])
                    {
                        zone_info.colors[device_idx] = colors[led_index + i];
                    }
                }
            }
            controller->SetDirectColor(zone_info);
        }
        else if(zone_info.mode == AKKO_MODE::MUSIC_FOLLOW || zone_info.mode == AKKO_MODE::MUSIC_FOLLOW_2)
        {
            controller->SetDirectMusic(zone_info);
        }

        led_index += zone_info.leds_count;
    }
}


void RGBController_AkkoController::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AkkoController::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AkkoController::DeviceUpdateMode()
{
    AKKO_MODE mode = (AKKO_MODE)modes[active_mode].value;
    AKKO_SPEED speed = (AKKO_SPEED)modes[active_mode].speed;
    AKKO_BRIGHTNESS brightness = (AKKO_BRIGHTNESS)modes[active_mode].brightness;
    AKKO_FLAGS flags = GetFlagsForMode(mode, modes[active_mode].direction, modes[active_mode].color_mode);
    RGBColor color = modes[active_mode].colors.size() > 0 ? modes[active_mode].colors[0] : ToRGBColor(0,0,0);

    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        const AKKO_ZONE& key    = it->first;
        AkkoZoneInfo& zone_info   = it->second;
        if (mode == AKKO_MODE::LIGHT_SHADOW && zone_info.mode == AKKO_MODE::LIGHT_SHADOW)
        {
            controller->SetDirectColor(zone_info);
        }
        else
        {
            if (IsZoneCompatibleWith(key, mode))
            {
                zone_info.mode = mode;
                zone_info.flags = flags;
            }
            if (modes[active_mode].brightness & MODE_FLAG_HAS_BRIGHTNESS)
            {
                zone_info.brightness = brightness;
            }
            if (modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
            {
                zone_info.speed = speed;
            }
            if (modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_LR || modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_UD)
            {
                zone_info.flags = GetFlagsForMode(mode, modes[active_mode].direction, modes[active_mode].color_mode);
            }
            zone_info.color = color;
            controller->SetZoneEffect(zone_info);
        }
    }
}

void RGBController_AkkoController::DeviceSaveMode(){
    size_t led_index = 0;
    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        AkkoZoneInfo& zone_info   = it->second;
        if (zone_info.mode == AKKO_MODE::USER_PICTURE) {
            for(std::size_t i = 0; i < zone_info.leds_count; i++)
            {
                unsigned int device_idx = leds[led_index + i].value;

                if(device_idx < zone_info.colors.size())
                {
                    if(zone_info.colors[device_idx] != colors[led_index + i])
                    {
                        zone_info.colors[device_idx] = colors[led_index + i];
                    }
                }
            }
            controller->SetCustomLeds(zone_info);
        } else {
            RGBColor color = colors[led_index];
            zone_info.color = color;
            controller->SetZoneEffect(zone_info);
        }
        led_index += zone_info.leds_count;
    }
}

AKKO_FLAGS RGBController_AkkoController::GetFlagsForMode(AKKO_MODE mode, unsigned int direction, unsigned int random_colors)
{
    bool is_music_follow = mode == AKKO_MODE::MUSIC_FOLLOW || mode == AKKO_MODE::MUSIC_FOLLOW_2;

    AKKO_FLAGS dazzle =  AKKO_FLAGS::DEFAULT;
    switch (random_colors)
    {
        case MODE_COLORS_RANDOM:
            dazzle = is_music_follow ? AKKO_FLAGS::DAZZLE_MUSIC_FOLLOW_ON : AKKO_FLAGS::DAZZLE_ON;
            break;
        case MODE_COLORS_MODE_SPECIFIC:
            dazzle = is_music_follow ? AKKO_FLAGS::DAZZLE_MUSIC_FOLLOW_OFF : AKKO_FLAGS::DAZZLE_OFF;
            break;
        default:
            break;
    }

    AKKO_FLAGS option = AKKO_FLAGS::DEFAULT;
    switch (direction)
    {
        case MODE_DIRECTION_LEFT:
            option = AKKO_FLAGS::DRIFT_LEFT;
            break;
        case MODE_DIRECTION_RIGHT:
            option = AKKO_FLAGS::DRIFT_RIGHT;
            break;
        case MODE_DIRECTION_UP:
            option = is_music_follow ? AKKO_FLAGS::DRIFT_DOWN : AKKO_FLAGS::DRIFT_UP;
            break;
        case MODE_DIRECTION_DOWN:
            option = AKKO_FLAGS::DRIFT_DOWN;
            break;
        default:
            break;
    }

    int combined = option | dazzle;
    return *reinterpret_cast<AKKO_FLAGS*>(&combined); //Casting to suppres warnings
}

unsigned int RGBController_AkkoController::GetColorModeFromFlags(AKKO_MODE mode, AKKO_FLAGS flags)
{
    unsigned int dazzle_flags = flags & 0x0F;
    switch (mode)
    {
        case AKKO_MODE::USER_PICTURE:
        case AKKO_MODE::LIGHT_SHADOW:
            return MODE_COLORS_PER_LED;
            break;
        default:
            break;
    }
    switch (dazzle_flags)
    {
        case AKKO_FLAGS::DAZZLE_ON:
        case AKKO_FLAGS::DAZZLE_MUSIC_FOLLOW_ON:
            return MODE_COLORS_RANDOM;
            break;
        case AKKO_FLAGS::DAZZLE_OFF:
        case AKKO_FLAGS::DAZZLE_MUSIC_FOLLOW_OFF:
            return MODE_COLORS_MODE_SPECIFIC;
            break;
        default:
            return MODE_COLORS_NONE;
            break;
    }
}

unsigned int RGBController_AkkoController::GetDirectionFromFlags(AKKO_FLAGS flags)
{
    unsigned int direction_flags = flags & 0xF0;
    switch (direction_flags)
    {
        case AKKO_FLAGS::DRIFT_RIGHT:
            return MODE_DIRECTION_RIGHT;
            break;
        case AKKO_FLAGS::DRIFT_UP:
            return MODE_DIRECTION_UP;
            break;
        case AKKO_FLAGS::DRIFT_DOWN:
            return MODE_DIRECTION_DOWN;
            break;
        case AKKO_FLAGS::DRIFT_LEFT:
        default:
            return MODE_DIRECTION_LEFT;
            break;
    }
}


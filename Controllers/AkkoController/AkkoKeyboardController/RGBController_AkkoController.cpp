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
#include "RGBControllerKeyNames.h"

#define NA  0xFFFFFFFF

enum AKKO_5075B {
    HEIGHT  = 6,
    WIDTH   = 15,
};

static unsigned int led_matrix_5075_ISO[AKKO_5075B::HEIGHT][AKKO_5075B::WIDTH] = {
    {   0,   6,  12,  18,  24,  30,  36,  42,  48,  54,  60,  66,  72,  78,  NA},
    {   1,   7,  13,  19,  25,  31,  37,  43,  49,  55,  61,  67,  73,  79,  85},
    {   2,   8,  14,  20,  26,  32,  38,  44,  50,  56,  62,  68,  74,  80,  86},
    {   3,   9,  15,  21,  27,  33,  39,  45,  51,  57,  63,  69,  81,  NA,  87},
    {   4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64,  70,  76,  82,  88},
    {   5,  17,  23,  NA,  NA,  NA,  41,  NA,  NA,  53,  59,  71,  77,  83,  89}
};

static const char *led_names_5075_ISO[] = {
    KEY_EN_ESCAPE,
    KEY_EN_BACK_TICK,
    KEY_EN_TAB,
    KEY_EN_CAPS_LOCK,
    KEY_EN_LEFT_SHIFT,
    KEY_EN_LEFT_CONTROL,
    KEY_EN_F1,
    KEY_EN_1,
    KEY_EN_Q,
    KEY_EN_A,
    KEY_NORD_ANGLE_BRACKET,
    KEY_EN_UNUSED,
    KEY_EN_F2,
    KEY_EN_2,
    KEY_EN_W,
    KEY_EN_S,
    KEY_EN_Z,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_F3,
    KEY_EN_3,
    KEY_EN_E,
    KEY_EN_D,
    KEY_EN_X,
    KEY_EN_LEFT_ALT,
    KEY_EN_F4,
    KEY_EN_4,
    KEY_EN_R,
    KEY_EN_F,
    KEY_EN_C,
    KEY_EN_UNUSED,
    KEY_EN_F5,
    KEY_EN_5,
    KEY_EN_T,
    KEY_EN_G,
    KEY_EN_V,
    KEY_EN_UNUSED,
    KEY_EN_F6,
    KEY_EN_6,
    KEY_EN_Y,
    KEY_EN_H,
    KEY_EN_B,
    KEY_EN_SPACE,
    KEY_EN_F7,
    KEY_EN_7,
    KEY_EN_U,
    KEY_EN_J,
    KEY_EN_N,
    KEY_EN_UNUSED,
    KEY_EN_F8,
    KEY_EN_8,
    KEY_EN_I,
    KEY_EN_K,
    KEY_EN_M,
    KEY_EN_RIGHT_ALT,
    KEY_EN_F9,
    KEY_EN_9,
    KEY_EN_O,
    KEY_EN_L,
    KEY_EN_COMMA,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_F10,
    KEY_EN_0,
    KEY_EN_P,
    KEY_EN_SEMICOLON,
    KEY_EN_PERIOD,
    KEY_EN_UNUSED,
    KEY_EN_F11,
    KEY_EN_MINUS,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_QUOTE,
    KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_F12,
    KEY_EN_EQUALS,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_UNUSED,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_LEFT_ARROW,
    KEY_EN_DELETE,
    KEY_EN_BACKSPACE,
    KEY_EN_ISO_ENTER,
    KEY_EN_ISO_BACK_SLASH,
    KEY_EN_UP_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_UNUSED,
    KEY_EN_HOME,
    KEY_EN_PAGE_UP,
    KEY_EN_PAGE_DOWN,
    KEY_EN_END,
    KEY_EN_RIGHT_ARROW,
};

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
    Direct.name         = "Direct";
    Direct.value        = AKKO_MODE::LIGHT_SHADOW;
    Direct.flags        = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    modes.push_back(Direct);

    mode Always_on;
    Always_on.name  = "Static";
    Always_on.value = AKKO_MODE::ALWAYS_ON;
    Always_on.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Always_on);

    mode Custom; // Same as User Picture, but with max brightness;
    Custom.name         = "Custom";
    Custom.value        = AKKO_MODE::USER_PICTURE;
    Custom.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Custom.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    mode Breath;
    Breath.name     = "Breathing";
    Breath.value    = AKKO_MODE::BREATH;
    Breath.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Breath);

    mode Meteor;
    Meteor.name     = "Meteor";
    Meteor.value    = AKKO_MODE::METEOR;
    Meteor.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Meteor);

    mode Ripple;
    Ripple.name     = "Ripple";
    Ripple.value    = AKKO_MODE::RIPPLE;
    Ripple.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Ripple);

    mode Aurora;
    Aurora.name     = "Aurora";
    Aurora.value    = AKKO_MODE::AURORA;
    Aurora.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Aurora);

    mode Raindrop;
    Raindrop.name   = "Raindrop";
    Raindrop.value  = AKKO_MODE::RAINDROP;
    Raindrop.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Raindrop);

    mode Coverage;
    Coverage.name   = "Coverage";
    Coverage.value  = AKKO_MODE::COVERAGE;
    Coverage.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Coverage);

    mode Steady_stream;
    Steady_stream.name  = "Steady Stream";
    Steady_stream.value = AKKO_MODE::STEADY_STREAM;
    Steady_stream.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Steady_stream);

    mode Neon;
    Neon.name   = "Neon";
    Neon.value  = AKKO_MODE::NEON;
    Neon.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_RANDOM_COLOR ;
    modes.push_back(Neon);

    mode Wave;
    Wave.name  = "Wave";//Rainbow Wave with customizable color
    Wave.value = AKKO_MODE::WAVE;
    Wave.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    modes.push_back(Wave);

    mode Stars_twinkle;
    Stars_twinkle.name  = "Stars twinkle";
    Stars_twinkle.value = AKKO_MODE::STARS_TWINKLE;
    Stars_twinkle.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Stars_twinkle);

    mode Like_shadows;
    Like_shadows.name   = "Like Shadows";
    Like_shadows.value  = AKKO_MODE::LIKE_SHADOWS;
    Like_shadows.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Like_shadows);

    mode Sine_wave;
    Sine_wave.name  = "Sine wave";
    Sine_wave.value = AKKO_MODE::SINE_WAVE;
    Sine_wave.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Sine_wave);

    mode Flowing_spring;
    Flowing_spring.name     = "Flowing Spring";
    Flowing_spring.value    = AKKO_MODE::FLOWING_SPRING;
    Flowing_spring.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Flowing_spring);

    mode Flowers_blooming;
    Flowers_blooming.name   = "Flowers blooming";
    Flowers_blooming.value  = AKKO_MODE::FLOWERS_BLOOMING;
    Flowers_blooming.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Flowers_blooming);

    mode Laser;
    Laser.name  = "Laser";
    Laser.value = AKKO_MODE::LASER;
    Laser.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Laser);

    mode Peak_turn;
    Peak_turn.name  = "Peak turn";
    Peak_turn.value = AKKO_MODE::PEAK_TURN;
    Peak_turn.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
    modes.push_back(Peak_turn);

    mode Inclined_rain;
    Inclined_rain.name  = "Inclined Rain";
    Inclined_rain.value = AKKO_MODE::INCLINED_RAIN;
    Inclined_rain.flags = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Inclined_rain);

    mode Light_trace;
    Light_trace.name    = "Light Trace";
    Light_trace.value   = AKKO_MODE::LIGHT_TRACE;
    Light_trace.flags   = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
    modes.push_back(Light_trace);

    mode Endless;
    Endless.name    = "Endless";
    Endless.value   = AKKO_MODE::ENDLESS;
    Endless.flags   = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR;
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
    Music_follow.flags  = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    modes.push_back(Music_follow);

    mode Music_follow_2;
    Music_follow_2.name     = "Music Follow 2 (Not Fully implemented)";
    Music_follow_2.value    = AKKO_MODE::MUSIC_FOLLOW_2;
    Music_follow_2.flags    = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
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
        if(it->flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
        {
            it->colors_min = 1;
            it->colors_max = 1;
            it->colors.resize(1);
            it->color_mode = MODE_COLORS_MODE_SPECIFIC;
        }
        if(it->flags & MODE_FLAG_HAS_SPEED)
        {
            // it->speed_min = AKKO_SPEED::S_MIN;    // speed_min is 0 when constructed
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
    
    for(std::vector<mode>::iterator it = modes.begin(); it != modes.end(); it++)
    {
        if (it->value == keyboard_zone_info.mode)
        {
            active_mode = std::distance(modes.begin(), it);

            it->speed      = AKKO_SPEED::S_MAX - keyboard_zone_info.speed;
            it->brightness = keyboard_zone_info.brightness;
            it->color_mode = GetColorModeFromFlags(keyboard_zone_info.mode, keyboard_zone_info.flags);
            it->direction  = GetDirectionFromFlags(keyboard_zone_info.flags);
            break;
        }
    }

    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        const AKKO_ZONE& key    = it->first;
        AkkoZoneInfo& zone_info   = it->second;
        if(key == AKKO_ZONE::Z_KEYBOARD)
        {
            unsigned int * led_matrix = (unsigned int *)&led_matrix_5075_ISO;
            int m_height = AKKO_5075B::HEIGHT;
            int m_width = AKKO_5075B::WIDTH;
            int n_leds = m_height * m_width;
            zone_info.leds_count = n_leds;

            zone keyboard;
            keyboard.name               = "Keyboard";
            keyboard.type               = ZONE_TYPE_MATRIX;
            keyboard.leds_min           = n_leds;
            keyboard.leds_max           = n_leds;
            keyboard.leds_count         = n_leds;
            keyboard.matrix_map         = new matrix_map_type;
            keyboard.matrix_map->height = m_height;
            keyboard.matrix_map->width  = m_width;
            keyboard.matrix_map->map    = led_matrix;
            zones.push_back(keyboard);

            for(int i = 0; i < n_leds; i++)
            {
                led keyboard_led;
                keyboard_led.name = led_names_5075_ISO[i];
                keyboard_led.value = i;
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
                if(i + led_index < colors.size())
                {
                    colors[i + led_index] = zone_info.colors[i];
                }
                else
                {
                    break;
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

        RGBColor color = colors[led_index];

        if(zone_info.mode == AKKO_MODE::USER_PICTURE)
        {
            bool changed = false;
            for(int i = 0; i < colors.size() && i < zone_info.colors.size(); i++)
            {
                if(!changed && zone_info.colors[i] != colors[led_index + i])
                {
                    changed = true;
                }
                if (changed)
                {
                    zone_info.colors[i] = colors[led_index + i];
                }
            }
            if(changed)
            {
                controller->SetCustomLeds(zone_info);
            }
        }
        else if(zone_info.mode == AKKO_MODE::MUSIC_FOLLOW || zone_info.mode == AKKO_MODE::MUSIC_FOLLOW_2)
        {
            zone_info.color = color;
            controller->SetDirectMusic(zone_info);
            break;
        }
        else if(zone_info.mode == AKKO_MODE::LIGHT_SHADOW)
        {
            zone_info.color = color;
            controller->SetDirectColor(zone_info);
            break;
        }
        else
        {
            if(zone_info.color != color)
            {
                zone_info.color = color;
                controller->SetZoneEffect(zone_info);
            }
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
    RGBColor color = modes[active_mode].colors.size() > 0 ? modes[active_mode].colors[0] : ToRGBColor(0, 0, 0);

    for(std::map<AKKO_ZONE, AkkoZoneInfo>::iterator it = zones_info.begin(); it != zones_info.end(); ++it)
    {
        const AKKO_ZONE& key    = it->first;
        AkkoZoneInfo& zone_info   = it->second;
        if(IsZoneCompatibleWith(key, mode))
        {
            if (mode == AKKO_MODE::LIGHT_SHADOW && zone_info.mode == AKKO_MODE::LIGHT_SHADOW)
            {
                zone_info.color = color;
                controller->SetDirectColor(zone_info);
            }
            else
            {
                zone_info.mode = mode;
                zone_info.speed = speed;
                zone_info.brightness = brightness;
                zone_info.flags = flags;
                zone_info.color = color;
                controller->SetZoneEffect(zone_info);
            }
        }
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
            return MODE_COLORS_PER_LED;
            break;
        case AKKO_MODE::LIGHT_SHADOW:
            return MODE_COLORS_MODE_SPECIFIC;
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
        case AKKO_FLAGS::DRIFT_LEFT:
            return MODE_DIRECTION_LEFT;
            break;
        case AKKO_FLAGS::DRIFT_RIGHT:
            return MODE_DIRECTION_RIGHT;
            break;
        case AKKO_FLAGS::DRIFT_UP:
            return MODE_DIRECTION_UP;
            break;
        case AKKO_FLAGS::DRIFT_DOWN:
            return MODE_DIRECTION_DOWN;
            break;
        default:
            return MODE_DIRECTION_LEFT;
            break;
    }
}


/*---------------------------------------------------------*\
| RGBController_MSIKeyboard1603.cpp                         |
|                                                           |
|   RGBController for MSI Mystic Light MS-1603 keyboards    |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_MSIKeyboard1603.h"

/**------------------------------------------------------------------*\
    @name MSI MS-1603 Mystic Light Keyboard
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :rotating_light:
    @effects :white_check_mark:
    @detectors DetectMSIKeyboard1603Controllers
    @comment Direct mode writes each of the 24 firmware regions separately and is limited to approximately 2 FPS.
\*-------------------------------------------------------------------*/

RGBController_MSIKeyboard1603::RGBController_MSIKeyboard1603
(
    MSIKeyboard1603Controller* controller_ptr
)
{
    controller  = controller_ptr;
    name        = controller->GetDeviceName();
    vendor      = "MSI";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "MSI Mystic Light MS-1603";
    version     = controller->GetFWVersion();
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerial();

    SetupZones();
    SetupModes();
    SetupColors();
}

RGBController_MSIKeyboard1603::~RGBController_MSIKeyboard1603()
{
    Shutdown();
    delete controller;
}

void RGBController_MSIKeyboard1603::SetupZones()
{
    zone_matrix_map_data.fill(0xFFFFFFFFu);

    for(std::size_t zone_idx = 0; zone_idx < MSIKeyboard1603Controller::ZoneCount; zone_idx++)
    {
        led mystic_led;
        mystic_led.name  = "Zone " + std::to_string(zone_idx + 1);
        mystic_led.value = (unsigned int)zone_idx;
        leds.push_back(mystic_led);
    }

    for(std::size_t zone_idx = 0; zone_idx < 11; zone_idx++)
    {
        zone_matrix_map_data[1 + zone_idx] = (unsigned int)zone_idx;
    }

    for(std::size_t zone_idx = 0; zone_idx < 13; zone_idx++)
    {
        zone_matrix_map_data[MatrixWidth + zone_idx] = (unsigned int)(11 + zone_idx);
    }

    zone mystic_zone;
    mystic_zone.name       = "Keyboard";
    mystic_zone.type       = ZONE_TYPE_MATRIX;
    mystic_zone.start_idx  = 0;
    mystic_zone.leds_min   = MSIKeyboard1603Controller::ZoneCount;
    mystic_zone.leds_max   = MSIKeyboard1603Controller::ZoneCount;
    mystic_zone.leds_count = MSIKeyboard1603Controller::ZoneCount;
    mystic_zone.matrix_map.Set(MatrixHeight, MatrixWidth, zone_matrix_map_data.data());
    zones.push_back(mystic_zone);
}

void RGBController_MSIKeyboard1603::DeviceUpdateLEDs()
{
    mode& active = modes[active_mode];

    if(active.value == MSI_1603_MODE_OFF)
    {
        controller->SetOff();
        return;
    }

    if(active.value == MSI_1603_MODE_BREATHING)
    {
        RGBColor breathing_color = 0x00FFFFFF;

        if(!active.colors.empty())
        {
            breathing_color = active.colors[0];
        }

        if(active.color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetRainbowBreathing(active.speed);
            return;
        }

        if(active.colors.size() > 1)
        {
            controller->SetBreathing(ColorFromRGB(breathing_color), ColorFromRGB(active.colors[1]), active.speed);
            return;
        }

        controller->SetBreathing(ColorFromRGB(breathing_color), active.speed);
        return;
    }

    if(active.value == MSI_1603_MODE_COLOR_CYCLE)
    {
        if(active.color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetRainbowColorCycle(active.speed);
            return;
        }

        std::vector<RGBColor> cycle_colors = active.colors;

        while(cycle_colors.size() < 3)
        {
            cycle_colors.push_back(cycle_colors.empty() ? ToRGBColor(0xFF, 0x00, 0xFF) : cycle_colors.back());
        }

        controller->SetColorCycle(
            ColorFromRGB(cycle_colors[0]),
            ColorFromRGB(cycle_colors[1]),
            ColorFromRGB(cycle_colors[2]),
            active.speed);
        return;
    }

    if(active.value == MSI_1603_MODE_BOUNCY_WALL)
    {
        if(active.color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetRainbowBouncyWall();
            return;
        }

        RGBColor wall_color = 0x0000FFFF;

        if(!active.colors.empty())
        {
            wall_color = active.colors[0];
        }

        controller->SetBouncyWall(ColorFromRGB(wall_color));
        return;
    }

    if(active.value == MSI_1603_MODE_WAVE)
    {
        if(active.color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetRainbowWave(active.speed, active.direction);
            return;
        }

        std::vector<RGBColor> wave_colors = active.colors;

        while(wave_colors.size() < 5)
        {
            wave_colors.push_back(wave_colors.empty() ? ToRGBColor(0xFF, 0x00, 0x00) : wave_colors.back());
        }

        std::array<MSI1603Color, 5> device_wave_colors =
        {{
            ColorFromRGB(wave_colors[0]),
            ColorFromRGB(wave_colors[1]),
            ColorFromRGB(wave_colors[2]),
            ColorFromRGB(wave_colors[3]),
            ColorFromRGB(wave_colors[4]),
        }};

        controller->SetWave(device_wave_colors, active.speed, active.direction);
        return;
    }

    if(active.value == MSI_1603_MODE_DANCE)
    {
        controller->SetDance(active.speed);
        return;
    }

    if(active.value == MSI_1603_MODE_WHEEL)
    {
        if(active.color_mode == MODE_COLORS_RANDOM)
        {
            controller->SetRainbowWheel(active.speed, active.direction);
            return;
        }

        std::vector<RGBColor> wheel_colors = active.colors;

        while(wheel_colors.size() < 3)
        {
            wheel_colors.push_back(wheel_colors.empty() ? ToRGBColor(0xFF, 0x00, 0x00) : wheel_colors.back());
        }

        controller->SetWheel(
            ColorFromRGB(wheel_colors[0]),
            ColorFromRGB(wheel_colors[1]),
            ColorFromRGB(wheel_colors[2]),
            active.speed,
            active.direction);
        return;
    }

    std::vector<MSI1603Color> zone_colors;
    zone_colors.reserve(colors.size());

    for(const RGBColor color : colors)
    {
        zone_colors.push_back(ColorFromRGB(color));
    }

    controller->SetStatic(zone_colors);
}

void RGBController_MSIKeyboard1603::DeviceUpdateZoneLEDs(int zone)
{
    if((zone < 0) || ((std::size_t)zone >= zones.size()))
    {
        return;
    }

    if(modes[active_mode].value == MSI_1603_MODE_OFF)
    {
        controller->SetOff();
        return;
    }

    if(modes[active_mode].value != MSI_1603_MODE_STATIC)
    {
        DeviceUpdateMode();
        return;
    }

    if(zones[zone].type != ZONE_TYPE_SINGLE)
    {
        DeviceUpdateLEDs();
        return;
    }

    controller->SetStatic(ColorFromRGB(zones[zone].colors[0]), (std::size_t)zone);
}

void RGBController_MSIKeyboard1603::DeviceUpdateSingleLED(int led)
{
    if((led < 0) || ((std::size_t)led >= colors.size()))
    {
        return;
    }

    if(modes[active_mode].value == MSI_1603_MODE_OFF)
    {
        controller->SetOff();
        return;
    }

    if(modes[active_mode].value != MSI_1603_MODE_STATIC)
    {
        DeviceUpdateMode();
        return;
    }

    controller->SetStatic(ColorFromRGB(colors[led]), (std::size_t)led);
}

void RGBController_MSIKeyboard1603::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

void RGBController_MSIKeyboard1603::DeviceSaveMode()
{
    controller->Save();
}

void RGBController_MSIKeyboard1603::SetupModes()
{
    mode off_mode;
    off_mode.name       = "Off";
    off_mode.value      = MSI_1603_MODE_OFF;
    off_mode.flags      = MODE_FLAG_MANUAL_SAVE;
    off_mode.color_mode = MODE_COLORS_NONE;
    off_mode.colors_min = 0;
    off_mode.colors_max = 0;
    modes.push_back(off_mode);

    mode static_mode;
    static_mode.name       = "Static";
    static_mode.value      = MSI_1603_MODE_STATIC;
    static_mode.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_MANUAL_SAVE;
    static_mode.color_mode = MODE_COLORS_PER_LED;
    static_mode.colors_min = 0;
    static_mode.colors_max = 0;
    modes.push_back(static_mode);

    mode breathing_mode;
    breathing_mode.name       = "Breathing";
    breathing_mode.value      = MSI_1603_MODE_BREATHING;
    breathing_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    breathing_mode.speed_min  = 0;
    breathing_mode.speed_max  = 2;
    breathing_mode.speed      = 1;
    breathing_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    breathing_mode.colors_min = 1;
    breathing_mode.colors_max = 2;
    breathing_mode.colors.push_back(ToRGBColor(0xFF, 0xFF, 0xFF));
    breathing_mode.colors.push_back(ToRGBColor(0xFF, 0xFF, 0xFF));
    modes.push_back(breathing_mode);

    mode color_cycle_mode;
    color_cycle_mode.name       = "Color Cycle";
    color_cycle_mode.value      = MSI_1603_MODE_COLOR_CYCLE;
    color_cycle_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    color_cycle_mode.speed_min  = 0;
    color_cycle_mode.speed_max  = 2;
    color_cycle_mode.speed      = 1;
    color_cycle_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    color_cycle_mode.colors_min = 3;
    color_cycle_mode.colors_max = 3;
    color_cycle_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0xFF));
    color_cycle_mode.colors.push_back(ToRGBColor(0xFF, 0xFF, 0x00));
    color_cycle_mode.colors.push_back(ToRGBColor(0x00, 0xFF, 0xFF));
    modes.push_back(color_cycle_mode);

    mode bouncy_wall_mode;
    bouncy_wall_mode.name       = "Bouncy Wall";
    bouncy_wall_mode.value      = MSI_1603_MODE_BOUNCY_WALL;
    bouncy_wall_mode.flags      = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    bouncy_wall_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    bouncy_wall_mode.colors_min = 1;
    bouncy_wall_mode.colors_max = 1;
    bouncy_wall_mode.colors.push_back(ToRGBColor(0x00, 0xFF, 0xFF));
    modes.push_back(bouncy_wall_mode);

    mode wave_mode;
    wave_mode.name       = "Wave";
    wave_mode.value      = MSI_1603_MODE_WAVE;
    wave_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    wave_mode.speed_min  = 0;
    wave_mode.speed_max  = 2;
    wave_mode.speed      = 1;
    wave_mode.direction  = MODE_DIRECTION_RIGHT;
    wave_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    wave_mode.colors_min = 5;
    wave_mode.colors_max = 5;
    wave_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0x00));
    wave_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0x40));
    wave_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0x80));
    wave_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0xBF));
    wave_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0xFF));
    modes.push_back(wave_mode);

    mode dance_mode;
    dance_mode.name       = "Dance";
    dance_mode.value      = MSI_1603_MODE_DANCE;
    dance_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_MANUAL_SAVE;
    dance_mode.speed_min  = 0;
    dance_mode.speed_max  = 2;
    dance_mode.speed      = 1;
    dance_mode.color_mode = MODE_COLORS_NONE;
    dance_mode.colors_min = 0;
    dance_mode.colors_max = 0;
    modes.push_back(dance_mode);

    mode wheel_mode;
    wheel_mode.name       = "Wheel";
    wheel_mode.value      = MSI_1603_MODE_WHEEL;
    wheel_mode.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_MANUAL_SAVE;
    wheel_mode.speed_min  = 0;
    wheel_mode.speed_max  = 2;
    wheel_mode.speed      = 1;
    wheel_mode.direction  = MODE_DIRECTION_LEFT;
    wheel_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    wheel_mode.colors_min = 3;
    wheel_mode.colors_max = 3;
    wheel_mode.colors.push_back(ToRGBColor(0xFF, 0x00, 0x00));
    wheel_mode.colors.push_back(ToRGBColor(0xFF, 0xFF, 0x00));
    wheel_mode.colors.push_back(ToRGBColor(0x00, 0xFF, 0xFF));
    modes.push_back(wheel_mode);
}

MSI1603Color RGBController_MSIKeyboard1603::ColorFromRGB(RGBColor color) const
{
    MSI1603Color device_color;

    device_color.R = RGBGetRValue(color);
    device_color.G = RGBGetGValue(color);
    device_color.B = RGBGetBValue(color);

    return(device_color);
}

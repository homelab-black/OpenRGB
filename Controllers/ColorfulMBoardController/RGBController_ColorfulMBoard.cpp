/*---------------------------------------------------------*\
| RGBController_ColorfulMBoard.cpp                         |
|                                                           |
|   RGBController for Colorful USB motherboard lighting     |
|                                                           |
|   OpenRGB contributors                       08 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_ColorfulMBoard.h"

/*---------------------------------------------------------*\
| Logical outputs from the 1020 vendor profile.  Wire slots |
| stay fixed when a zone is resized.  Slots 480 and 481 are |
| reserved for the profile's two 12V outputs.               |
\*---------------------------------------------------------*/
struct ColorfulMBoardZone
{
    const char*     name;
    unsigned int    first_slot;
    unsigned int    capacity;
};

static const ColorfulMBoardZone colorful_mboard_zones[] =
{
    { "5V_1",   0, 100 },
    { "5V_2", 100, 100 },
    { "5V_3", 200, 100 },
    { "5V_4", 300, 100 },
    { "5V_5", 400,  40 },
    { "5V_6", 440,  40 },
};

/**------------------------------------------------------------------*\
    @name Colorful Motherboard
    @category Motherboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectColorfulMBoardControllers
    @comment Native modes run in the controller after one command report.
\*-------------------------------------------------------------------*/

RGBController_ColorfulMBoard::RGBController_ColorfulMBoard(ColorfulMBoardController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "Colorful";
    type        = DEVICE_TYPE_MOTHERBOARD;
    description = "Colorful USB motherboard lighting";
    location    = controller->GetDeviceLocation();

    mode direct;
    direct.name                     = "Direct";
    direct.value                    = COLORFUL_MBOARD_MODE_DIRECT;
    direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR;
    direct.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(direct);

    mode breathing;
    breathing.name                  = "Breathing";
    breathing.value                 = COLORFUL_MBOARD_MODE_BREATHING;
    breathing.flags                 = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    breathing.colors_min            = 1;
    breathing.colors_max            = 1;
    breathing.colors.resize(1);
    breathing.colors[0]             = ToRGBColor(0xFF, 0x00, 0x00);
    breathing.speed_min             = COLORFUL_MBOARD_MODE_SPEED_MIN;
    breathing.speed_max             = COLORFUL_MBOARD_MODE_SPEED_MAX;
    breathing.speed                 = 3;
    modes.push_back(breathing);

    mode color_cycle;
    color_cycle.name                = "Color Cycle";
    color_cycle.value               = COLORFUL_MBOARD_MODE_COLOR_CYCLE;
    color_cycle.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    color_cycle.color_mode          = MODE_COLORS_NONE;
    color_cycle.speed_min           = COLORFUL_MBOARD_MODE_SPEED_MIN;
    color_cycle.speed_max           = COLORFUL_MBOARD_MODE_SPEED_MAX;
    color_cycle.speed               = 3;
    color_cycle.brightness_min      = 0;
    color_cycle.brightness_max      = 100;
    color_cycle.brightness          = 100;
    modes.push_back(color_cycle);

    mode rainbow;
    rainbow.name                    = "Rainbow";
    rainbow.value                   = COLORFUL_MBOARD_MODE_RAINBOW;
    rainbow.flags                   = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    rainbow.color_mode              = MODE_COLORS_NONE;
    rainbow.speed_min               = COLORFUL_MBOARD_MODE_SPEED_MIN;
    rainbow.speed_max               = COLORFUL_MBOARD_MODE_SPEED_MAX;
    rainbow.speed                   = 3;
    rainbow.brightness_min          = 0;
    rainbow.brightness_max          = 100;
    rainbow.brightness              = 100;
    modes.push_back(rainbow);

    mode off;
    off.name                        = "Off";
    off.value                       = COLORFUL_MBOARD_MODE_OFF;
    off.flags                       = MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    off.color_mode                  = MODE_COLORS_NONE;
    modes.push_back(off);

    SetupZones();
}

RGBController_ColorfulMBoard::~RGBController_ColorfulMBoard()
{
    Shutdown();

    delete controller;
}

void RGBController_ColorfulMBoard::SetupZones()
{
    /*-----------------------------------------------------*\
    | Preserve colours by wire slot, not compact LED index.|
    | Resizing an earlier zone shifts later UI indices.    |
    \*-----------------------------------------------------*/
    RGBColor previous_colors[COLORFUL_MBOARD_LED_COUNT] = {};

    for(std::size_t led_idx = 0; led_idx < leds.size() && led_idx < colors.size(); led_idx++)
    {
        if(leds[led_idx].value < COLORFUL_MBOARD_LED_COUNT)
        {
            previous_colors[leds[led_idx].value] = colors[led_idx];
        }
    }

    leds.clear();
    colors.clear();
    zones.resize(sizeof(colorful_mboard_zones) / sizeof(colorful_mboard_zones[0]));

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        const ColorfulMBoardZone& output = colorful_mboard_zones[zone_idx];

        zones[zone_idx].name     = output.name;
        zones[zone_idx].type     = ZONE_TYPE_LINEAR;
        zones[zone_idx].leds_min = 0;
        zones[zone_idx].leds_max = output.capacity;
        zones[zone_idx].flags   |= ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME;

        if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
        {
            zones[zone_idx].leds_count = 0;
        }
        else if(zones[zone_idx].leds_count > output.capacity)
        {
            zones[zone_idx].leds_count = output.capacity;
        }

        for(unsigned int led_idx = 0; led_idx < zones[zone_idx].leds_count; led_idx++)
        {
            led new_led;
            new_led.name  = zones[zone_idx].name + ", LED " + std::to_string(led_idx + 1);
            new_led.value = output.first_slot + led_idx;
            leds.push_back(new_led);
        }
    }

    SetupColors();

    for(std::size_t led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        colors[led_idx] = previous_colors[leds[led_idx].value];
    }
}

void RGBController_ColorfulMBoard::DeviceConfigureZone(int zone_idx)
{
    if((std::size_t)zone_idx < zones.size())
    {
        SetupZones();

        /*-------------------------------------------------*\
        | Clear removed LEDs on the next complete frame.    |
        | The Direct guard preserves a running native mode. |
        \*-------------------------------------------------*/
        DeviceUpdateLEDs();
    }
}

void RGBController_ColorfulMBoard::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | Only Direct mode drives the LEDs from the colour list.|
    | Stray updates while a native mode is active must not  |
    | stop that mode.                                       |
    \*-----------------------------------------------------*/
    if(modes[active_mode].value != COLORFUL_MBOARD_MODE_DIRECT)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Expand configured LEDs into their fixed wire slots.  |
    | Disabled outputs and unused positions remain black.  |
    \*-----------------------------------------------------*/
    std::vector<RGBColor> frame(COLORFUL_MBOARD_LED_COUNT, ToRGBColor(0, 0, 0));

    for(std::size_t led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        frame[leds[led_idx].value] = colors[led_idx];
    }

    controller->SetLEDs(frame);
}

void RGBController_ColorfulMBoard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ColorfulMBoard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ColorfulMBoard::DeviceUpdateMode()
{
    const mode& selected_mode = modes[active_mode];
    unsigned char brightness  = (unsigned char)((selected_mode.brightness * 255) / 100);

    switch(selected_mode.value)
    {
        case COLORFUL_MBOARD_MODE_DIRECT:
            DeviceUpdateLEDs();
            break;

        case COLORFUL_MBOARD_MODE_BREATHING:
            controller->SetBreathe(selected_mode.colors[0], (unsigned char)selected_mode.speed);
            break;

        case COLORFUL_MBOARD_MODE_COLOR_CYCLE:
            controller->SetCircle(brightness, (unsigned char)selected_mode.speed);
            break;

        case COLORFUL_MBOARD_MODE_RAINBOW:
            controller->SetRainbow(brightness, (unsigned char)selected_mode.speed);
            break;

        case COLORFUL_MBOARD_MODE_OFF:
            controller->SetSleep();
            break;
    }
}

/*---------------------------------------------------------*\
| RGBController_AsusLegacyACPI_Windows.cpp                  |
|                                                           |
|   RGBController for legacy ASUS motherboard ACPI          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AsusLegacyACPI_Windows.h"

#define ASUS_LEGACY_ACPI_BRIGHTNESS_MIN               0
#define ASUS_LEGACY_ACPI_BRIGHTNESS_MAX               100

RGBController_AsusLegacyACPI_Windows::RGBController_AsusLegacyACPI_Windows(AsusLegacyACPIController_Windows *controller_ptr)
{
    controller  = controller_ptr;
    name        = "ASUS Maximus VIII Formula Lighting";
    vendor      = "ASUS";
    description = "Legacy ACPI Device";
    location    = "\\\\.\\ATKACPI";
    type        = DEVICE_TYPE_MOTHERBOARD;

    mode Static;
    Static.name           = "Static";
    Static.value          = ASUS_LEGACY_ACPI_MODE_DIRECT;
    Static.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode     = MODE_COLORS_PER_LED;
    Static.brightness_min = ASUS_LEGACY_ACPI_BRIGHTNESS_MIN;
    Static.brightness_max = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Static.brightness     = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = ASUS_LEGACY_ACPI_MODE_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Breathing.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min     = 1;
    Breathing.colors_max     = 1;
    Breathing.brightness_min = ASUS_LEGACY_ACPI_BRIGHTNESS_MIN;
    Breathing.brightness_max = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Breathing.brightness     = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    mode Strobe;
    Strobe.name           = "Strobe";
    Strobe.value          = ASUS_LEGACY_ACPI_MODE_STROBE;
    Strobe.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Strobe.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Strobe.colors_min     = 1;
    Strobe.colors_max     = 1;
    Strobe.brightness_min = ASUS_LEGACY_ACPI_BRIGHTNESS_MIN;
    Strobe.brightness_max = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Strobe.brightness     = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Strobe.colors.resize(1);
    modes.push_back(Strobe);

    mode Rainbow;
    Rainbow.name           = "Rainbow";
    Rainbow.value          = ASUS_LEGACY_ACPI_MODE_RAINBOW;
    Rainbow.flags          = MODE_FLAG_HAS_BRIGHTNESS;
    Rainbow.color_mode     = MODE_COLORS_NONE;
    Rainbow.brightness_min = ASUS_LEGACY_ACPI_BRIGHTNESS_MIN;
    Rainbow.brightness_max = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    Rainbow.brightness     = ASUS_LEGACY_ACPI_BRIGHTNESS_MAX;
    modes.push_back(Rainbow);

    mode Off;
    Off.name       = "Off";
    Off.value      = ASUS_LEGACY_ACPI_MODE_OFF;
    Off.flags      = 0;
    Off.color_mode = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();

    RGBColor current_color = 0;
    if(controller->GetColor(&current_color))
    {
        colors[0] = current_color;
        modes[1].colors[0] = current_color;
        modes[2].colors[0] = current_color;
    }

    unsigned char current_mode = ASUS_LEGACY_ACPI_MODE_STATIC;
    if(controller->GetMode(&current_mode)
    && current_mode >= ASUS_LEGACY_ACPI_MODE_STATIC
    && current_mode <= ASUS_LEGACY_ACPI_MODE_RAINBOW)
    {
        active_mode = current_mode == ASUS_LEGACY_ACPI_MODE_STATIC
                    ? 0
                    : current_mode - ASUS_LEGACY_ACPI_MODE_STATIC;
    }
}

RGBController_AsusLegacyACPI_Windows::~RGBController_AsusLegacyACPI_Windows()
{
    Shutdown();
    delete controller;
}

void RGBController_AsusLegacyACPI_Windows::SetupZones()
{
    zone z;
    z.name       = "Motherboard";
    z.type       = ZONE_TYPE_SINGLE;
    z.leds_min   = 1;
    z.leds_max   = 1;
    z.leds_count = 1;
    zones.push_back(z);

    led l;
    l.name = "Motherboard LEDs";
    leds.push_back(l);

    SetupColors();
}

void RGBController_AsusLegacyACPI_Windows::DeviceUpdateLEDs()
{
    active_mode = 0;
    controller->SetDirectColor(ApplyBrightness(colors[0]));
}

void RGBController_AsusLegacyACPI_Windows::DeviceUpdateZoneLEDs(int /*zone*/)
{
    active_mode = 0;
    controller->SetDirectColor(ApplyBrightness(colors[0]));
}

void RGBController_AsusLegacyACPI_Windows::DeviceUpdateSingleLED(int /*led*/)
{
    active_mode = 0;
    controller->SetDirectColor(ApplyBrightness(colors[0]));
}

void RGBController_AsusLegacyACPI_Windows::DeviceUpdateMode()
{
    mode set_mode = modes[active_mode];

    if(set_mode.value == ASUS_LEGACY_ACPI_MODE_DIRECT)
    {
        controller->SetDirectColor(ApplyBrightness(GetActiveColor()));
    }
    else if(set_mode.value == ASUS_LEGACY_ACPI_MODE_OFF)
    {
        controller->SetOff();
    }
    else
    {
        RGBColor color = GetActiveColor();
        if(set_mode.value == ASUS_LEGACY_ACPI_MODE_RAINBOW)
        {
            color = 0;
        }

        controller->SetMode((unsigned char)set_mode.value, ApplyBrightness(color));
    }
}

RGBColor RGBController_AsusLegacyACPI_Windows::ApplyBrightness(RGBColor color) const
{
    const unsigned int brightness = modes[active_mode].brightness;
    const unsigned char red   = (unsigned char)((RGBGetRValue(color) * brightness + 50) / 100);
    const unsigned char green = (unsigned char)((RGBGetGValue(color) * brightness + 50) / 100);
    const unsigned char blue  = (unsigned char)((RGBGetBValue(color) * brightness + 50) / 100);
    return ToRGBColor(red, green, blue);
}

RGBColor RGBController_AsusLegacyACPI_Windows::GetActiveColor() const
{
    if(active_mode < modes.size() && !modes[active_mode].colors.empty())
    {
        return modes[active_mode].colors[0];
    }

    return colors.empty() ? 0 : colors[0];
}

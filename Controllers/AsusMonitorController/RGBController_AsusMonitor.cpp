/*---------------------------------------------------------*\
| RGBController_AsusMonitor.cpp                             |
|                                                           |
|   RGBController for Asus monitors                         |
|                                                           |
|   Morgan Guimard (morg)                       19 oct 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AsusMonitor.h"

/**------------------------------------------------------------------*\
    @name Asus monitors
    @category Monitor
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectAsusMonitorControllers
    @comment ALB01 front white lamp is a second zone. RGB maps to brightness (peak channel) and CCT (red=warm, blue=cool).
\*-------------------------------------------------------------------*/

RGBController_AsusMonitor::RGBController_AsusMonitor(AsusMonitorController* controller_ptr)
{
    controller                          = controller_ptr;
    name                                = controller->GetNameString();
    vendor                              = "ASUS";
    type                                = DEVICE_TYPE_MONITOR;
    description                         = "ASUS monitor";
    location                            = controller->GetDeviceLocation();
    serial                              = controller->GetSerialString();
    number_of_leds                      = controller->GetNumberOfLEDs();

    controller->SendInit();

    mode Direct;
    Direct.name                         = "Direct";
    Direct.flags                        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode                   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_AsusMonitor::~RGBController_AsusMonitor()
{
    Shutdown();

    delete controller;
}

void RGBController_AsusMonitor::SetupZones()
{
    zone rgb_zone;

    rgb_zone.name       = controller->HasFrontWhite() ? "Rear RGB" : "Monitor";
    rgb_zone.type       = ZONE_TYPE_LINEAR;
    rgb_zone.leds_min   = number_of_leds;
    rgb_zone.leds_max   = number_of_leds;
    rgb_zone.leds_count = number_of_leds;

    zones.emplace_back(rgb_zone);

    unsigned int total_leds = number_of_leds;

    if(controller->HasFrontWhite())
    {
        zone white_zone;

        white_zone.name       = "Front Light";
        white_zone.type       = ZONE_TYPE_SINGLE;
        white_zone.leds_min   = 1;
        white_zone.leds_max   = 1;
        white_zone.leds_count = 1;

        zones.emplace_back(white_zone);
        total_leds += 1;
    }

    leds.resize(total_leds);

    for(unsigned int i = 0; i < number_of_leds; i++)
    {
        leds[i].name = "LED " + std::to_string(i);
    }

    if(controller->HasFrontWhite())
    {
        leds[number_of_leds].name = "Front";
    }

    SetupColors();
}

void RGBController_AsusMonitor::DeviceUpdateLEDs()
{
    if(controller->HasFrontWhite() && colors.size() > number_of_leds)
    {
        std::vector<RGBColor> rgb_colors;

        for(unsigned int i = 0; i < number_of_leds; i++)
        {
            rgb_colors.push_back(colors[i]);
        }

        controller->SetDirect(rgb_colors);
        controller->SetFrontWhite(colors[number_of_leds]);
    }
    else
    {
        controller->SetDirect(colors);
    }
}

void RGBController_AsusMonitor::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AsusMonitor::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AsusMonitor::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

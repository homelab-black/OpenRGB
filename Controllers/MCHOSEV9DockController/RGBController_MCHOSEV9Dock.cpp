/*---------------------------------------------------------*\
| RGBController_MCHOSEV9Dock.cpp                            |
|                                                           |
|   RGBController for MCHOSE V9 charging dock               |
|                                                           |
|   Michael Mano <michael.mano26@gmail.com>     24 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_MCHOSEV9Dock.h"

using namespace std::chrono_literals;

/**------------------------------------------------------------------*\
    @name MCHOSE V9 Dock
    @category HeadsetStand
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectMCHOSEV9DockControllers
    @comment Charging dock for the MCHOSE V9 Turbo and V9 Turbo+
        headsets.
\*-------------------------------------------------------------------*/

RGBController_MCHOSEV9Dock::RGBController_MCHOSEV9Dock(MCHOSEV9DockController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "MCHOSE";
    type                            = DEVICE_TYPE_HEADSET_STAND;
    description                     = "MCHOSE V9 Charging Dock";
    version                         = controller->GetFirmwareVersion();
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = MCHOSE_V9_DOCK_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Static;
    Static.name                     = "Static";
    Static.value                    = MCHOSE_V9_DOCK_MODE_STATIC;
    Static.flags                    = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.brightness_min           = MCHOSE_V9_DOCK_BRIGHTNESS_MIN;
    Static.brightness_max           = MCHOSE_V9_DOCK_BRIGHTNESS_MAX;
    Static.brightness               = MCHOSE_V9_DOCK_BRIGHTNESS_MAX;
    Static.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min               = 1;
    Static.colors_max               = 1;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.value                 = MCHOSE_V9_DOCK_MODE_BREATHING;
    Breathing.flags                 = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED;
    Breathing.speed_min             = MCHOSE_V9_DOCK_SPEED_MIN;
    Breathing.speed_max             = MCHOSE_V9_DOCK_SPEED_MAX;
    Breathing.speed                 = MCHOSE_V9_DOCK_SPEED_DEFAULT;
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    mode SpectrumCycle;
    SpectrumCycle.name              = "Spectrum Cycle";
    SpectrumCycle.value             = MCHOSE_V9_DOCK_MODE_COLOR_CYCLE;
    SpectrumCycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    SpectrumCycle.speed_min         = MCHOSE_V9_DOCK_SPEED_MIN;
    SpectrumCycle.speed_max         = MCHOSE_V9_DOCK_SPEED_MAX;
    SpectrumCycle.speed             = MCHOSE_V9_DOCK_SPEED_DEFAULT;
    SpectrumCycle.brightness_min    = MCHOSE_V9_DOCK_BRIGHTNESS_MIN;
    SpectrumCycle.brightness_max    = MCHOSE_V9_DOCK_BRIGHTNESS_MAX;
    SpectrumCycle.brightness        = MCHOSE_V9_DOCK_BRIGHTNESS_MAX;
    SpectrumCycle.color_mode        = MODE_COLORS_NONE;
    modes.push_back(SpectrumCycle);

    mode Off;
    Off.name                        = "Off";
    Off.value                       = MCHOSE_V9_DOCK_MODE_STATIC;
    Off.flags                       = 0;
    Off.color_mode                  = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();

    /*-----------------------------------------------------*\
    | The dock only shows Direct colors while they are      |
    | being streamed, so keep resending them                |
    \*-----------------------------------------------------*/
    last_update_time                = std::chrono::steady_clock::now();
    keepalive_thread_run            = 1;
    keepalive_thread                = new std::thread(&RGBController_MCHOSEV9Dock::KeepaliveThread, this);
}

RGBController_MCHOSEV9Dock::~RGBController_MCHOSEV9Dock()
{
    Shutdown();

    keepalive_thread_run = 0;
    keepalive_thread->join();
    delete keepalive_thread;

    delete controller;
}

void RGBController_MCHOSEV9Dock::SetupZones()
{
    zone dock_zone;

    dock_zone.name                  = "Dock";
    dock_zone.type                  = ZONE_TYPE_LINEAR;
    dock_zone.leds_min              = MCHOSE_V9_DOCK_LED_COUNT;
    dock_zone.leds_max              = MCHOSE_V9_DOCK_LED_COUNT;
    dock_zone.leds_count            = MCHOSE_V9_DOCK_LED_COUNT;
    zones.push_back(dock_zone);

    for(unsigned int led_idx = 0; led_idx < MCHOSE_V9_DOCK_LED_COUNT; led_idx++)
    {
        led dock_led;
        dock_led.name               = "Dock LED " + std::to_string(led_idx);
        leds.push_back(dock_led);
    }

    SetupColors();
}

void RGBController_MCHOSEV9Dock::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == MCHOSE_V9_DOCK_MODE_DIRECT)
    {
        last_update_time = std::chrono::steady_clock::now();
        controller->SetLEDs(colors);
    }
}

void RGBController_MCHOSEV9Dock::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MCHOSEV9Dock::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MCHOSEV9Dock::DeviceUpdateMode()
{
    bool            enable      = (modes[active_mode].name != "Off");
    unsigned char   speed       = 0;
    unsigned char   brightness  = MCHOSE_V9_DOCK_BRIGHTNESS_MAX;
    RGBColor        color       = 0;

    if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
    {
        speed       = modes[active_mode].speed;
    }

    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness  = modes[active_mode].brightness;
    }

    if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        color       = modes[active_mode].colors[0];
    }

    controller->SetEffect(enable, modes[active_mode].value, speed, brightness, color);

    if(modes[active_mode].value == MCHOSE_V9_DOCK_MODE_DIRECT)
    {
        last_update_time = std::chrono::steady_clock::now();
        controller->SetLEDs(colors);
    }
}

void RGBController_MCHOSEV9Dock::KeepaliveThread()
{
    while(keepalive_thread_run.load())
    {
        if(modes[active_mode].value == MCHOSE_V9_DOCK_MODE_DIRECT
        && (std::chrono::steady_clock::now() - last_update_time) > std::chrono::milliseconds(250))
        {
            UpdateLEDsInternal();
        }
        std::this_thread::sleep_for(50ms);
    }
}

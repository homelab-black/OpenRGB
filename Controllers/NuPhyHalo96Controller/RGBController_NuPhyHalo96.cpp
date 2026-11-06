/*---------------------------------------------------------*\
| RGBController_NuPhyHalo96.cpp                             |
|                                                           |
|   OpenRGB Controller for NuPhy Halo96 V2 keyboard.        |
|   Implements the OpenRGB RGBController API.               |
|                                                           |
|   Amir Yelenov (GreamDesu)                    22 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/
#include "RGBController_NuPhyHalo96.h"
#include "LogManager.h"
#include <thread>

RGBController_NuPhyHalo96::RGBController_NuPhyHalo96(NuPhyHalo96Controller* controller_ptr)
{
    LOG_INFO("RGBController_NuPhyHalo96: Constructor - Entry. Name: %s", this->name.c_str());
    this->controller = controller_ptr;

    this->name = "NuPhy Halo96 V2";
    this->vendor = "NuPhy";
    this->description = "NuPhy Halo96 Mechanical Keyboard";
    this->version = "1.0";

    if (controller_ptr) 
    {
        this->serial = controller_ptr->GetSerialString();
        this->location = controller_ptr->GetDeviceLocation();
        // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Serial: %s, Location: %s", this->serial.c_str(), this->location.c_str()); // Redundant with Detect.cpp logs
    }
    else
    {
        this->serial = "N/A (controller_ptr was null)";
        this->location = "N/A (controller_ptr was null)";
        LOG_ERROR("RGBController_NuPhyHalo96: Constructor - CRITICAL: controller_ptr is NULL!");
    }
    this->type = DEVICE_TYPE_KEYBOARD;

    // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Calling SetupZones()");
    SetupZones();
    // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Finished SetupZones(). zones.size() = %zu", this->zones.size());
    // if (!this->zones.empty()) {
    //     LOG_INFO("RGBController_NuPhyHalo96: Constructor - zones[0].leds_count = %u", this->zones[0].leds_count);
    // } else {
    //     LOG_WARNING("RGBController_NuPhyHalo96: Constructor - zones vector is empty after SetupZones()");
    // }

    if (!this->zones.empty() && this->zones[0].leds_count > 0)
    {
        this->leds.resize(this->zones[0].leds_count);
        // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Resized leds vector to %zu", this->leds.size());
        for (size_t i = 0; i < this->leds.size(); ++i)
        {
            this->leds[i].name = "LED " + std::to_string(i + 1);
            this->leds[i].value = static_cast<unsigned int>(i);
        }
        // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Populated leds vector with %zu elements.", this->leds.size());
    }
    else
    {
        LOG_WARNING("RGBController_NuPhyHalo96: Constructor - No zones or zero LEDs in zone 0. Not populating leds vector.");
    }

    // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Calling base::SetupColors()");
    SetupColors();
    // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Finished base::SetupColors(). colors.size() = %zu", this->colors.size());
    // if (this->colors.empty() && !this->zones.empty() && this->zones[0].leds_count > 0) {
    //     LOG_WARNING("RGBController_NuPhyHalo96: Constructor - colors vector is empty, but leds_count was > 0. This is problematic after SetupColors.");
    // }

    this->modes.resize(1);
    if (!this->modes.empty())
    {
        this->modes[0].name = "Static";
        this->modes[0].value = 0; 
        this->modes[0].flags = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
        this->modes[0].speed_min = 0;
        this->modes[0].speed_max = 0;
        this->modes[0].brightness_min = 0;
        this->modes[0].brightness_max = 0;
        this->modes[0].colors_min = 0;
        this->modes[0].colors_max = 0;
        this->modes[0].color_mode = MODE_COLORS_PER_LED; 
        // LOG_INFO("RGBController_NuPhyHalo96: Constructor - Added 'Static' mode. modes.size() = %zu", this->modes.size());
    }
    else
    {
        LOG_ERROR("RGBController_NuPhyHalo96: Constructor - Failed to resize modes vector for 'Static' mode!");
    }
    
    this->active_mode = 0;
    LOG_INFO("RGBController_NuPhyHalo96: Constructor - Exit. Name: %s", this->name.c_str());
}

RGBController_NuPhyHalo96::~RGBController_NuPhyHalo96()
{
    LOG_INFO("RGBController_NuPhyHalo96: Destructor - Entry. Name: %s", this->name.c_str());
    if (this->controller)
    {
        delete this->controller;
        this->controller = nullptr;
    }
    LOG_INFO("RGBController_NuPhyHalo96: Destructor - Exit. Name: %s", this->name.c_str());
}

void RGBController_NuPhyHalo96::SetupZones()
{
    // LOG_INFO("RGBController_NuPhyHalo96: SetupZones - Entry");
    this->zones.clear();
    this->zones.resize(1);
    if(this->zones.empty())
    {
        LOG_ERROR("RGBController_NuPhyHalo96: SetupZones - zones is empty after resize(1)!");
        return; 
    }
    this->zones[0].name = "Main LEDs";
    this->zones[0].type = ZONE_TYPE_LINEAR;
    this->zones[0].leds_count = 99;
    this->zones[0].leds_min = 99;
    this->zones[0].leds_max = 99;
    // LOG_INFO("RGBController_NuPhyHalo96: SetupZones - Zone 0 configured: name='%s', type=%d, leds_count=%u", this->zones[0].name.c_str(), this->zones[0].type, this->zones[0].leds_count);
    // LOG_INFO("RGBController_NuPhyHalo96: SetupZones - Exit");
}

void RGBController_NuPhyHalo96::ResizeZone(int /*zone*/, int /*new_size*/)
{
    // This keyboard has a fixed layout, so zones are not resizable.
    // This function can be left empty or log a message.
}

void RGBController_NuPhyHalo96::DeviceUpdateLEDs()
{
    if (this->controller && this->controller->IsDeviceOpen() && !this->modes.empty() && static_cast<size_t>(this->active_mode) < this->modes.size())
    {
        const auto& current_mode_obj = this->modes[this->active_mode]; 
        if (current_mode_obj.name == "Static")
        {
            this->controller->SetLEDs(this->colors); 
        }
    }
}

void RGBController_NuPhyHalo96::UpdateZoneLEDs(int zone_idx)
{
    // LOG_INFO("RGBController_NuPhyHalo96: UpdateZoneLEDs - Called for zone %d.", zone_idx);
    if (this->controller && this->controller->IsDeviceOpen() && zone_idx >= 0 && static_cast<size_t>(zone_idx) < this->zones.size())
    {
        this->controller->SetLEDs(this->colors);
    }
}

void RGBController_NuPhyHalo96::UpdateSingleLED(int led_idx)
{
    // LOG_INFO("RGBController_NuPhyHalo96: UpdateSingleLED - Called for LED %d.", led_idx);
    if (this->controller && this->controller->IsDeviceOpen())
    {
        // LOG_INFO("RGBController_NuPhyHalo96: UpdateSingleLED - Sending all %zu colors via SetLEDs (which now handles mode enabling).", this->colors.size());
        this->controller->SetLEDs(this->colors);
    }
}

void RGBController_NuPhyHalo96::DeviceUpdateMode()
{
    LOG_INFO("RGBController_NuPhyHalo96: DeviceUpdateMode - Called. Active mode: %d ('%s'). Name: %s", 
        this->active_mode, 
        (!this->modes.empty() && static_cast<size_t>(this->active_mode) < this->modes.size()) ? this->modes[this->active_mode].name.c_str() : "Unknown",
        this->name.c_str());
    ClearCallbacks();
    if (!this->modes.empty() && static_cast<size_t>(this->active_mode) < this->modes.size())
    {
        SetCustomMode(); 
        SignalUpdate();
    }
    else
    {
        LOG_WARNING("RGBController_NuPhyHalo96: DeviceUpdateMode - Active mode index out of bounds or modes empty. Name: %s", this->name.c_str());
    }
}

void RGBController_NuPhyHalo96::SetCustomMode()
{
    LOG_INFO("RGBController_NuPhyHalo96: SetCustomMode - Called. Name: %s", this->name.c_str());
    if (this->controller && this->controller->IsDeviceOpen())
    {
        // LOG_INFO("RGBController_NuPhyHalo96: SetCustomMode - Calling EnsureSoftwareModeActive(). Name: %s", this->name.c_str());
        this->controller->EnsureSoftwareModeActive();
    }
    else
    {
        LOG_WARNING("RGBController_NuPhyHalo96: SetCustomMode - Controller or device not open. Name: %s", this->name.c_str());
    }
}

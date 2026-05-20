/*---------------------------------------------------------*\
| OpenRGBPluginAPI.cpp                                      |
|                                                           |
|   Interface for OpenRGB plugins to call OpenRGB functions |
|                                                           |
|   Adam Honse (CalcProgrammer1)                08 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "OpenRGBPluginAPI.h"
#include "RGBController_Virtual.h"

OpenRGBPluginAPI::OpenRGBPluginAPI()
{
    log_manager         = ResourceManager::get()->GetLogManager();
    plugin_manager      = ResourceManager::get()->GetPluginManager();
    profile_manager     = ResourceManager::get()->GetProfileManager();
    resource_manager    = ResourceManager::get();
    settings_manager    = ResourceManager::get()->GetSettingsManager();
}

/*---------------------------------------------------------*\
| LogManager APIs                                           |
\*---------------------------------------------------------*/
void OpenRGBPluginAPI::LogEntry(const char* filename, int line, unsigned int level, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    log_manager->LogEntry_va(filename, line, level, fmt, va);

    va_end(va);
}

/*---------------------------------------------------------*\
| PluginManager APIs                                        |
\*---------------------------------------------------------*/
RGBControllerInterface* OpenRGBPluginAPI::CreateVirtualRGBController(RGBController_Setup* setup, bool register_controller)
{
    RGBController_Virtual* rgb_controller = new RGBController_Virtual(setup);

    if(register_controller)
    {
        LOG_INFO("[PluginManager] Registering RGB controller %s", rgb_controller->GetName().c_str());

        /*-------------------------------------------------*\
        | Add the new controller to the list                |
        \*-------------------------------------------------*/
        rgb_controllers.push_back(rgb_controller);
    }

    return(rgb_controller);
}

void OpenRGBPluginAPI::UpdateVirtualRGBControllerZone(RGBControllerInterface* rgb_controller, int zone_idx, zone updated_zone)
{
    if(rgb_controller)
    {
        ((RGBController_Virtual*)rgb_controller)->UpdateVirtualZone(zone_idx, updated_zone);
    }
}

void OpenRGBPluginAPI::DeleteVirtualRGBController(RGBControllerInterface* rgb_controller)
{
    delete (RGBController*)rgb_controller;
}

/*---------------------------------------------------------*\
| ProfileManager APIs                                       |
\*---------------------------------------------------------*/
void OpenRGBPluginAPI::ClearActiveProfile()
{
    profile_manager->ClearActiveProfile();
}

std::vector<std::string> OpenRGBPluginAPI::GetProfileList()
{
    return(profile_manager->GetProfileList());
}

bool OpenRGBPluginAPI::LoadProfile(std::string profile_name)
{
    return(profile_manager->LoadProfile(profile_name));
}

/*---------------------------------------------------------*\
| ResourceManager APIs                                      |
\*---------------------------------------------------------*/
filesystem::path OpenRGBPluginAPI::GetConfigurationDirectory()
{
    return(resource_manager->GetConfigurationDirectory());
}

bool OpenRGBPluginAPI::GetDetectionEnabled()
{
    return(resource_manager->GetDetectionEnabled());
}

unsigned int OpenRGBPluginAPI::GetDetectionPercent()
{
    return(resource_manager->GetDetectionPercent());
}

std::string OpenRGBPluginAPI::GetDetectionString()
{
    return(resource_manager->GetDetectionString());
}

void OpenRGBPluginAPI::RescanDevices()
{
    resource_manager->RescanDevices();
}

void OpenRGBPluginAPI::WaitForDetection()
{
    resource_manager->WaitForDetection();
}

std::vector<RGBControllerInterface*> OpenRGBPluginAPI::GetRGBControllers()
{
    return(resource_manager->GetRGBControllerInterfaces());
}

/*---------------------------------------------------------*\
| SettingsManager APIs                                      |
\*---------------------------------------------------------*/
nlohmann::json OpenRGBPluginAPI::GetSettings(std::string settings_key)
{
    return(settings_manager->GetSettings(settings_key));
}

void OpenRGBPluginAPI::SaveSettings()
{
    settings_manager->SaveSettings();
}

void OpenRGBPluginAPI::SetSettings(std::string settings_key, nlohmann::json new_settings)
{
    settings_manager->SetSettings(settings_key, new_settings);
}

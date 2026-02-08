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
void OpenRGBPluginAPI::append(const char* filename, int line, unsigned int level, const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);

    log_manager->append_va(filename, line, level, fmt, va);

    va_end(va);
}

/*---------------------------------------------------------*\
| PluginManager APIs                                        |
\*---------------------------------------------------------*/

/*---------------------------------------------------------*\
| ProfileManager APIs                                       |
\*---------------------------------------------------------*/
void OpenRGBPluginAPI::ClearActiveProfile()
{
    profile_manager->ClearActiveProfile();
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

void OpenRGBPluginAPI::WaitForDetection()
{
    resource_manager->WaitForDetection();
}

std::vector<RGBController*> & OpenRGBPluginAPI::GetRGBControllers()
{
    return(resource_manager->GetRGBControllers());
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

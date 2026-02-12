/*---------------------------------------------------------*\
| SettingsManager.cpp                                       |
|                                                           |
|   OpenRGB Settings Manager maintains a list of application|
|   settings in JSON format.  Other components may register |
|   settings with this class and store/load values.         |
|                                                           |
|   Adam Honse (CalcProgrammer1)                04 Nov 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <fstream>
#include <iostream>
#include "LogManager.h"
#include "NetworkClient.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "StringUtils.h"

SettingsManager::SettingsManager()
{
    config_found = false;
}

SettingsManager::~SettingsManager()
{
}

SettingsAccessGuard SettingsManager::GetSettings()
{
    if(ResourceManager::get()->IsLocalClient() && (ResourceManager::get()->GetLocalClient()->GetSupportsSettingsManagerAPI()))
    {
        /*-------------------------------------------------*\
        | If this is a local client, request the settings   |
        | from the server                                   |
        \*-------------------------------------------------*/
        try
        {
            json result = nlohmann::json::parse(ResourceManager::get()->GetLocalClient()->SettingsManager_GetSettings());
            // todo: maybe only invalidate this after a save?
            local_settings = result.get<Settings>();
            return SettingsAccessGuard(local_settings, mutex);
        }
        catch(...)
        {

        }
    }
    return SettingsAccessGuard(settings_data, mutex);
}

void SettingsManager::ModifySettings(std::function<void(Settings& settings)> modify_fn)
{
    mutex.lock();
    modify_fn(settings_data);
    mutex.unlock();
    SaveSettings();
}

void SettingsManager::SetSettingsFromJsonString(std::string settings_json_str)
{
    /*-----------------------------------------------------*\
    | Parse the JSON string                                 |
    \*-----------------------------------------------------*/
    nlohmann::json settings_json = nlohmann::json::parse(settings_json_str);
    Settings settings = settings_json.get<Settings>();

    mutex.lock();
    if(ResourceManager::get()->IsLocalClient() && (ResourceManager::get()->GetLocalClient()->GetSupportsSettingsManagerAPI()))
    {
        local_settings = settings;
        json serialized = local_settings;
        ResourceManager::get()->GetLocalClient()->SettingsManager_SetSettings(serialized.dump());
    }
    else
    {
        settings_data = settings;
    }
    mutex.unlock();
}

void SettingsManager::LoadSettings(const filesystem::path& filename)
{
    /*-----------------------------------------------------*\
    | Clear any stored settings before loading              |
    \*-----------------------------------------------------*/
    mutex.lock();

    /*-----------------------------------------------------*\
    | Store settings filename, so we can save to it later   |
    \*-----------------------------------------------------*/
    settings_filename = filename;

    /*-----------------------------------------------------*\
    | Open input file in binary mode                        |
    \*-----------------------------------------------------*/
    config_found = filesystem::exists(filename);
    json raw_json = LoadSettingsAsJson(filename);
    try
    {
        settings_data = raw_json.get<Settings>();
    }
    catch(const std::exception& e)
    {
        /*-----------------------------------------*\
        | If an exception was caught, that means    |
        | the JSON parsing failed.  Clear out any   |
        | data in the store as it is corrupt.  We   |
        | could attempt a reload for backup         |
        | location                                  |
        \*-----------------------------------------*/
        LOG_ERROR("[SettingsManager] JSON parsing of %s failed: %s", filename.c_str(), e.what());
    }
    mutex.unlock();
}

void SettingsManager::SaveSettingsJson(const filesystem::path& filename, const json& settings)
{
    LOG_DEBUG("[SettingsManager] Saving settings to %s", filename.c_str());
    std::ofstream settings_file(filename, std::ios::out | std::ios::binary);
    if(!settings_file)
    {
        return;
    }

    try
    {
        settings_file << settings.dump(4);
    }
    catch(const std::exception& e)
    {
        LOG_ERROR("[SettingsManager] Cannot write to file: %s", e.what());
    }

    settings_file.close();
}

json SettingsManager::LoadSettingsAsJson(const filesystem::path& filename)
{
    if (!filesystem::exists(filename))
    {
        return json {};
    }
    LOG_DEBUG("[SettingsManager] Reading settings from %s", filename.c_str());
    json data;
    std::ifstream settings_file(filename, std::ios::in | std::ios::binary);

    /*-------------------------------------------------*\
    | Read settings into JSON store                     |
    \*-------------------------------------------------*/
    if(settings_file)
    {
        try
        {
            settings_file >> data;
        }
        catch(const std::exception& e)
        {
            /*-----------------------------------------*\
            | If an exception was caught, that means    |
            | the JSON parsing failed.  Clear out any   |
            | data in the store as it is corrupt.  We   |
            | could attempt a reload for backup         |
            | location                                  |
            \*-----------------------------------------*/
            LOG_ERROR("[SettingsManager] Reading config file failed: %s", e.what());
        }
    }

    settings_file.close();
    return data;
}


void SettingsManager::SaveSettings()
{
    mutex.lock();
    if(ResourceManager::get()->IsLocalClient() && (ResourceManager::get()->GetLocalClient()->GetSupportsSettingsManagerAPI()))
    {
        /*-------------------------------------------------*\
        | If this is a local client, save the settings on   |
        | the server                                        |
        \*-------------------------------------------------*/
        json settings_json = local_settings;
        ResourceManager::get()->GetLocalClient()->SettingsManager_SetSettings(settings_json.dump());
        ResourceManager::get()->GetLocalClient()->SettingsManager_SaveSettings();
    }

    SaveSettingsJson(settings_filename, settings_data);
    mutex.unlock();
}

void SettingsManager::MigrateSettings(const filesystem::path& filename)
{
    /*----------------------------*\
    | pre settings rework settings |
    \*----------------------------*/
    json settings_json = LoadSettingsAsJson(filename);

    /*----------------------------------------------------*\
    | If no settings read, then there's nothing to migrate |
    \*----------------------------------------------------*/
    if(settings_json.is_null())
    {
        return;
    }


    LOG_DEBUG("[SettingsManager] Checking if settings migration is needed");
    std::string version;
    if(!settings_json.contains(VERSION_CFG_KEY))
    {
        version = "0.0";
    }
    else
    {
        version = settings_json[VERSION_CFG_KEY];
    }

    LOG_DEBUG("[SettingsManager] Read config version %s", version.c_str());

    if (version == "0.0")
    {
        LOG_DEBUG("[SettingsManager] Migrating from 0.0 -> 1.0");
        /*-----------------------------------------*\
        | migrate AutoStart.port from string to int |
        \*-----------------------------------------*/
        LOG_TRACE("[SettingsManager] Migrating from AutoStart.port string -> int");
        if(settings_json.contains("AutoStart") && settings_json["AutoStart"].contains("port"))
        {
            json& port_json = settings_json["AutoStart"]["port"];
            if(port_json.is_string())
            {
                std::string port_str = port_json.get<std::string>();
                if (port_str.empty())
                {
                    port_json = 0;
                }
                else
                {
                    port_json = std::stoi(port_str);
                }
            }
            else
            {
                port_json = 0;
            }
        }

        /*--------------------------*\
        | Migrate controllers        |
        \*--------------------------*/
        std::string controller_keys[] {
            "ASRock Polychrome USB",
            "Asus Aura Core Laptop",
            "CorsairDominatorSettings",
            "DDPDevices",
            "DebugDevices",
            "DMXDevices",
            "E131Devices",
            "Gigabyte RGB Fusion 2 SMBus",
            "Gigabyte RGB Fusion 2 USB",
            "ElgatoLightStripDevices",
            "ENESMBusSettings",
            "EspurnaDevices",
            "EVision2Settings",
            "EndorfySettings",
            "FanBusDevices",
            "GoveeDevices",
            "KasaSmartDevices",
            "LEDStripDevices",
            "LIFXDevices",
            "LinuxLEDDevices",
            "PhilipsHueDevices",
            "PhilipsWizDevices",
            "QMKOpenRGBDevices",
            "NanoleafDevices",
            "QMKVialRGBDevices",
            "YeelightDevices",
        };
        LOG_TRACE("[SettingsManager] Migrating controllers");
        settings_json["Controllers"] = json::object({ {"data", {}}});
        for(const std::string& key : controller_keys)
        {
            /*------------------------------------------------------------*\
            | Move $.<ControllerKey> to $.Controllers.data.<ControllerKey> |
            \*------------------------------------------------------------*/
            LOG_TRACE("[SettingsManager] Migrating controller %s", key.c_str());
            if (settings_json.contains(key))
            {
                settings_json["Controllers"]["data"][key] = settings_json[key];
                settings_json.erase(key);
            }
        }

        /*----------------------------------------------*\
        | Migrate ui.RunZoneChecks -> ui.run_zone_checks |
        \*----------------------------------------------*/
        LOG_TRACE("[SettingsManager] Migrating UserInterface.RunZoneChecks -> UserInterface.run_zone_checks");
        if (settings_json.contains("UserInterface") && settings_json["UserInterface"].contains("RunZoneChecks"))
        {
            settings_json["UserInterface"]["run_zone_checks"] = settings_json["UserInterface"]["RunZoneChecks"];
            settings_json["UserInterface"]["RunZoneChecks"].clear();
        }

        /*------------------------------------*\
        | We should be able to deserialize now |
        \*------------------------------------*/
        settings_json[VERSION_CFG_KEY] = SETTINGS_VERSION;
        SaveSettingsJson(filename, settings_json);
        LOG_DEBUG("[SettingsManager] Sucessfully migrated from 0.0 -> 1.0");
    }
    else if(version == "1.0")
    {
        LOG_DEBUG("[SettingsManager] Migration not needed, already at version 1.0");
    }
}

void SettingsManager::SetControllerSettingsAndSave(const std::string& key, json settings)
{
    mutex.lock();
    settings_data.controllers.data[key] = settings;
    mutex.unlock();
    SaveSettings();
}

json SettingsManager::GetControllerSettingsRaw(const std::string& key)
{
    json j = json::object();
    mutex.lock();
    if(settings_data.controllers.data.contains(key))
    {
        j = settings_data.controllers.data[key];
    }
    mutex.unlock();
    return j;
}

void SettingsManager::SetControllerSettingsAndSaveRaw(const std::string& key, json settings)
{
    mutex.lock();
    settings_data.controllers.data[key] = settings;
    mutex.unlock();
    SaveSettings();
}

SettingsAccessGuard::SettingsAccessGuard(Settings& _settings, std::mutex& _mutex) : settings(_settings), lock(_mutex)
{
    /*-------------------------------------*\
    | Debug message to invesitage deadlocks |
    \*-------------------------------------*/
    // LOG_DEBUG("[SettingsAccessGuard] ++++++ Acquiring exclusive access to settings ++++++");
}

SettingsAccessGuard::~SettingsAccessGuard()
{
    /*-------------------------------------*\
    | Debug message to invesitage deadlocks |
    \*-------------------------------------*/
    // if (lock.owns_lock())
    // {
    //     LOG_DEBUG("[SettingsAccessGuard] ------ Automatically releasing exclusive access to settings ------");
    // }
}

Settings& SettingsAccessGuard::access_mut()
{
    return settings;
}

const Settings& SettingsAccessGuard::access()
{
    return settings;
};

void SettingsAccessGuard::Save()
{
    lock.unlock();
    ResourceManager::get()->GetSettingsManager()->SaveSettings();
    lock.lock();
}

void SettingsAccessGuard::Release()
{
    LOG_DEBUG("[SettingsAccessGuard] ------ Manually releasing exclusive access to settings ------");
    lock.unlock();
}

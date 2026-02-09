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

static const std::string ui_settings_keys[7] =
{
    "UserInterface",
    "AutoStart",
    "Theme",
    "Plugins",
    "Client",
    "LogManager",
    "Server"
};

SettingsManager::SettingsManager()
{
    config_found = false;
}

SettingsManager::~SettingsManager()
{

}

SettingsAccessGuard SettingsManager::GetSettings()
{
    // bool ui_settings_key = false;
    // for(std::size_t settings_key_idx = 0; settings_key_idx < 7; settings_key_idx++)
    // {
    //     if(settings_key == ui_settings_keys[settings_key_idx])
    //     {
    //         ui_settings_key = true;
    //         break;
    //     }
    // }

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
    else
    {
        /*-------------------------------------------------*\
        | Check to see if the key exists in the settings    |
        | store and return the settings associated with the |
        | key if it exists.  We lock the mutex to protect   |
        | the value from changing while data is being read  |
        | and copy before unlocking.                        |
        \*-------------------------------------------------*/
        return SettingsAccessGuard(settings_data, mutex);
        // mutex.lock();
        // if(settings_data.contains(settings_key))
        // {
        //     result = settings_data[settings_key];
        // }
        // mutex.unlock();
    }
    // return result;
}

void SettingsManager::ModifySettings(std::function<void(Settings& settings)> modify_fn)
{
    mutex.lock();
    modify_fn(settings_data);
    mutex.unlock();
    SaveSettings();
}

// void SettingsManager::SetSettings(std::string settings_key, json new_settings)
// {
//     bool ui_settings_key = false;

//     for(std::size_t settings_key_idx = 0; settings_key_idx < 7; settings_key_idx++)
//     {
//         if(settings_key == ui_settings_keys[settings_key_idx])
//         {
//             ui_settings_key = true;
//             break;
//         }
//     }

//     if(!ui_settings_key && ResourceManager::get()->IsLocalClient() && (ResourceManager::get()->GetLocalClient()->GetSupportsSettingsManagerAPI()))
//     {
//         /*-------------------------------------------------*\
//         | If this is a local client, request the settings   |
//         | from the server                                   |
//         \*-------------------------------------------------*/
//         // nlohmann::json settings_json;

//         // settings_json[settings_key] = new_settings;
//         json settings_json = local_settings;

//         ResourceManager::get()->GetLocalClient()->SettingsManager_SetSettings(settings_json.dump());
//     }
//     else
//     {
//         // noop, we edit an existing reference
//         // mutex.lock();
//         // settings_data[settings_key] = new_settings;
//         // mutex.unlock();
//     }
// }

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
    if(config_found)
    {
        std::ifstream settings_file(settings_filename, std::ios::in | std::ios::binary);

        /*-------------------------------------------------*\
        | Read settings into JSON store                     |
        \*-------------------------------------------------*/
        if(settings_file)
        {
            try
            {
                json data;
                settings_file >> data;
                settings_data = data.get<Settings>();
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
                LOG_ERROR("[SettingsManager] JSON parsing failed: %s", e.what());
            }
        }

        settings_file.close();
    }

    mutex.unlock();
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

    std::ofstream settings_file(settings_filename, std::ios::out | std::ios::binary);

    if(settings_file)
    {
        try
        {
            json data = settings_data;
            settings_file << data;
        }
        catch(const std::exception& e)
        {
            LOG_ERROR("[SettingsManager] Cannot write to file: %s", e.what());
        }

        settings_file.close();
    }
    mutex.unlock();
}

template<typename T>
std::optional<T> SettingsManager::GetControllerSettings(const std::string& key)
{
    mutex.lock();
    std::optional<T> deserialized;
    if (settings_data.controllers.data.contains(key))
    {
        deserialized = settings_data.controllers.data[key].get<T>();
    }
    else
    {
        deserialized = std::nullopt;
    }
    mutex.unlock();
    return deserialized;
}

template<typename T>
void SettingsManager::SetControllerSettingsAndSave(const std::string& key, T settings)
{
    json serialized = settings;
    mutex.lock();
    settings_data.controllers.data[key] = serialized;
    mutex.unlock();
    SaveSettings();
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
    lock.unlock();
}

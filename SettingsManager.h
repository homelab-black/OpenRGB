/*---------------------------------------------------------*\
| SettingsManager.h                                         |
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

#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <mutex>
#include "filesystem.h"
#include "SettingsJson.h"
#include "LogManager.h"

using json = nlohmann::json;

class SettingsAccessGuard
{
public:
    SettingsAccessGuard(Settings& _settings, std::mutex& _mutex);
    // : settings(_settings), lock(_mutex)
    // {
    //     LOG_DEBUG("[SettingsAccessGuard] Acquiring exclusive access to settings");
    // }

    ~SettingsAccessGuard();
    // {
    //     LOG_DEBUG("[SettingsAccessGuard] Automatically releasing exclusive access to settings");
    // }

    /*-------------------------------------------*\
    | Accesses the settings immutably (read-only) |
    \*-------------------------------------------*/
    const Settings& access();
    /*--------------------------------------------*\
    | Accesses the settings mutably (write-access) |
    \*--------------------------------------------*/
    Settings& access_mut();

    /*----------------------------------------------------*\
    | Saves the currently accessed settings.               |
    | This drops the internal lock, use this when possible |
    \*----------------------------------------------------*/
    void Save();

    /*------------------------------------------------*\
    | Releases the lock in this access guard           |
    | After calling this this access guard is invalid. |
    | Any further operation is undefined behaviour     |
    | until lock is reacquired.                        |
    \*------------------------------------------------*/
    void Release();

    // template<typename T>
    // T GetSettingsValue(std::function<T(const Settings& settings)> getter_fn);

private:
    Settings& settings;
    std::unique_lock<std::mutex> lock;
};


class SettingsManagerInterface
{
public:
    virtual SettingsAccessGuard GetSettings()                       = 0;
    virtual void ModifySettings(std::function<void(Settings& settings)> modify_fn) = 0;
    virtual void SetSettingsFromJsonString(std::string settings_json_str)    = 0;

    virtual void LoadSettings(const filesystem::path& filename)              = 0;
    virtual void SaveSettings()                                              = 0;

protected:
    virtual ~SettingsManagerInterface() {};
};

class SettingsManager: public SettingsManagerInterface
{
public:
    SettingsManager();
    ~SettingsManager();

    SettingsAccessGuard GetSettings() override;
    void ModifySettings(std::function<void(Settings& settings)> modify_fn) override;
    void SetSettingsFromJsonString(std::string settings_json_str) override;

    /*----------------------------*\
    | Controller-specific settings |
    \*----------------------------*/
    // template<typename T>
    // std::optional<T> GetControllerSettings(const std::string& key);

    // template<typename T>
    // T GetControllerSettingsOrDefault(const std::string& key);

    void SetControllerSettingsAndSave(const std::string& key, json settings);

    /*---------------------------------------------------------------------*\
    | Template methods must be in header file or else we get a linker error |
    | I think?                                                              |
    \*---------------------------------------------------------------------*/
    template<typename T>
    std::optional<T> GetControllerSettings(const std::string& key)
    {
        json data = GetControllerSettingsRaw(key);
        /*----------------------------*\
        | skip parsing if data is null |
        \*----------------------------*/
        if (data.is_null())
        {
            return std::nullopt;
        }

        try
        {
            return data.get<T>();
        }
        catch (std::exception& e)
        {
            LOG_ERROR("[SettingsManager] Failed to parse controller key '%s' to type %s: %s", key.c_str(), typeid(T).name(), e.what());
            LOG_DEBUG("[SettingsManager] raw json: %s", to_string(data).c_str());
            return std::nullopt;
        }
    }

    template<typename T>
    T GetControllerSettingsOrDefault(const std::string& key)
    {
        /*------------------------------------*\
        | use default constructor for `T` here |
        \*------------------------------------*/
        T settings;
        json data = GetControllerSettingsRaw(key);
        if (!data.is_null())
        {
            settings = data.get<T>();
        }
        return settings;
    }

    /*-------------------------------------------*\
    | Gets settings at controller key as raw JSON |
    |                                             |
    | Prefer defining a deserialisable cpp struct |
    | and calling `GetControllerSettings`         |
    \*-------------------------------------------*/
    json GetControllerSettingsRaw(const std::string& key);
    /*------------------------------------------------*\
    | Saves raw JSON to the controller key as raw JSON |
    |                                                  |
    | Prefer defining a deserialisable cpp struct      |
    | and calling `GetControllerSettings`              |
    \*------------------------------------------------*/
    void SetControllerSettingsAndSaveRaw(const std::string& key, json settings);


    void LoadSettings(const filesystem::path& filename) override;
    void SaveSettings() override;

    /*----------------------------------------------------------*\
    | Migrates settings from an older version to the current one |
    | Should be called right after `LoadSettings`                |
    \*----------------------------------------------------------*/
    void MigrateSettings(const filesystem::path& filename);

private:
    json LoadSettingsAsJson(const filesystem::path& filename);
    void SaveSettingsJson(const filesystem::path& filename, const json& settings);

private:
    Settings             settings_data;
    json             settings_prototype;
    filesystem::path settings_filename;
    std::mutex       mutex;
    bool             config_found;


    // Todo: make subset of settings that local users actually use
    // for example, ui settings are unused by local users
    Settings        local_settings;
};

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
#include <mutex>
#include "filesystem.h"
#include "SettingsJson.h"

using json = nlohmann::json;

class SettingsAccessGuard
{
public:
    SettingsAccessGuard(Settings& _settings, std::mutex& _mutex)
    : settings(_settings), lock(_mutex)
    {}

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
    template<typename T>
    std::optional<T> GetControllerSettings(const std::string& key);
    template<typename T>
    void SetControllerSettingsAndSave(const std::string& key, T settings);

    void LoadSettings(const filesystem::path& filename) override;
    void SaveSettings() override;

private:
    Settings             settings_data;
    json             settings_prototype;
    filesystem::path settings_filename;
    std::mutex       mutex;
    bool             config_found;


    Settings        local_settings;
};

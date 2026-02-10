/*---------------------------------------------------------*\
| PhilipsHueSettingsHandler.h                               |
|                                                           |
|   Settings Handler for Philips Hue                        |
|   Due to conflict in jsoh.hpp library, hueplusplus and    |
|   SettingsManager should not be included in the same file |
|   so handle settings in a separate class.                 |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      17 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstddef>
#include <string>

#define PHILIPSHUE_SETTINGS_KEY "PhilipsHueDevices"

struct BridgeSettings
{
    std::string ip;
    std::string mac;
    std::string username;
    std::string clientkey;
    bool autoconnect;
    bool entertainment;
};

struct HueSettings
{
    std::vector<BridgeSettings> bridges;
};

class PhilipsHueSettingsHandler
{
public:
    PhilipsHueSettingsHandler();
    ~PhilipsHueSettingsHandler();

    std::size_t     GetBridgeCount();
    const BridgeSettings&  GetBridge(unsigned int bridge_idx);

    void            SetBridge(unsigned int bridge_idx, BridgeSettings settings);
    void            SaveSettings();

private:
    HueSettings     settings;
};

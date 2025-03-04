/*---------------------------------------------------------*\
| YeelightController.h                                      |
|                                                           |
|   Driver for Yeelight                                     |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include <MiioDevice.h>

#include <string>
#include <vector>

class YeelightController
{
public:
    YeelightController(std::string ip, std::string token);

    std::string GetLocation();
    std::string GetName();
    std::string GetVersion();
    std::string GetManufacturer();
    std::string GetUniqueID();

    void SetPower();
    void SetColor(unsigned int rgb);
    void SetBrightness(unsigned int level);

    json GetProperties(std::vector<std::string> props);

    unsigned int        GetRgb() const;
    unsigned int        GetBrightness() const;
    bool                GetPower() const;

private:
    std::string         name;
    std::string         version;
    std::string         location;
    std::string         token;
    unsigned int        rgb;
    unsigned int        level;
    bool                power;

    MiioDevice          miio_device;

};

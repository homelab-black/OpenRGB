/*---------------------------------------------------------*\
| YeelightController.cpp                                    |
|                                                           |
|   Driver for Yeelight                                     |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "YeelightController.h"
#include <MiioDevice.h>


YeelightController::YeelightController(std::string ip, std::string token) : miio_device(ip, token)
{

    json props = GetProperties({"name", "fw_ver", "power", "bright", "rgb"});

    this->location = "IP: " + ip;
    this->token = token;

    this->name = props["result"][0];

    if (this->name.empty()) {
        this->name = "Yeelight (" + ip + ")";
    }

    this->version = props["result"][1];
    this->power = props["result"][2].get<std::string>() == "on";
    this->level = static_cast<unsigned int>(std::stoul(props["result"][3].get<std::string>()));
    this->rgb = static_cast<unsigned int>(std::stoul(props["result"][4].get<std::string>()));

}

std::string YeelightController::GetLocation()
{
    return(location);
}

std::string YeelightController::GetName()
{
    return(name);
}

std::string YeelightController::GetVersion()
{
    return(version);
}

std::string YeelightController::GetManufacturer()
{
    return("Yeelight");
}

std::string YeelightController::GetUniqueID()
{
    return(token);
}

unsigned int YeelightController::GetBrightness() const
{
    return level;
}

bool YeelightController::GetPower() const
{
    return power;
}

unsigned int YeelightController::GetRgb() const
{
    return rgb;
}

void YeelightController::SetPower()
{
    json command;

    command["method"] =  "set_power";
    command["params"][0] = "on";

    miio_device.sendCommand(command);
}

void YeelightController::SetColor(unsigned int rgb)
{
    json command;

    command["method"] =  "set_rgb";
    command["params"][0] = rgb;

    miio_device.sendCommand(command);
}

void YeelightController::SetBrightness(unsigned int level)
{
    json command;

    command["method"] =  "set_bright";
    command["params"][0] = level;

    miio_device.sendCommand(command);
}

json YeelightController::GetProperties(std::vector<std::string> props)
{
    json command;

    command["method"] = "get_prop";
    command["params"] = props;
    // {
        // "name",
        // "power",
        // "bright",
        // "color_mode",
        // "rgb",
        // "flow_params"
    // };

    return miio_device.sendCommand(command);
}

/*---------------------------------------------------------*\
| LinuxLEDController_Linux.h                                |
|                                                           |
|   Driver for Linux sysfs LEDs                             |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      25 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define LINUX_LED_DEVICE_CFG_KEY "LinuxLEDDevices"

struct LinuxLEDDevice
{
    std::string name;
    std::string red_path;
    std::string green_path;
    std::string blue_path;
    std::string rgb_path;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LinuxLEDDevice,
    name,
    red_path,
    green_path,
    blue_path,
    rgb_path
);

struct LinuxLEDSettings
{
    std::vector<LinuxLEDDevice> devices;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LinuxLEDSettings, devices);

class LinuxLEDController
{
public:
    LinuxLEDController(std::string dev_name);
    ~LinuxLEDController();

    std::string GetName();

    std::string GetRedPath();
    std::string GetBluePath();
    std::string GetGreenPath();
    std::string GetRgbPath();

    void OpenRedPath(std::string red_path);
    void OpenGreenPath(std::string green_path);
    void OpenBluePath(std::string blue_path);
    void OpenRgbPath(std::string rgb_path);

    void SetRGB(unsigned char red, unsigned char grn, unsigned char blu);

private:
    std::string     led_r_path;
    std::string     led_g_path;
    std::string     led_b_path;
    std::string     led_rgb_path;
    std::ofstream   led_r_brightness;
    std::ofstream   led_g_brightness;
    std::ofstream   led_b_brightness;
    std::ofstream   led_rgb_brightness;
    std::ofstream   led_rgb_color;
    std::string     name;
};

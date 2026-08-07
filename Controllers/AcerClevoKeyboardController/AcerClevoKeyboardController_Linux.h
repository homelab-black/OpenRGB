/*---------------------------------------------------------*\
| AcerClevoKeyboardController_Linux.h                       |
|                                                           |
|   Hardware driver for Acer Clevo keyboard backlight       |
|   Communicates via Linux sysfs (TUXEDO drivers)           |
|                                                           |
|   Shivam Purohit                               08 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <mutex>

class AcerClevoKeyboardController
{
public:
    AcerClevoKeyboardController(const std::string& sysfs_path, const std::string& name);
    ~AcerClevoKeyboardController();

    std::string     GetDeviceName();
    std::string     GetLocation();

    unsigned int    GetZoneCount();

    void            SetBrightness(unsigned char brightness);
    unsigned char   GetBrightness();

    void            SetColor(unsigned char red, unsigned char green, unsigned char blue);

private:
    std::string     sysfs_path;
    std::string     device_name;
    unsigned int    zone_count;
    unsigned char   brightness;
    unsigned char   color_r;
    unsigned char   color_g;
    unsigned char   color_b;
    std::mutex      mutex;

    bool            WriteSysfsFile(const std::string& filename, const std::string& value);
    std::string     ReadSysfsFile(const std::string& filename);
    unsigned int    DetectZoneCount();
};

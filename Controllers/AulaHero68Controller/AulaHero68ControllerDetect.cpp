/*---------------------------------------------------------*\
| AulaHero68ControllerDetect.cpp                            |
|                                                           |
|   Detector for AULA Hero 68 keyboard                      |
|                                                           |
|   Original author: dunn1o, 2026-08-09                     |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include <set>

#include "AulaHero68Controller.h"
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController_AulaHero68.h"
#include "StringUtils.h"

#define AULA_HERO_68_VID 0x372E
#define AULA_HERO_68_PID 0x103E

DetectedControllers DetectAulaHero68Controllers(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    static std::set<std::string> registered_serials;

    LOG_DEBUG("[AULA Hero 68] Interface %i\tPage %04X\tUsage %i\tPath %s", info->interface_number, info->usage_page, info->usage, info->path);

    /*---------------------------------------------------------*\
    | Avoid registering duplicate HID interfaces by tracking    |
    | the device serial number below.                           |
    \*---------------------------------------------------------*/

    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        wchar_t serial_string[128];
        std::string serial;

        if(hid_get_serial_number_string(dev, serial_string, 128) == 0)
        {
            serial = StringUtils::wstring_to_string(serial_string);
        }

        if(!serial.empty() && registered_serials.count(serial) != 0)
        {
            LOG_TRACE("[AULA Hero 68] Skipping duplicate serial %s from path %s", serial.c_str(), info->path);
            hid_close(dev);
            return detected_controllers;
        }

        if(!serial.empty())
        {
            registered_serials.insert(serial);
        }

        AulaHero68Controller*     controller     = new AulaHero68Controller(dev, info->path);
        RGBController_AulaHero68* rgb_controller = new RGBController_AulaHero68(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return detected_controllers;
}

REGISTER_HID_DETECTOR_I("AULA Hero 68", DetectAulaHero68Controllers, AULA_HERO_68_VID, AULA_HERO_68_PID, 2);

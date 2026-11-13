/*---------------------------------------------------------*\
| AulaControllerDetect.cpp                                  |
|                                                           |
|   Detector for Aula Keyboards                             |
|                                                           |
|   Antipov Kostiantyn (dev@diablosat.cc)      21.05.2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AulaF2067Controller.h"
#include <hidapi.h>
#include "Detector.h"
#include "RGBController_AulaF2067.h"

/*-----------------------------------------------------*\
| Aula vendor ID                                        |
\*-----------------------------------------------------*/
#define AULA_VID                                0x1A2C

/*-----------------------------------------------------*\
| Keyboard product IDs                                  |
\*-----------------------------------------------------*/
#define AULA_F2067_PID                          0x9CF4

void DetectAulaF2067Controllers(const hid_device_info* info, const std::string& _)
{
    if(hid_device* dev = hid_open_path(info->path))
    {
        AulaF2067Controller* controller = new AulaF2067Controller(dev, info->path);

        RGBController_AulaF2067* rgb_controller = new RGBController_AulaF2067(controller);
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR_IPU("Aula F2067", DetectAulaF2067Controllers, AULA_VID, AULA_F2067_PID, 0x01, 0x000C, 0x0001);
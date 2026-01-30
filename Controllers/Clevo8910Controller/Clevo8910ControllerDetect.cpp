
/*---------------------------------------------------------*\
|   Clevo8910ControllerDetector.cpp                         |
|                                                           |
|   Driver for Clevo Color per key keyboard                 |
|                                                           |
|   EzyUser     9 Jan 2026                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "RGBController.h"
#include "hidapi.h"
#include "Clevo8910Controller.h"
#include "RGBController_Clevo8910.h"

#define CLEVO8910_VID        0x048D
#define CLEVO8910_PID        0x8910


void DetectClevo8910Controller(hid_device_info* info, const std::string& name)
{


    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        Clevo8910Controller*     controller	= new Clevo8910Controller(dev, *info, info->product_id, name);
        RGBController_Clevo8910* rgb_controller	= new RGBController_Clevo8910(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR_PU("Clevo 8910 Keyboard", DetectClevo8910Controller, CLEVO8910_VID, CLEVO8910_PID, 0xFF89, 0xCC);

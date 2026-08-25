/*---------------------------------------------------------*\
|   GamepowerEva5ControllerDetect.cpp                       |
|                                                           |
|   Driver for Gamepower Eva 5 keyboard                     |
|                                                           |
|   Made by FeusX  (23 Aug 2026)                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#include <hidapi.h>
#include "DetectionManager.h"
#include "RGBController_GamepowerEva5.h"
#include "GamepowerEva5Controller.h"

#define GAMEPOWER_VID 0x1A2C
#define GAMEPOWER_EVA5_PID 0x9A0B

DetectedControllers DetectGamepowerEva5Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        GamepowerEva5Controller* controller = new GamepowerEva5Controller(dev, info->path, name);
        RGBController_GamepowerEva5* rgb_controller = new RGBController_GamepowerEva5(controller);
        detected_controllers.push_back(rgb_controller);
    }

    return detected_controllers;
}

REGISTER_HID_DETECTOR_IPU("Gamepower Eva5", DetectGamepowerEva5Controllers, GAMEPOWER_VID, GAMEPOWER_EVA5_PID, 0x01, 0xFF00, 0x01);

/*---------------------------------------------------------*\
| EvoFoxRoninControllerDetect.cpp                           |
|                                                           |
|   Detector for EvoFox Ronin TKL RGB keyboard              |
|                                                           |
|   <Atif Usmani> (<AtifUsmani>)               <30 09 2026> |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "EvoFoxRoninController.h"
#include "RGBController_EvoFoxRonin.h"

DetectedControllers DetectEvoFoxRoninControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    // The Ronin exposes two HID interfaces: interface 0 is the keyboard
    // input interface and interface 1 is the RGB control interface.
    if(info->interface_number != EVOFOX_RONIN_INTERFACE)
    {
        return detected_controllers;
    }

    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        EvoFoxRoninController* controller = new EvoFoxRoninController(dev, info->path, name);
        RGBController_EvoFoxRonin* rgb_controller = new RGBController_EvoFoxRonin(controller);
        detected_controllers.push_back(rgb_controller);
    }

    return detected_controllers;
}

REGISTER_HID_DETECTOR_IP("EvoFox Ronin TKL", DetectEvoFoxRoninControllers, EVOFOX_RONIN_VID, EVOFOX_RONIN_PID, EVOFOX_RONIN_INTERFACE, EVOFOX_RONIN_USAGE_PAGE);

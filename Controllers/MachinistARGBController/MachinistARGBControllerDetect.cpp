/*---------------------------------------------------------*\
| MachinistARGBControllerDetect.cpp                         |
|                                                           |
|   Detector for MACHINIST F-X9D ARGB Controller            |
|                                                           |
|   Ayane Suarez                                04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "MachinistARGBController.h"
#include "RGBController_MachinistARGB.h"
#include <hidapi.h>

DetectedControllers DetectMachinistARGBControllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        MachinistARGBController*     controller     = new MachinistARGBController(dev, info->path);
        RGBController_MachinistARGB* rgb_controller = new RGBController_MachinistARGB(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Machinist F-X9D ARGB", DetectMachinistARGBControllers, MACHINIST_VID, MACHINIST_PID, 0, 0x0001, 0x0000);

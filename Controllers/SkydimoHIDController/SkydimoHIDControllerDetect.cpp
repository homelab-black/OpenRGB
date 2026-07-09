/*---------------------------------------------------------*\
| SkydimoHIDControllerDetect.cpp                            |
|                                                           |
|   Detector for Skydimo HID devices                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "SkydimoHIDController.h"
#include "RGBController_SkydimoHID.h"

#define SKYDIMO_VID             0x1A86
#define SKYDIMO_SK0902_PID      0xE316

DetectedControllers DetectSkydimoHID(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if (dev)
    {
        SkydimoHIDController*       controller     = new SkydimoHIDController(dev, info->path);
        RGBController_SkydimoHID*   rgb_controller = new RGBController_SkydimoHID(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Skydimo SK0902", DetectSkydimoHID, SKYDIMO_VID, SKYDIMO_SK0902_PID, 0, 0xFF00, 0x01);

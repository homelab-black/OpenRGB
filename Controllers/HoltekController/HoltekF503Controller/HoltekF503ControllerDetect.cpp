/*---------------------------------------------------------*\
| HoltekF503ControllerDetect.cpp                            |
|                                                           |
|   Detector for Holtek F503 Mouse                          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "HoltekF503Controller.h"
#include "RGBController_HoltekF503.h"

/*---------------------------------------------------------*\
| Holtek Semiconductor Inc. vendor ID                       |
\*---------------------------------------------------------*/
#define HOLTEK_VID                                  0x04D9

/*---------------------------------------------------------*\
| Mouse product IDs                                         |
\*---------------------------------------------------------*/
#define HOLTEK_F503_PID                             0xF503

DetectedControllers DetectHoltekF503Controllers(hid_device_info *info, const std::string &name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        HoltekF503Controller*     controller     = new HoltekF503Controller(dev, info->path, name);
        RGBController_HoltekF503* rgb_controller = new RGBController_HoltekF503(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Holtek USB Gaming Mouse", DetectHoltekF503Controllers, HOLTEK_VID, HOLTEK_F503_PID, 2, 0xFFA0, 0x01);

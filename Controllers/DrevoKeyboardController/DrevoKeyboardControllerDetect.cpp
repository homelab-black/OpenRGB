/*---------------------------------------------------------*\
| DrevoKeyboardControllerDetect.cpp                         |
|                                                           |
|   Detector for DREVO BladeMaster PRO keyboards            |
|                                                           |
|   T-bond                                      27 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "DrevoKeyboardController.h"
#include "RGBController_DrevoKeyboard.h"

DetectedControllers DetectDrevoKeyboardControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        DrevoKeyboardController*     controller     = new DrevoKeyboardController(dev, *info, name);
        RGBController_DrevoKeyboard* rgb_controller = new RGBController_DrevoKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("DREVO BladeMaster PRO 88K", DetectDrevoKeyboardControllers, DREVO_VID, DREVO_BLADEMASTER_PRO_88_PID, DREVO_INTERFACE, DREVO_USAGE_PAGE, DREVO_USAGE);

/*---------------------------------------------------------*\
| ASUSLampArrayControllerDetect.cpp                         |
|                                                           |
|   Detector for ASUS Vivobook LampArray Keyboard (ITE5570) |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "ASUSLampArrayController.h"
#include "RGBController_ASUSLampArray.h"
#include <vector>
#include <hidapi.h>

#define ASUS_VID                        0x0B05
#define ASUS_VIVOBOOK_19B6_PID          0x19B6
#define ASUS_VIVOBOOK_5570_PID          0x5570

DetectedControllers DetectASUSLampArray(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        ASUSLampArrayController*     controller     = new ASUSLampArrayController(dev, info->path, info->product_id);
        RGBController_ASUSLampArray* rgb_controller = new RGBController_ASUSLampArray(controller, name);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR("ASUS Vivobook LampArray (19B6)", DetectASUSLampArray, ASUS_VID, ASUS_VIVOBOOK_19B6_PID);
REGISTER_HID_DETECTOR("ASUS Vivobook LampArray (5570)", DetectASUSLampArray, ASUS_VID, ASUS_VIVOBOOK_5570_PID);

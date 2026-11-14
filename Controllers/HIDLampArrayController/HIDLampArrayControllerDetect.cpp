/*---------------------------------------------------------*\
| HIDLampArrayControllerDetect.cpp                          |
|                                                           |
|   Detector for HID LampArray Devices                      |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      26 Mar 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "HIDLampArrayController.h"
#include "RGBController.h"
#include "RGBController_HIDLampArray.h"
#include <vector>
#include <hidapi.h>

DetectedControllers DetectHIDLampArrayControllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Legion 9 18IAX10 chassis lighting is drawn on the     |
    | keyboard device. A second sidebar entry would be the  |
    | same strip and logo again.                            |
    \*-----------------------------------------------------*/
    if(info->vendor_id == 0x048D && info->product_id == 0xC903)
    {
        return(detected_controllers);
    }

    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        HIDLampArrayController* controller = new HIDLampArrayController(dev, info->path);

        /*-------------------------------------------------*\
        | Only create the RGBController if there are lamps  |
        | detected in the controller.  Otherwise, delete    |
        | the controller.                                   |
        \*-------------------------------------------------*/
        if(controller->GetLampCount() > 0)
        {
            RGBController_HIDLampArray* rgb_controller = new RGBController_HIDLampArray(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            delete controller;
        }
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_PU_ONLY("HID LampArray Device", DetectHIDLampArrayControllers, 0x59, 0x01);

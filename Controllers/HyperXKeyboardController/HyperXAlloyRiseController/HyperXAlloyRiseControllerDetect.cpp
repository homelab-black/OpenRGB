/*---------------------------------------------------------*\
| HyperXAlloyRiseControllerDetect.cpp                       |
|                                                           |
|   Driver for HyperX Alloy Rise keyboards                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "HyperXAlloyRiseController.h"
#include "RGBController_HyperXAlloyRise.h"

/*---------------------------------------------------------*\
| HyperX keyboard vendor and product IDs (HP)               |
\*---------------------------------------------------------*/
#define HP_KEYBOARD_VID                             0x03F0
#define HYPERX_ALLOY_RISE_PID                       0x0FA0

DetectedControllers DetectHyperXAlloyRise(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        HyperXAlloyRiseController*     controller     = new HyperXAlloyRiseController(dev, info->path, name);
        RGBController_HyperXAlloyRise* rgb_controller = new RGBController_HyperXAlloyRise(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| The RGB channel is HID interface 2 (MI_02), the one with  |
| the 64 byte interrupt OUT endpoint                        |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("HyperX Alloy Rise", DetectHyperXAlloyRise, HP_KEYBOARD_VID, HYPERX_ALLOY_RISE_PID, 2);

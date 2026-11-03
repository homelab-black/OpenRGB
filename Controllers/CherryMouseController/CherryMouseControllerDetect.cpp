/*---------------------------------------------------------*\
| CherryMouseControllerDetect.cpp                           |
|                                                           |
|   Detector for Cherry mouse                               |
|                                                           |
|   Rik Vanden Boer                             03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "CherryMouseController.h"
#include "RGBController_CherryMouse.h"

/*---------------------------------------------------------*\
| Cherry VID, vendor usage page and usage                   |
\*---------------------------------------------------------*/
#define CHERRY_MOUSE_VID                        0x046A
#define CHERRY_MOUSE_USAGE_PAGE                 0xFF1C
#define CHERRY_MOUSE_USAGE                      0x0092

DetectedControllers DetectCherryMice(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        CherryMouseController*     controller     = new CherryMouseController(dev, info->path, name);
        RGBController_CherryMouse* rgb_controller = new RGBController_CherryMouse(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Mice                                                      |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Cherry Mouse MC 3.1", DetectCherryMice, CHERRY_MOUSE_VID, MC_3_1_PID, 1, CHERRY_MOUSE_USAGE_PAGE, CHERRY_MOUSE_USAGE);

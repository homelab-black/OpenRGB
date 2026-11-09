/*---------------------------------------------------------*\
| GigabyteM6980XControllerDetect.cpp                        |
|                                                           |
|   Detector for the Gigabyte M6980X mouse                  |
|                                                           |
|   Tom Rehm (Tom-8192)                         09 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "GigabyteM6980XController.h"
#include "RGBController_GigabyteM6980X.h"

/*---------------------------------------------------------*\
| Gigabyte M6980X vendor and product ID (OEM Areson)        |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_VID                 0xE0FF
#define GIGABYTE_M6980X_PID                 0x0001

/*---------------------------------------------------------*\
| The feature report lives on the mouse collection of       |
| interface 0                                               |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_INTERFACE           0
#define GIGABYTE_M6980X_USAGE_PAGE          0x0001
#define GIGABYTE_M6980X_USAGE               0x0002

DetectedControllers DetectGigabyteM6980XControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        GigabyteM6980XController* controller = new GigabyteM6980XController(dev, *info, name);

        /*-------------------------------------------------*\
        | Other models share the USB ID                     |
        \*-------------------------------------------------*/
        if(controller->IsM6980X())
        {
            RGBController_GigabyteM6980X* rgb_controller = new RGBController_GigabyteM6980X(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            delete controller;
        }
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Gigabyte M6980X", DetectGigabyteM6980XControllers, GIGABYTE_M6980X_VID, GIGABYTE_M6980X_PID, GIGABYTE_M6980X_INTERFACE, GIGABYTE_M6980X_USAGE_PAGE, GIGABYTE_M6980X_USAGE);

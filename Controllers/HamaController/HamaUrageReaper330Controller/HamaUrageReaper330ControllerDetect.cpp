/*---------------------------------------------------------*\
| HamaUrageReaper330ControllerDetect.cpp                    |
|                                                           |
|   Detector for Hama uRage Reaper 330 mouse                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "HamaUrageReaper330Controller.h"
#include "RGBController_HamaUrageReaper330.h"

/*---------------------------------------------------------*\
| Holtek Semiconductor Inc. vendor ID                       |
\*---------------------------------------------------------*/
#define HAMA_URAGE_VID                             0x04D9

/*---------------------------------------------------------*\
| Mouse product IDs                                         |
\*---------------------------------------------------------*/
#define HAMA_URAGE_REAPER_330_PID                  0xA23A

DetectedControllers DetectHamaUrageReaper330Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        HamaUrageReaper330Controller*     controller     = new HamaUrageReaper330Controller(dev, info->path, name);
        RGBController_HamaUrageReaper330* rgb_controller = new RGBController_HamaUrageReaper330(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Configuration is on USB interface 2 (vendor HID).         |
| Interface 0 is the plain mouse input.                     |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("Hama uRage Reaper 330", DetectHamaUrageReaper330Controllers, HAMA_URAGE_VID, HAMA_URAGE_REAPER_330_PID, 2);

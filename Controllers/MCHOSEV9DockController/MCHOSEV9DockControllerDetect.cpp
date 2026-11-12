/*---------------------------------------------------------*\
| MCHOSEV9DockControllerDetect.cpp                          |
|                                                           |
|   Detector for MCHOSE V9 charging dock                    |
|                                                           |
|   Michael Mano <michael.mano26@gmail.com>     24 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "MCHOSEV9DockController.h"
#include "RGBController_MCHOSEV9Dock.h"

/*---------------------------------------------------------*\
| MCHOSE vendor and product IDs                             |
\*---------------------------------------------------------*/
#define MCHOSE_VID                      0x3837
#define MCHOSE_V9_DOCK_PID        0x6008

DetectedControllers DetectMCHOSEV9DockControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        MCHOSEV9DockController*        controller      = new MCHOSEV9DockController(dev, info->path, name);
        RGBController_MCHOSEV9Dock*    rgb_controller  = new RGBController_MCHOSEV9Dock(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("MCHOSE V9 Dock", DetectMCHOSEV9DockControllers, MCHOSE_VID, MCHOSE_V9_DOCK_PID, 0, 0xFF00, 0x01);

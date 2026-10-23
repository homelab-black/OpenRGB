/*---------------------------------------------------------*\
| AntEsportsMK4100ControllerDetect.cpp                      |
|                                                           |
| Detector for Ant Esports MK4100 keyboard                  |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "LogManager.h"
#include "AntEsportsMK4100Controller.h"
#include "RGBController_AntEsportsMK4100.h"

#define ANT_ESPORTS_RGB_USAGE_PAGE 0xFF00
#define ANT_ESPORTS_RGB_USAGE      0x02

DetectedControllers DetectAntEsportsMK4100Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(info && info->path)
    {
        hid_device* dev = hid_open_path(info->path);
        if(dev)
        {
            AntEsportsMK4100Controller* controller = new AntEsportsMK4100Controller(dev, info->path, name);
            if(controller->IsInitialized())
            {
                RGBController_AntEsportsMK4100* rgb_controller = new RGBController_AntEsportsMK4100(controller);
                detected_controllers.push_back(rgb_controller);
            }
            else
            {
                LOG_WARNING("[Ant Esports MK4100] Initialization failed on %s: %s",
                            info->path, controller->GetErrorString().c_str());
                delete controller;
            }
        }
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_PU("Ant Esports MK4100", DetectAntEsportsMK4100Controllers, MK4100_VID, MK4100_PID, ANT_ESPORTS_RGB_USAGE_PAGE, ANT_ESPORTS_RGB_USAGE);

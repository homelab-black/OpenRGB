/*---------------------------------------------------------*\
| AntEsportsMK4100ControllerDetect.cpp                      |
|                                                           |
| Detector for Ant Esports MK4100 keyboard                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "AntEsportsMK4100Controller.h"
#include "RGBController_AntEsportsMK4100.h"

#define ANT_ESPORTS_VID       0x36AE
#define ANT_ESPORTS_MK4100_PID 0xFE9C
#define ANT_ESPORTS_RGB_USAGE_PAGE 0xFF00
#define ANT_ESPORTS_RGB_USAGE      0x02

DetectedControllers DetectAntEsportsMK4100Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    if(!info || !info->path) return detected_controllers;

    hid_device* dev = hid_open_path(info->path);
    if(!dev) return detected_controllers;

    AntEsportsMK4100Controller* controller = new AntEsportsMK4100Controller(dev, info->path, name);
    if(!controller->IsInitialized())
    {
        LOG_WARNING("[Ant Esports MK4100] Initialization failed on %s: %s",
                    info->path, controller->GetErrorString().c_str());
        delete controller;
        return detected_controllers;
    }

    RGBController_AntEsportsMK4100* rgb_controller = new RGBController_AntEsportsMK4100(controller);
    detected_controllers.push_back(rgb_controller);
    return detected_controllers;
}

REGISTER_HID_DETECTOR_PU("Ant Esports MK4100", DetectAntEsportsMK4100Controllers,
                          ANT_ESPORTS_VID, ANT_ESPORTS_MK4100_PID,
                          ANT_ESPORTS_RGB_USAGE_PAGE, ANT_ESPORTS_RGB_USAGE);

/*---------------------------------------------------------*\
| IlovbeeB87ControllerDetect.cpp                           |
|                                                           |
|   Detector for ilovbee B87 keyboard                       |
|                                                           |
|   w568w                                       18 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <unordered_set>
#include "DetectionManager.h"
#include "IlovbeeB87Controller.h"
#include "LogManager.h"
#include "RGBController_IlovbeeB87.h"

#define ILOVBEE_VID             0x258A
#define ILOVBEE_B87_PID         0x0199
#define ILOVBEE_B87_INTERFACE   1

// Interface 1 exposes several top-level collections through one hidraw path.
static std::unordered_set<std::string> used_paths;

void ResetIlovbeeB87ControllerPaths()
{
    used_paths.clear();
}

DetectedControllers DetectIlovbeeB87Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(used_paths.find(info->path) != used_paths.end())
    {
        return(detected_controllers);
    }

    hid_device* dev = hid_open_path(info->path);

    if(!dev)
    {
        LOG_WARNING("[%s] Could not open HID path %s: %ls", name.c_str(), info->path, hid_error(NULL));
        return(detected_controllers);
    }

    IlovbeeB87Controller* controller = new IlovbeeB87Controller(dev, *info, name);

    if(!controller->Initialize())
    {
        LOG_ERROR("[%s] Device did not return a valid B87 lighting configuration", name.c_str());
        delete controller;
        return(detected_controllers);
    }

    LOG_INFO("[%s] Detected ilovbee B87 at HID path %s", name.c_str(), info->path);

    RGBController_IlovbeeB87* rgb_controller = new RGBController_IlovbeeB87(controller);
    detected_controllers.push_back(rgb_controller);
    used_paths.insert(info->path);

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_I("ilovbee B87", DetectIlovbeeB87Controllers, ILOVBEE_VID, ILOVBEE_B87_PID, ILOVBEE_B87_INTERFACE);
REGISTER_PRE_DETECTION_HOOK(ResetIlovbeeB87ControllerPaths);

/*---------------------------------------------------------*\
| AsusLegacyACPIControllerDetect_Windows.cpp                |
|                                                           |
|   Detector for legacy ASUS motherboard ACPI lighting      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "AsusLegacyACPIController_Windows.h"
#include "LogManager.h"
#include "RGBController_AsusLegacyACPI_Windows.h"

DetectedControllers DetectAsusLegacyACPIControllers()
{
    DetectedControllers detected_controllers;
    LOG_INFO("[AsusLegacyACPI] Detector started");

    AsusLegacyACPIController_Windows *controller = new AsusLegacyACPIController_Windows();

    if(!controller->IsSupported())
    {
        LOG_INFO("[AsusLegacyACPI] Controller not available");
        delete controller;
        return detected_controllers;
    }

    RGBController *rgb_controller = new RGBController_AsusLegacyACPI_Windows(controller);
    detected_controllers.push_back(rgb_controller);
    LOG_INFO("[AsusLegacyACPI] Controller registered");
    return detected_controllers;
}

REGISTER_DETECTOR("ASUS Maximus VIII Formula Lighting", DetectAsusLegacyACPIControllers);

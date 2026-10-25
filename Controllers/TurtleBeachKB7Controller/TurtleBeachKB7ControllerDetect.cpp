/*---------------------------------------------------------*\
| TurtleBeachKB7ControllerDetect.cpp                        |
|                                                           |
|   Detector for Turtle Beach Command Series KB7 keyboard   |
|                                                           |
|   Joe Peterson (joevpeterson@gmail.com)       04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "RGBController_TurtleBeachKB7.h"
#include "TurtleBeachKB7Controller.h"

DetectedControllers DetectTurtleBeachKB7Controllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        TurtleBeachKB7Controller*      controller      = new TurtleBeachKB7Controller(dev, info->path);
        RGBController_TurtleBeachKB7*  rgb_controller  = new RGBController_TurtleBeachKB7(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_I("Turtle Beach Command Series KB7", DetectTurtleBeachKB7Controllers, TURTLE_BEACH_VID, TURTLE_BEACH_KB7_PID, TURTLE_BEACH_KB7_INTERFACE);

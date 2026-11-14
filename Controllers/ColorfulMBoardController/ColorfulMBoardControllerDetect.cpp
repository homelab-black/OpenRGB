/*---------------------------------------------------------*\
| ColorfulMBoardControllerDetect.cpp                       |
|                                                           |
|   Detector for Colorful USB motherboard lighting          |
|                                                           |
|   OpenRGB contributors                       08 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ColorfulMBoardController.h"
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController_ColorfulMBoard.h"

#define COLORFUL_VID                        0x2F4C
#define COLORFUL_IGAME_B850I_MINI_OC_PID     0x1020
#define COLORFUL_IGAME_B850I_INTERFACE      1
#define COLORFUL_MBOARD_USAGE_PAGE          0xFF01
#define COLORFUL_MBOARD_USAGE               0x01

DetectedControllers DetectColorfulMBoardControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        ColorfulMBoardController*     controller     = new ColorfulMBoardController(dev, info->path, name);
        RGBController_ColorfulMBoard* rgb_controller = new RGBController_ColorfulMBoard(controller);

        detected_controllers.push_back(rgb_controller);
    }
    else
    {
        LOG_WARNING("[ColorfulMBoardController] Failed to open %s at %s", name.c_str(), info->path);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| This board exposes the FF01 lighting collection on       |
| interface 1.  Older Colorful boards use interface 2.     |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Colorful iGame B850I MINI OC", DetectColorfulMBoardControllers, COLORFUL_VID, COLORFUL_IGAME_B850I_MINI_OC_PID, COLORFUL_IGAME_B850I_INTERFACE, COLORFUL_MBOARD_USAGE_PAGE, COLORFUL_MBOARD_USAGE);

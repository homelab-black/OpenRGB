/*---------------------------------------------------------*\
| GMK67ControllerDetect.cpp                                 |
|                                                           |
|   Detector for the GMK67 keyboard                         |
|                                                           |
|   NOTE: The GMK67 uses the same USB VID/PID (05AC:024F)   |
|   as the Keychron K3 V2. Both detectors will match a      |
|   connected keyboard, so both a "Keychron ..." and a      |
|   "GMK67" device may appear. Use the GMK67 entry for a    |
|   GMK67 keyboard.                                         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "GMK67Controller.h"
#include "RGBController_GMK67.h"

/*---------------------------------------------------------*\
| GMK67 USB identifiers                                     |
\*---------------------------------------------------------*/
#define GMK67_VID           0x05AC
#define GMK67_PID           0x024F
#define GMK67_INTERFACE     0
#define GMK67_USAGE_PAGE    0x0001
#define GMK67_USAGE         0x0006

DetectedControllers DetectGMK67Controllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        GMK67Controller*     controller     = new GMK67Controller(dev, info->path, name);
        RGBController_GMK67* rgb_controller  = new RGBController_GMK67(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("GMK67", DetectGMK67Controllers, GMK67_VID, GMK67_PID, GMK67_INTERFACE, GMK67_USAGE_PAGE, GMK67_USAGE);

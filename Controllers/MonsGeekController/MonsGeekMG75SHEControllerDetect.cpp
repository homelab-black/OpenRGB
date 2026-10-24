/*---------------------------------------------------------*\
| MonsGeekMG75SHEControllerDetect.cpp                       |
|                                                           |
|   Detector for the MonsGeek MG75S HE keyboard             |
|                                                           |
|   Sergey Tsai (mustang651)                 14 Sep 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "MonsGeekMG75SHEController.h"
#include "RGBController_MonsGeekMG75SHE.h"

#define MONSGEEK_VID                            0x3151
#define MONSGEEK_MG75S_HE_PID                   0x4035
#define MONSGEEK_MG75S_HE_RUNTIME_PID           0x5009
#define MONSGEEK_MG75S_HE_INTERFACE             2
#define MONSGEEK_MG75S_HE_USAGE_PAGE            0xFFFF
#define MONSGEEK_MG75S_HE_USAGE                 0x0002

DetectedControllers DetectMonsGeekMG75SHEControllers(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device* device = hid_open_path(info->path);

    if(device)
    {
        MonsGeekMG75SHEController* controller = new MonsGeekMG75SHEController(device, info->path);
        detected_controllers.push_back(new RGBController_MonsGeekMG75SHE(controller));
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("MonsGeek MG75S HE", DetectMonsGeekMG75SHEControllers, MONSGEEK_VID, MONSGEEK_MG75S_HE_PID, MONSGEEK_MG75S_HE_INTERFACE, MONSGEEK_MG75S_HE_USAGE_PAGE, MONSGEEK_MG75S_HE_USAGE);
REGISTER_HID_DETECTOR_IPU("MonsGeek MG75S HE", DetectMonsGeekMG75SHEControllers, MONSGEEK_VID, MONSGEEK_MG75S_HE_RUNTIME_PID, MONSGEEK_MG75S_HE_INTERFACE, MONSGEEK_MG75S_HE_USAGE_PAGE, MONSGEEK_MG75S_HE_USAGE);

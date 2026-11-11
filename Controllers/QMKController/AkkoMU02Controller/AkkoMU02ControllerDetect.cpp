/*---------------------------------------------------------*\
| AkkoMU02ControllerDetect.cpp                              |
|                                                           |
|   Detector for Akko MU02 VIA keyboards                    |
|                                                           |
|   OpenRGB contributors                        11 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "AkkoMU02Controller.h"
#include "RGBController_AkkoMU02.h"

DetectedControllers DetectAkkoMU02Controllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected;
    hid_device* dev = hid_open_path(info->path);
    if(!dev)
    {
        return detected;
    }
    AkkoMU02Controller* controller = new AkkoMU02Controller(dev, info->path);
    if(controller->GetSupported())
    {
        detected.push_back(new RGBController_AkkoMU02(controller));
    }
    else
    {
        delete controller;
    }
    return detected;
}

REGISTER_HID_DETECTOR_PU("Akko MU02 ISO", DetectAkkoMU02Controllers, 0x342D, 0xE4CC, 0xFF60, 0x61);

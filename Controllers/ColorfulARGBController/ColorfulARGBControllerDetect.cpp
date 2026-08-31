/*---------------------------------------------------------*\
| ColorfulARGBControllerDetect.cpp                           |
|                                                             |
|   Detector for the onboard ARGB header on the Colorful      |
|   B650M Battle-AX Plus V14 (VID 0x2F4C / PID 0x1000, HID    |
|   interface #2).                                             |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-only                    |
\*---------------------------------------------------------*/

#include "ColorfulARGBController.h"
#include "DetectionManager.h"
#include "RGBController_ColorfulARGB.h"
#include <hidapi.h>

#define COLORFUL_ARGB_VID          0x2F4C
#define COLORFUL_ARGB_PID          0x1000
#define COLORFUL_ARGB_INTERFACE    2

std::vector<RGBController*> DetectColorfulARGBControllers(hid_device_info* info, const std::string& /*name*/)
{
    std::vector<RGBController*> controllers;

    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        ColorfulARGBController*     controller     = new ColorfulARGBController(dev, info->path);
        RGBController_ColorfulARGB* rgb_controller = new RGBController_ColorfulARGB(controller);

        controllers.push_back(rgb_controller);
    }

    return(controllers);
}

REGISTER_HID_DETECTOR_I("Colorful B650M Battle-AX Plus ARGB", DetectColorfulARGBControllers, COLORFUL_ARGB_VID, COLORFUL_ARGB_PID, COLORFUL_ARGB_INTERFACE);

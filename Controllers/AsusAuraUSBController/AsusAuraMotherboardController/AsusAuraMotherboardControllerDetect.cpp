/*---------------------------------------------------------*\
| AsusAuraMotherboardControllerDetect.cpp                   |
|                                                           |
|   Detector for ASUS Aura motherboard devices              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include <dmiinfo.h>
#include "DetectionManager.h"
#include "AsusAuraMotherboardController.h"
#include "RGBController_AsusAuraMotherboard.h"

/*---------------------------------------------------------*\
| ASUS VID                                                  |
\*---------------------------------------------------------*/
#define AURA_USB_VID                                   0x0B05

/*---------------------------------------------------------*\
| Motherboard PIDs                                          |
\*---------------------------------------------------------*/
#define AURA_MOTHERBOARD_B850F_GAMING_WIFI7_NEO_PID    0x1CD2

DetectedControllers DetectAsusAuraMotherboard(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        DMIInfo dmi;
        AuraMotherboardController*     controller           = new AuraMotherboardController(dev, info->path, info->product_id, name + " - " + dmi.getMainboard());
        RGBController_AuraMotherboard* rgb_controller       = new RGBController_AuraMotherboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Motherboard Detectors                                       |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Asus Aura Motherboard", DetectAsusAuraMotherboard,   AURA_USB_VID, AURA_MOTHERBOARD_B850F_GAMING_WIFI7_NEO_PID,  1, 0xFF0A, 1);
REGISTER_HID_DETECTOR_IPU("FAKE", DetectAsusAuraMotherboard, 0x145f, 0x0248,  1, 0xFFA0, 1);


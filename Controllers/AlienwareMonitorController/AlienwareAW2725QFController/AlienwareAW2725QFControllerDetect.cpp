/*---------------------------------------------------------*\
| AlienwareAW2725QFControllerDetect.cpp                     |
|                                                           |
|   Detector for Alienware AW2725QF monitor                 |
|                                                           |
|   Ferréol DUBOIS COLI (Fefe_du_973)           23 Jan 2025 |
|   Jingcheng Tang (at1289)                     08 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "AlienwareAW2725QFController.h"
#include "DetectionManager.h"
#include "RGBController_AlienwareAW2725QF.h"

/*---------------------------------------------------------*\
| Alienware Vendor ID                                       |
\*---------------------------------------------------------*/
#define ALIENWARE_VID                               0x187C

/*---------------------------------------------------------*\
| Alienware Product ID                                       |
\*---------------------------------------------------------*/
#define ALIENWARE_AW2725QF_PID                      0x1015
#define ALIENWARE_USAGE_PAGE                        0xFFDA
#define ALIENWARE_USAGE                             0x00DA

DetectedControllers DetectAlienwareAW2725QFControllers(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        AlienwareAW2725QFController*     controller     = new AlienwareAW2725QFController(dev, info->path);
        RGBController_AlienwareAW2725QF* rgb_controller = new RGBController_AlienwareAW2725QF(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_PU("Alienware AW2725QF", DetectAlienwareAW2725QFControllers, ALIENWARE_VID, ALIENWARE_AW2725QF_PID, ALIENWARE_USAGE_PAGE, ALIENWARE_USAGE);

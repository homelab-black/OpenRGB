/*---------------------------------------------------------*\
| EVisionV3KeyboardControllerDetect.cpp                     |
|                                                           |
|   Detector for EVision "RDR" tri-mode keyboards (VIA)     |
|                                                           |
|   Jan Imrich (imrija)                         05 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "EVisionV3KeyboardController.h"
#include "RGBController_EVisionV3Keyboard.h"

/*-----------------------------------------------------*\
| VIA raw HID usage page and usage                      |
\*-----------------------------------------------------*/
#define EVISION_V3_USAGE_PAGE       0xFF60
#define EVISION_V3_USAGE            0x61

/******************************************************************************************\
*                                                                                          *
*   DetectEVisionV3Keyboards                                                               *
*                                                                                          *
*       Tests the USB address to see if an EVision "RDR" tri-mode keyboard exists there.   *
*                                                                                          *
*       The 320F:5055 product ID is shared by the whole firmware family, so only           *
*       register the controller when the device answers the VIA protocol handshake.        *
*                                                                                          *
\******************************************************************************************/

void DetectEVisionV3Keyboards(hid_device_info* info, const std::string& /*name*/)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        EVisionV3KeyboardController* controller = new EVisionV3KeyboardController(dev, info->path);

        if(controller->GetSupported())
        {
            RGBController_EVisionV3Keyboard* rgb_controller = new RGBController_EVisionV3Keyboard(controller);

            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
        else
        {
            delete controller;
        }
    }
}

REGISTER_HID_DETECTOR_IPU("EVision Keyboard 320F:5055", DetectEVisionV3Keyboards, EVISION_V3_VID, EVISION_V3_RDR_PID, 1, EVISION_V3_USAGE_PAGE, EVISION_V3_USAGE);

/*---------------------------------------------------------*\
| ChilkeyKeyboardControllerDetect.cpp                       |
|                                                           |
|   Detector for Chilkey Slice75 HE keyboard                |
|                                                           |
|   Aaron Coyco                                 28 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "ChilkeyKeyboardController.h"
#include "RGBController_ChilkeyKeyboard.h"

/*---------------------------------------------------------*\
| Chilkey vendor ID                                         |
\*---------------------------------------------------------*/
#define CHILKEY_KEYBOARD_VID                    0x1CA3

/*---------------------------------------------------------*\
| Product IDs                                                |
\*---------------------------------------------------------*/
#define CHILKEY_SLICE75_HE_ANSI_PID             0x0701

DetectedControllers DetectChilkeyKeyboardControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        ChilkeyKeyboardController*     controller      = new ChilkeyKeyboardController(dev, info, name);
        RGBController_ChilkeyKeyboard* rgb_controller  = new RGBController_ChilkeyKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Chilkey Slice75 HE (ANSI)", DetectChilkeyKeyboardControllers, CHILKEY_KEYBOARD_VID, CHILKEY_SLICE75_HE_ANSI_PID, 2, 0xFFA0, 0x0001);

/*---------------------------------------------------------*\
| SonixGamingKeyboardControllerDetect.cpp                   |
|                                                           |
|   Detector for Sonix based keyboards using the "Gaming    |
|   Force Z" software (CZC.Gaming Revenant)                 |
|                                                           |
|   Roman Ondráček                              05 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "SonixGamingKeyboardController.h"
#include "RGBController_SonixGamingKeyboard.h"

/*---------------------------------------------------------*\
| Sonix vendor ID                                           |
\*---------------------------------------------------------*/
#define SONIX_VID                                   0x0C45

/*---------------------------------------------------------*\
| Keyboard product IDs                                      |
|   NOTE: The vendor reuses this ID for many keyboards with |
|   different layouts, so the device is named generically  |
\*---------------------------------------------------------*/
#define SONIX_GAMING_KEYBOARD_8006_PID              0x8006

DetectedControllers DetectSonixGamingKeyboards(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        SonixGamingKeyboardController*      controller      = new SonixGamingKeyboardController(dev, info->path, name);
        RGBController_SonixGamingKeyboard*  rgb_controller  = new RGBController_SonixGamingKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| The lighting feature report lives on the boot keyboard    |
| interface (interface 0, usage page 0x01, usage 0x06)      |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Sonix Gaming Keyboard 0C45:8006", DetectSonixGamingKeyboards, SONIX_VID, SONIX_GAMING_KEYBOARD_8006_PID, 0, 0x0001, 0x06);

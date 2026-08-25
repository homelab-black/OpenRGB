/*---------------------------------------------------------*\
| CosmicByteFireflyKeyboardDetect.cpp                       |
|                                                           |
|   Detector for Cosmic Byte Firefly keyboard               |
|                                                           |
|   Akankshya Das                               25 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "CosmicByteFireflyKeyboardController.h"
#include "RGBcontroller_CosmicByteFireflyKeyboard.h"

/*---------------------------------------------------------*\
| Cosmic Byte Firefly Keyboard IDs                          |
\*---------------------------------------------------------*/
#define FIREFLY_VID          0x04D9
#define FIREFLY_PID          0xA1CD

DetectedControllers DetectCosmicByteFireflyKeyboardControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        CosmicByteFireflyKeyboardController*     controller     = new CosmicByteFireflyKeyboardController(dev, info->path, info->product_id, name);
        RGBcontroller_CosmicByteFireflyKeyboard* rgb_controller = new RGBcontroller_CosmicByteFireflyKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_I("Cosmic Byte Firefly Keyboard", DetectCosmicByteFireflyKeyboardControllers, FIREFLY_VID, FIREFLY_PID, 2);
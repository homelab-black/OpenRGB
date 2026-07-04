/*---------------------------------------------------------*\
| NZXTFunctionKeyboardControllerDetect.cpp                         |
|                                                           |
|   Detector for Clevo per-key RGB keyboard (ITE 8291)      |
|                                                           |
|   Kyle Cascade (kyle@cascade.family)          16 Jan 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "Detector.h"
#include "NZXTFunctionKeyboardController.h"
#include "RGBController_NZXTFunctionKeyboard.h"
#include "RGBController.h"
#include "libusb.h"
#include <hidapi.h>

void DetectNZXTFunctionKeyboardControllers(hid_device_info* info, const std::string& name)
{
    NZXTFunctionKeyboardController*     controller     = new NZXTFunctionKeyboardController(*info);
    RGBController_NZXTFunctionKeyboard* rgb_controller = new RGBController_NZXTFunctionKeyboard(controller);
    rgb_controller->name                         = name;

    ResourceManager::get()->RegisterRGBController(rgb_controller);
}

REGISTER_HID_DETECTOR_IPU("NZXT Function Keyboard", DetectNZXTFunctionKeyboardControllers, 0x1e71, 0x2106, 0, 0x001, 0x006);

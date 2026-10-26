/*---------------------------------------------------------*\
| DasKeyboardX50ControllerDetect.cpp                        |
|                                                           |
|   Detector for the Das Keyboard X50 RGB                  |
|                                                           |
|   Sergio Presa                                15 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "DasKeyboardX50Controller.h"
#include "RGBController_DasKeyboardX50.h"
#include "StringUtils.h"
#include "hidapi_wrapper.h"

#define DAS_KEYBOARD_VID                0x24F0
#define DAS_KEYBOARD_X50_PID            0x202B

DetectedControllers DetectDasKeyboardX50(hidapi_wrapper wrapper, hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         device = wrapper.hid_open_path(info->path);

    if(device != nullptr)
    {
        std::string serial = info->serial_number != nullptr ? StringUtils::wchar_to_string(info->serial_number) : "";
        DasKeyboardX50Controller*     controller     = new DasKeyboardX50Controller(wrapper, device, info->path, serial);
        RGBController_DasKeyboardX50* rgb_controller = new RGBController_DasKeyboardX50(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_WRAPPED_DETECTOR_I("Das Keyboard X50 RGB", DetectDasKeyboardX50, DAS_KEYBOARD_VID, DAS_KEYBOARD_X50_PID, 2);

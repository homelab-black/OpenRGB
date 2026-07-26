/*---------------------------------------------------------*\
| RGBController_AcerNitroHidKeyboard.cpp                    |
|                                                           |
|   Detector for Acer Nitro HID Keyboard                    |
|                                                           |
|   Marek Pokropiński                            26.07.2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <vector>
#include "DetectionManager.h"
#include "AcerNitroHidKeyboardController.h"
#include "i2c_smbus.h"
#include "LogManager.h"
#include "pci_ids.h"
#include "RGBController_AcerNitroHidKeyboard.h"

using namespace std::chrono_literals;

/*---------------------------------------------------------*\
| HID vendor and product IDs for ENEK EC in Acer Nitro      |
\*---------------------------------------------------------*/

#define ACER_ENEC_VID         0x0CF2
#define ACER_ENEC_PID         0x5130

DetectedControllers DetectAcerHIDControllers(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);
    if(dev)
    {
        AcerNitroHidKeyboardController* controller = new AcerNitroHidKeyboardController(dev, info->path);
        RGBController_AcerNitroHidKeyboard* rgb_controller = new RGBController_AcerNitroHidKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
        LOG_INFO("[AcerNitroHidKeyboardControllerDetect] Detected AcerNitroHidKeyboardController at %s", info->path);
    }
    else
    {
        LOG_WARNING("[AcerNitroHidKeyboardControllerDetect] Failed to open AcerNitroHidKeyboardController at %s", info->path);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR("AcerNitroHidKeyboardController", DetectAcerHIDControllers, ACER_ENEC_VID, ACER_ENEC_PID);
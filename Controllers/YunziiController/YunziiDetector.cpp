/*---------------------------------------------------------*\
| YunziiDetector.cpp                                        |
|                                                           |
|   Detector for Yunzii keyboards                           |
|                                                           |
|   Kosta A. (kostaarvanitis)                   23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include <sstream>
#include <iomanip>
#include <string>

#include "Detector.h"
#include "RGBController.h"
#include "LogManager.h"
#include "YunziiYZ75KeyboardController.h"
#include "RGBController_YunziiYZ75Keyboard.h"

#define YUNZII_VID                      0x3151

#define YUNZI_YZ75_KEYBOARD_PID         0x5025 // Gaming Keyboard

/******************************************************************************************\
*                                                                                          *
*   DetectYunziiYZ75Keyboard                                                               *
*                                                                                          *
*       Tests the USB address to see if a Thermalright controller exists there.            *
*                                                                                          *
\******************************************************************************************/

static void DetectYunziiYZ75Keyboard(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        YunziiYZ75KeyboardController*      controller = new YunziiYZ75KeyboardController(dev, info, name);
        RGBController_YunziiYZ75Keyboard*  rgb_controller = new RGBController_YunziiYZ75Keyboard(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
    else
    {
        LOG_ERROR("[%s] Failure to open!", name.c_str());
    }
}

REGISTER_HID_DETECTOR_IPU("Yunzii YZ75", DetectYunziiYZ75Keyboard, YUNZII_VID, YUNZI_YZ75_KEYBOARD_PID, 2, 0xFFFF, 0x02);

/*---------------------------------------------------------*\
| DrevoKeyboardControllerDetect.cpp                         |
|                                                           |
|   Detector for Drevo keyboards                            |
|                                                           |
|   Dennis Blokland                             02 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "DrevoKeyboardController.h"
#include "RGBController_DrevoKeyboard.h"

/*---------------------------------------------------------*\
| Drevo vendor ID                                           |
\*---------------------------------------------------------*/
#define DREVO_VID                                   0x0416

/*---------------------------------------------------------*\
| Drevo keyboard product IDs                                |
\*---------------------------------------------------------*/
#define DREVO_TYRFING_V2_PID                        0xA0F8

/*---------------------------------------------------------*\
| Interface 1 exposes seven top level collections.  The one |
|   that accepts the lighting report is the Generic Desktop |
|   page with an undefined usage, so filter on it to avoid  |
|   registering the device once per collection              |
\*---------------------------------------------------------*/
#define DREVO_LIGHTING_USAGE_PAGE                   0x0001
#define DREVO_LIGHTING_USAGE                        0x0000
#define DREVO_LIGHTING_INTERFACE                    1

DetectedControllers DetectDrevoKeyboard(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        DrevoKeyboardController*     controller     = new DrevoKeyboardController(dev, info->path, name);
        RGBController_DrevoKeyboard* rgb_controller = new RGBController_DrevoKeyboard(controller);

        detected_controllers.push_back(rgb_controller);
    }
    else
    {
        LOG_ERROR("[%s] Unable to open %s, check that the udev rules are installed", name.c_str(), info->path);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Drevo Tyrfing V2", DetectDrevoKeyboard, DREVO_VID, DREVO_TYRFING_V2_PID, DREVO_LIGHTING_INTERFACE, DREVO_LIGHTING_USAGE_PAGE, DREVO_LIGHTING_USAGE);

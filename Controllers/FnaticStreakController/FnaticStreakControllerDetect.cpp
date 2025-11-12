/*---------------------------------------------------------*\
| FnaticStreakControllerDetect.cpp                          |
|                                                           |
|   Detector for Fnatic Streak keyboard                     |
|                                                           |
|   Based on leddy by Hanna Czenczek                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "FnaticStreakController.h"
#include "RGBController_FnaticStreak.h"
#include <hidapi.h>

/******************************************************************************************\
*                                                                                          *
*   DetectFnaticStreakControllers                                                          *
*                                                                                          *
*       Tests the USB address to see if a Fnatic Streak keyboard controller exists there. *
*                                                                                          *
\******************************************************************************************/

void DetectFnaticStreakControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        FnaticStreakController*     controller     = new FnaticStreakController(dev, info->path, info->product_id, name);
        RGBController_FnaticStreak* rgb_controller = new RGBController_FnaticStreak(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}   /* DetectFnaticStreakControllers() */

/*---------------------------------------------------------*\
| Register Fnatic Streak keyboards                         |
| Interface 1 is the RGB control interface                  |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("Fnatic Streak",     DetectFnaticStreakControllers, FNATIC_VID, FNATIC_STREAK_PID,         1);
REGISTER_HID_DETECTOR_I("Fnatic miniStreak", DetectFnaticStreakControllers, FNATIC_VID, FNATIC_MINISTREAK_PID,     1);

/*---------------------------------------------------------*\
| AlienwareControllerDetect.cpp                             |
|                                                           |
|   Detector for Dell Alienware RGB USB controller          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AlienwareController.h"
#include "DetectionManager.h"
#include "RGBController_Alienware.h"

/*---------------------------------------------------------*\
| Alienware vendor ID                                       |
\*---------------------------------------------------------*/
#define ALIENWARE_VID                               0x187C

/*---------------------------------------------------------*\
| Alienware product ID                                      |
\*---------------------------------------------------------*/
#define ALIENWARE_G_SERIES_PID1                      0x0550
#define ALIENWARE_G_SERIES_PID2                      0x0551
#define ALIENWARE_AURORA_ACT1250_PID                 0x0550

/*---------------------------------------------------------*\
| The product ID does not identify the model, only the      |
| platform ID from the controller does                      |
\*---------------------------------------------------------*/
static DetectedControllers DetectAlienwareByPlatform(hid_device_info* info, const std::string& name, bool act1250)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        AlienwareController* controller = new AlienwareController(dev, *info, name);

        if(controller->HasDirectMode() == act1250)
        {
            RGBController_Alienware* rgb_controller = new RGBController_Alienware(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            delete controller;
            hid_close(dev);
        }
    }

    return(detected_controllers);
}

DetectedControllers DetectAlienwareControllers(hid_device_info* info, const std::string& name)
{
    return(DetectAlienwareByPlatform(info, name, false));
}

DetectedControllers DetectAlienwareACT1250Controllers(hid_device_info* info, const std::string& name)
{
    return(DetectAlienwareByPlatform(info, name, true));
}

/*---------------------------------------------------------*\
| Detection of a device stops at the first detector that    |
| finds it, so keep Dell G Series first.  Its devices then  |
| do not reach the ACT1250 detector                         |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR("Dell G Series LED Controller", DetectAlienwareControllers, ALIENWARE_VID, ALIENWARE_G_SERIES_PID1);
REGISTER_HID_DETECTOR("Dell G Series LED Controller", DetectAlienwareControllers, ALIENWARE_VID, ALIENWARE_G_SERIES_PID2);
REGISTER_HID_DETECTOR("Alienware Aurora ACT1250", DetectAlienwareACT1250Controllers, ALIENWARE_VID, ALIENWARE_AURORA_ACT1250_PID);

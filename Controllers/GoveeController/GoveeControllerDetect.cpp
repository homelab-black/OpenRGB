/*---------------------------------------------------------*\
| GoveeControllerDetect.cpp                                 |
|                                                           |
|   Detector for Govee wireless lighting devices            |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      01 Dec 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "DetectionManager.h"
#include "GoveeController.h"
#include "RGBController.h"
#include "RGBController_Govee.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

DetectedControllers DetectGoveeControllers()
{
    DetectedControllers detected_controllers;
    json                govee_settings;

    /*-----------------------------------------------------*\
    | Get Govee settings from settings manager              |
    \*-----------------------------------------------------*/
    govee_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("GoveeDevices");

    /*-----------------------------------------------------*\
    | If the Govee settings contains devices, process       |
    \*-----------------------------------------------------*/
    if(govee_settings.contains("devices"))
    {
        for(unsigned int device_idx = 0; device_idx < govee_settings["devices"].size(); device_idx++)
        {
            if(govee_settings["devices"][device_idx].contains("ip"))
            {
                std::string govee_ip  = govee_settings["devices"][device_idx]["ip"];

                GoveeController*     controller     = new GoveeController(govee_ip);
                RGBController_Govee* rgb_controller = new RGBController_Govee(controller);

                detected_controllers.push_back(rgb_controller);
            }
        }
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Govee", DetectGoveeControllers);

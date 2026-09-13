/*---------------------------------------------------------*\
| YeelightControllerDetect.cpp                              |
|                                                           |
|   Detector for Yeelight                                   |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController_Yeelight.h"
#include "SettingsManager.h"
#include "YeelightController.h"

DetectedControllers DetectYeelightControllers()
{
    DetectedControllers detected_controllers;
    json                yeelight_settings;

    /*-----------------------------------------------------*\
    | Get Yeelight settings from settings manager           |
    \*-----------------------------------------------------*/
    yeelight_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("YeelightDevices");

    /*-----------------------------------------------------*\
    | If the Yeelight settings contains devices, process    |
    \*-----------------------------------------------------*/
    if(yeelight_settings.contains("devices"))
    {
        for(unsigned int device_idx = 0; device_idx < yeelight_settings["devices"].size(); device_idx++)
        {
            std::string yeelight_host_ip;

            if(yeelight_settings["devices"][device_idx].contains("host_ip"))
            {
                yeelight_host_ip = yeelight_settings["devices"][device_idx]["host_ip"];
            }

            if(yeelight_settings["devices"][device_idx].contains("ip"))
            {
                std::string yeelight_ip = yeelight_settings["devices"][device_idx]["ip"];
                yeelight_protocol protocol = YeelightController::GetProtocolFromSettings(yeelight_settings["devices"][device_idx]);

                YeelightController*     controller     = new YeelightController(yeelight_ip, yeelight_host_ip, protocol);
                RGBController_Yeelight* rgb_controller = new RGBController_Yeelight(controller);

                detected_controllers.push_back(rgb_controller);
            }
        }
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Yeelight", DetectYeelightControllers);

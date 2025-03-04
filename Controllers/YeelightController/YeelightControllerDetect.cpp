/*---------------------------------------------------------*\
| YeelightControllerDetect.cpp                              |
|                                                           |
|   Detector for Yeelight                                   |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "YeelightController.h"
#include "RGBController_Yeelight.h"
#include "SettingsManager.h"

/******************************************************************************************\
*                                                                                          *
*   DetectYeelightControllers                                                              *
*                                                                                          *
*       Detect Yeelight devices                                                            *
*                                                                                          *
\******************************************************************************************/

void DetectYeelightControllers()
{
    json                    yeelight_settings;

    /*-------------------------------------------------*\
    | Get Yeelight settings from settings manager    |
    \*-------------------------------------------------*/
    yeelight_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("YeelightDevices");

    /*-------------------------------------------------*\
    | If the Yeelight settings contains devices, process|
    \*-------------------------------------------------*/
    if(yeelight_settings.contains("devices"))
    {
        for(unsigned int device_idx = 0; device_idx < yeelight_settings["devices"].size(); device_idx++)
        {

            if(yeelight_settings["devices"][device_idx].contains("ip"))
            {
                std::string yeelight_ip = yeelight_settings["devices"][device_idx]["ip"];
                std::string token;

                if(yeelight_settings["devices"][device_idx].contains("token"))
                {
                    token = yeelight_settings["devices"][device_idx]["token"];
                }

                YeelightController*     controller     = new YeelightController(yeelight_ip, token);
                RGBController_Yeelight* rgb_controller = new RGBController_Yeelight(controller);

                ResourceManager::get()->RegisterRGBController(rgb_controller);
            }
        }
    }

}   /* DetectYeelightControllers() */

REGISTER_DETECTOR("Yeelight", DetectYeelightControllers);

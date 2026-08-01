/*---------------------------------------------------------*\
| TuyaControllerDetect.cpp                                   |
|                                                             |
|   Detector for Tuya-based WiFi RGB lights                   |
|                                                             |
|   Tuya devices (and OEM-branded devices built on the Tuya  |
|   platform, such as LEDVANCE SMART+) do not broadcast       |
|   themselves for automatic discovery in a way that safely  |
|   identifies them as lighting devices, and control requires |
|   a per-device local_key that can only be obtained from the |
|   vendor's cloud account. Devices are therefore configured  |
|   manually, the same approach already used for Govee.       |
|                                                              |
|   settings.json:                                             |
|   {                                                          |
|       "TuyaDevices":                                         |
|       {                                                      |
|           "devices":                                         |
|           [                                                  |
|               {                                              |
|                   "ip":  "192.168.1.50",                     |
|                   "id":  "<Tuya device id>",                 |
|                   "key": "<Tuya local_key>"                  |
|               }                                               |
|           ]                                                   |
|       }                                                        |
|   }                                                             |
|                                                                  |
|   This file is part of the OpenRGB project                      |
|   SPDX-License-Identifier: GPL-2.0-or-later                     |
\*---------------------------------------------------------*/

#include <string>
#include <vector>
#include "DetectionManager.h"
#include "TuyaController.h"
#include "RGBController.h"
#include "RGBController_Tuya.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

DetectedControllers DetectTuyaControllers()
{
    DetectedControllers detected_controllers;
    json                tuya_settings;

    tuya_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("TuyaDevices");

    if(tuya_settings.contains("devices"))
    {
        for(unsigned int device_idx = 0; device_idx < tuya_settings["devices"].size(); device_idx++)
        {
            json& device_settings = tuya_settings["devices"][device_idx];

            if(device_settings.contains("ip") && device_settings.contains("id") && device_settings.contains("key"))
            {
                std::string tuya_ip  = device_settings["ip"];
                std::string tuya_id  = device_settings["id"];
                std::string tuya_key = device_settings["key"];

                if(tuya_key.length() != 16)
                {
                    continue;
                }

                TuyaController*     controller     = new TuyaController(tuya_ip, tuya_id, tuya_key);
                RGBController_Tuya* rgb_controller = new RGBController_Tuya(controller);

                detected_controllers.push_back(rgb_controller);
            }
        }
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Tuya", DetectTuyaControllers);

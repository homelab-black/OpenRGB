/*---------------------------------------------------------*\
| NanoleafControllerDetect.cpp                              |
|                                                           |
|   Detector for Nanoleaf                                   |
|                                                           |
|   Nikita Rushmanov                            13 Jan 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "RGBController_Nanoleaf.h"
#include "SettingsManager.h"

DetectedControllers DetectNanoleafControllers()
{
    DetectedControllers detected_controllers;

    NanoleafSettings nanoleaf_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<NanoleafSettings>(NANOLEAF_CFG_KEY);

    /*-------------------------------------------------*\
    | using foreach loop since we iterate through a map |
    \*-------------------------------------------------*/
    for(const auto& [location, device] : nanoleaf_settings.devices)
    {
        try
        {
            RGBController_Nanoleaf* rgb_controller = new RGBController_Nanoleaf(device.ip, device.port, device.auth_token);
            detected_controllers.push_back(rgb_controller);
        }
        catch(...)
        {
            LOG_DEBUG("[Nanoleaf] Could not connect to device at %s:%d using auth_token %s", device.ip.c_str(), device.port, device.auth_token.c_str());
        }
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Nanoleaf", DetectNanoleafControllers);

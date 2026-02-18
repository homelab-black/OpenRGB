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
#include "ManualDeviceSettings.h"

DetectedControllers DetectYeelightControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get Yeelight settings from settings manager           |
    \*-----------------------------------------------------*/
    ManualDeviceSettings<YeelightDevice> yeelight_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<ManualDeviceSettings<YeelightDevice>>(YEELIGHT_CFG_KEY);

    /*-----------------------------------------------------*\
    | If the Yeelight settings contains devices, process    |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < yeelight_settings.devices.size(); device_idx++)
    {
        const YeelightDevice& device = yeelight_settings.devices[device_idx];
        YeelightController* controller = new YeelightController(
            device.ip,
            device.host_ip,
            device.music_mode
        );
        RGBController_Yeelight* rgb_controller = new RGBController_Yeelight(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Yeelight", DetectYeelightControllers);

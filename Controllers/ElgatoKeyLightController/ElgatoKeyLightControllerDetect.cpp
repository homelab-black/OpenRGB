/*---------------------------------------------------------*\
| ElgatoKeyLightControllerDetect.cpp                        |
|                                                           |
|   Detector for Elgato Key Light                           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "ElgatoKeyLightController.h"
#include "ResourceManager.h"
#include "RGBController_ElgatoKeyLight.h"
#include "SettingsManager.h"
#include "ElgatoSettings.h"


DetectedControllers DetectElgatoKeyLightControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get KeyLight settings from settings manager           |
    \*-----------------------------------------------------*/
    SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
    std::optional<ElgatoSettings> cfg_settings = sm->GetControllerSettings<ElgatoSettings>(ELGATO_KEY_LIGHT_CFG_KEY);
    ElgatoSettings settings = cfg_settings.value_or(ElgatoSettings {});
    /*-----------------------------------------------------*\
    | If the Elgato Key Light settings contains devices,    |
    | process                                               |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < settings.devices.size(); device_idx++)
    {
        std::string elgato_keylight_ip = settings.devices[device_idx].ip;

        ElgatoKeyLightController*     controller     = new ElgatoKeyLightController(elgato_keylight_ip);
        RGBController_ElgatoKeyLight* rgb_controller = new RGBController_ElgatoKeyLight(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("ElgatoKeyLight", DetectElgatoKeyLightControllers);

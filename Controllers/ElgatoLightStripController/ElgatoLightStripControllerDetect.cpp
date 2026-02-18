/*---------------------------------------------------------*\
| ElgatoLightStripControllerDetect.cpp                      |
|                                                           |
|   Detector for Elgato Light Strip                         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "ElgatoLightStripController.h"
#include "ResourceManager.h"
#include "RGBController_ElgatoLightStrip.h"
#include "SettingsManager.h"
#include "ElgatoSettings.h"

DetectedControllers DetectElgatoLightStripControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get LightStrip settings from settings manager         |
    \*-----------------------------------------------------*/
    SettingsManager* sm = ResourceManager::get()->GetSettingsManager();
    std::optional<ElgatoSettings> cfg_settings = sm->GetControllerSettings<ElgatoSettings>(ELGATO_LIGHTSTRIP_CFG_KEY);
    ElgatoSettings settings = cfg_settings.value_or(ElgatoSettings {});

    /*-----------------------------------------------------*\
    | If the Elgato Light Strip settings contains devices,  |
    | process                                               |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < settings.devices.size(); device_idx++)
    {
        std::string elgato_lightstrip_ip = settings.devices[device_idx].ip;

        ElgatoLightStripController*     controller     = new ElgatoLightStripController(elgato_lightstrip_ip);
        RGBController_ElgatoLightStrip* rgb_controller = new RGBController_ElgatoLightStrip(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Elgato Light Strip", DetectElgatoLightStripControllers);

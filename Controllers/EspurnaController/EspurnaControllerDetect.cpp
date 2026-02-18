/*---------------------------------------------------------*\
| EspurnaControllerDetect.cpp                               |
|                                                           |
|   Detctor for Espurna                                     |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      11 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "EspurnaController.h"
#include "ResourceManager.h"
#include "RGBController_Espurna.h"
#include "RGBController.h"
#include "SettingsManager.h"

DetectedControllers DetectEspurnaControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get Espurna settings from settings manager            |
    \*-----------------------------------------------------*/
    EspurnaSettings espurna_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<EspurnaSettings>(ESPURNA_CFG_KEY);

    /*-----------------------------------------------------*\
    | If the Espurna settings contains devices, process     |
    \*-----------------------------------------------------*/

    for(unsigned int device_idx = 0; device_idx < espurna_settings.devices.size(); device_idx++)
    {
        std::string ip = espurna_settings.devices[device_idx].ip;
        std::string port = espurna_settings.devices[device_idx].port;
        std::string apikey = espurna_settings.devices[device_idx].apikey;

        std::string value = ip + "," + port + "," + apikey;

        EspurnaController*     controller     = new EspurnaController();
        controller->Initialize((char *)value.c_str());

        RGBController_Espurna* rgb_controller = new RGBController_Espurna(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Espurna", DetectEspurnaControllers);

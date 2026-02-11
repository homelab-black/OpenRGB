/*---------------------------------------------------------*\
| PhilipsWizControllerDetect.cpp                            |
|                                                           |
|   Detector for Philips Wiz                                |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      03 Nov 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "PhilipsWizController.h"
#include "ResourceManager.h"
#include "RGBController_PhilipsWiz.h"
#include "SettingsManager.h"
#include "ManualDeviceSettings.h"

DetectedControllers DetectPhilipsWizControllers()
{
    DetectedControllers detected_controllers;
    /*-----------------------------------------------------*\
    | Get Philips Wiz settings from settings manager        |
    \*-----------------------------------------------------*/
    ManualDeviceSettings<PhilipsWizDevice> wiz_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<ManualDeviceSettings<PhilipsWizDevice>>(PHILIPZWIZ_CFG_KEY);

    /*-----------------------------------------------------*\
    | If the Wiz settings contains devices, process         |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < wiz_settings.devices.size(); device_idx++)
    {
        std::string wiz_ip  = wiz_settings.devices[device_idx].ip;
        bool wiz_cool        = wiz_settings.devices[device_idx].use_cool_white;
        bool wiz_warm        = wiz_settings.devices[device_idx].use_warm_white;
        std::string wiz_white_strategy = wiz_settings.devices[device_idx].selected_white_strategy;

        PhilipsWizController*     controller     = new PhilipsWizController(wiz_ip, wiz_cool, wiz_warm, wiz_white_strategy);
        RGBController_PhilipsWiz* rgb_controller = new RGBController_PhilipsWiz(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Philips Wiz", DetectPhilipsWizControllers);

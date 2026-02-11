/*---------------------------------------------------------*\
| LEDStripControllerDetect.cpp                              |
|                                                           |
|   Detector for serial LED strips                          |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      11 Dec 2016 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "LEDStripController.h"
#include "LogManager.h"
#include "ResourceManager.h"
#include "RGBController_LEDStrip.h"
#include "SettingsManager.h"
#include "ManualDeviceSettings.h"

DetectedControllers DetectLEDStripControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get LED Strip settings from settings manager          |
    \*-----------------------------------------------------*/
    ManualDeviceSettings<LEDStripDevice> ledstrip_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<ManualDeviceSettings<LEDStripDevice>>(LEDSTRIP_CFG_KEY);


    /*-----------------------------------------------------*\
    | If the LEDStrip settings contains devices, process    |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < ledstrip_settings.devices.size(); device_idx++)
    {
        LEDStripDevice dev = ledstrip_settings.devices[device_idx];

        if(dev.port.empty())
        {
            LOG_WARNING("[LEDStripController] port value cannot be left empty.");
            break;
        }

        if(dev.baud <= 0)
        {
            LOG_WARNING("[LEDStripController] baud value cannot be left empty.");
            break;
        }

        if(dev.num_leds <= 0)
        {
            LOG_WARNING("[LEDStripController] num_leds value cannot be left empty.");
            break;
        }

        std::string value = dev.port + "," + std::to_string(dev.baud) + "," + std::to_string(dev.num_leds);

        LEDStripController*     controller     = new LEDStripController(dev.name);
        controller->Initialize((char *)value.c_str(), dev.protocol);

        RGBController_LEDStrip* rgb_controller = new RGBController_LEDStrip(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("LED Strip", DetectLEDStripControllers);

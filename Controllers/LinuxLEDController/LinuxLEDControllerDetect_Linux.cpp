/*---------------------------------------------------------*\
| LinuxLEDControllerDetect_Linux.cpp                        |
|                                                           |
|   Detector for Linux sysfs LEDs                           |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      25 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController_LinuxLED_Linux.h"
#include "SettingsManager.h"

DetectedControllers DetectLinuxLEDControllers()
{
    DetectedControllers detected_controllers;

    /*-----------------------------------------------------*\
    | Get Linux LED settings from settings manager          |
    \*-----------------------------------------------------*/
    LinuxLEDSettings linux_led_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<LinuxLEDSettings>(LINUX_LED_DEVICE_CFG_KEY);

    /*-----------------------------------------------------*\
    | If the LinuxLED settings contains devices, process    |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < linux_led_settings.devices.size(); device_idx++)
    {
        std::string name        = linux_led_settings.devices[device_idx].name;
        std::string red_path    = linux_led_settings.devices[device_idx].red_path;
        std::string green_path  = linux_led_settings.devices[device_idx].green_path;
        std::string blue_path   = linux_led_settings.devices[device_idx].blue_path;
        std::string rgb_path    = linux_led_settings.devices[device_idx].rgb_path;


        LinuxLEDController*     controller     = new LinuxLEDController(name);
        controller->OpenRedPath(red_path);
        controller->OpenGreenPath(green_path);
        controller->OpenBluePath(blue_path);
        controller->OpenRgbPath(rgb_path);

        RGBController_LinuxLED* rgb_controller = new RGBController_LinuxLED(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("Linux LED", DetectLinuxLEDControllers);

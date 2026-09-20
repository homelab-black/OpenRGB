/*---------------------------------------------------------*\
| CorsairHydroH100iControllerDetect.cpp                     |
|                                                           |
|   Detector for Corsair H100i Hydro Series cooler          |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          20 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "CorsairHydroH100iController.h"
#include "RGBController_CorsairHydroH100i.h"

/*-----------------------------------------------------*\
| Corsair vendor ID                                     |
\*-----------------------------------------------------*/
#define CORSAIR_VID                     0x1B1C

/*-----------------------------------------------------*\
| Corsair H100i product ID                             |
\*-----------------------------------------------------*/
#define CORSAIR_H100I_PID               0x0C04

DetectedControllers DetectCorsairHydroH100iControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        CorsairHydroH100iController*     controller     = new CorsairHydroH100iController(dev, info->path, name);
        RGBController_CorsairHydroH100i* rgb_controller = new RGBController_CorsairHydroH100i(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR("Corsair H100i Series", DetectCorsairHydroH100iControllers, CORSAIR_VID, CORSAIR_H100I_PID);
REGISTER_CUSTOM_UDEV_RULE(corsair_hydro_h100i, "Corsair H100i Series", "SUBSYSTEMS==\"usb|hidraw\", ATTRS{idVendor}==\"1b1c\", ATTRS{idProduct}==\"0c04\", TAG+=\"uaccess\", TAG+=\"Corsair_Hydro_H100i\"");

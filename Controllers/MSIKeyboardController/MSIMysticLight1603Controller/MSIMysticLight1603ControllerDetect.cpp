/*---------------------------------------------------------*\
| MSIMysticLight1603ControllerDetect.cpp                    |
|                                                           |
|   Detector for MSI Mystic Light MS-1603 keyboards         |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "MSIMysticLight1603Controller.h"
#include "RGBController_MSIMysticLight1603.h"

/*---------------------------------------------------------*\
| USB IDs                                                   |
\*---------------------------------------------------------*/
#define MSI_USB_VID                                 0x1462

DetectedControllers DetectMSIMysticLight1603Controllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev != nullptr)
    {
        MSIMysticLight1603Controller*     controller     = new MSIMysticLight1603Controller(dev, info->path);
        RGBController_MSIMysticLight1603* rgb_controller = new RGBController_MSIMysticLight1603(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_PU("MSI Mystic Light MS_1603", DetectMSIMysticLight1603Controllers, MSI_USB_VID, 0x1603, 0x00FF, 0x01);

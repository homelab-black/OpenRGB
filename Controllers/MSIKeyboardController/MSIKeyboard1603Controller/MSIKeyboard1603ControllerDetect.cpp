/*---------------------------------------------------------*\
| MSIKeyboard1603ControllerDetect.cpp                       |
|                                                           |
|   Detector for MSI Mystic Light MS-1603 keyboards         |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "MSIKeyboard1603Controller.h"
#include "RGBController_MSIKeyboard1603.h"

/*---------------------------------------------------------*\
| USB IDs                                                   |
\*---------------------------------------------------------*/
#define MSI_USB_VID                                 0x1462

DetectedControllers DetectMSIKeyboard1603Controllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev != nullptr)
    {
        MSIKeyboard1603Controller*     controller     = new MSIKeyboard1603Controller(dev, info->path);
        RGBController_MSIKeyboard1603* rgb_controller = new RGBController_MSIKeyboard1603(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_PU("MSI Keyboard MS_1603", DetectMSIKeyboard1603Controllers, MSI_USB_VID, 0x1603, 0x00FF, 0x01);


/*---------------------------------------------------------*\
|   ThermaltakeMousepadDetect.cpp                           |
|                                                           |
|   Detector for Thermaltake Argent MP1 Mousepad            |
|                                                           |
|   John Scripter                               26 Oct 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "RGBController.h"
#include "RGBController_ThermaltakeArgentMP1.h"
#include "ThermaltakeArgentMP1Controller.h"

std::vector<RGBController*> DetectThermaltakeArgentMP1(hid_device_info* info, const std::string& args)
{
    (void)args;
    std::vector<RGBController*> controllers;

    // The device exposes 4 endpoints; ensure we target the correct interface matching Endpoint 4
    // Usually, specific endpoints map to a distinct interface number (e.g., interface 1 or 2)
    if(info->interface_number == 1 || info->interface_number == -1)
    {
        hid_device* dev = hid_open_path(info->path);

        if(dev)
        {
            ThermaltakeArgentMP1Controller*     controller     = new ThermaltakeArgentMP1Controller(dev, *info);
            RGBController_ThermaltakeArgentMP1* rgb_controller = new RGBController_ThermaltakeArgentMP1(controller);

            controllers.push_back(rgb_controller);
        }
    }

    return controllers;
}

REGISTER_HID_DETECTOR("Thermaltake Argent MP1", DetectThermaltakeArgentMP1, 0x264A, 0x9011);

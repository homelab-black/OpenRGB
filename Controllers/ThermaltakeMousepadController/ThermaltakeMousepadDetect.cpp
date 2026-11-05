
/*---------------------------------------------------------*\
|   ThermaltakeMousepadDetect.cpp                           |
|                                                           |
|   Detector for Thermaltake Argent MP1 Mousepad            |
|                                                           |
|   John Scripter                                4 Nov 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "RGBController.h"
#include "RGBController_ThermaltakeArgentMP1.h"
#include "ThermaltakeArgentMP1Controller.h"

/*---------------------------------------------------------*\
|  Vendor and Product Constants                             |
\*---------------------------------------------------------*/
#define THERMALTAKE_VID                 0x264A
#define THERMALTAKE_ARGENT_MP1_PID      0x9011
#define THERMALTAKE_MP1_USAGE_PAGE      0xFF00
#define THERMALTAKE_MP1_USAGE           0x0001


DetectedControllers DetectThermaltakeArgentMP1(hid_device_info* info, const std::string&)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        ThermaltakeArgentMP1Controller*     controller     = new ThermaltakeArgentMP1Controller(dev, *info);
        RGBController_ThermaltakeArgentMP1* rgb_controller = new RGBController_ThermaltakeArgentMP1(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Device Registration                                       |
\*---------------------------------------------------------*/

REGISTER_HID_DETECTOR_IPU("Thermaltake Argent MP1", DetectThermaltakeArgentMP1, THERMALTAKE_VID, THERMALTAKE_ARGENT_MP1_PID, 1, THERMALTAKE_MP1_USAGE_PAGE, THERMALTAKE_MP1_USAGE);


/*---------------------------------------------------------*\
| TOPCRGBControllerDetect.cpp                               |
|                                                           |
|   Detector for TOPC RGB devices                           |
|                                                           |
|   Diam (last.monoliths@gmail.com)              6 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController.h"
#include "RGBController_TOPCRGB.h"
#include "TOPCRGBController.h"

const char* TOPC_RGB_DETECT_NAME = "TOPCRGBControllerDetect";

DetectedControllers DetectTOPCRGBControllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    LOG_DEBUG("[%s] Detector called for path %s (VID %04X PID %04X, usage page %04X usage %04X, interface %d)",
              TOPC_RGB_DETECT_NAME,
              info->path,
              info->vendor_id,
              info->product_id,
              info->usage_page,
              info->usage,
              info->interface_number);

    dev = hid_open_path(info->path);

    if(dev)
    {
        LOG_DEBUG("[%s] hid_open_path succeeded, creating controller", TOPC_RGB_DETECT_NAME);

        TOPCRGBController* controller = new TOPCRGBController(dev, info->path);

        RGBController_TOPCRGB* rgb_controller = new RGBController_TOPCRGB(controller);

        detected_controllers.push_back(rgb_controller);
    }
    else
    {
        LOG_ERROR("[%s] hid_open_path failed for path %s", TOPC_RGB_DETECT_NAME, info->path);
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| The RGB control interface is the vendor-defined HID        |
| interface (usage page 0xFF01, usage 0x0001).  Filtering    |
| on the usage page/usage ensures only a single controller   |
| is created even if the device exposes other HID top level  |
| collections.                                              |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_PU("TOPC RGB Device", DetectTOPCRGBControllers, TOPC_RGB_VID, TOPC_RGB_PID, TOPC_RGB_USAGE_PAGE, TOPC_RGB_USAGE);


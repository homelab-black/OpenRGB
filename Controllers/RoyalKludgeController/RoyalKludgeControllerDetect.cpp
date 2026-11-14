/*---------------------------------------------------------*\
| RoyalKludgeControllerDetect.cpp                           |
|                                                           |
|   Detector for Royal Kludge RK987 keyboards               |
|                                                           |
|   johnm                                       01 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController_RoyalKludge.h"

DetectedControllers DetectRoyalKludgeKeyboard(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    dev = hid_open_path(info->path);

    if(dev)
    {
        /*-------------------------------------------------*\
        | Windows and macOS expose each HID collection as a  |
        | separate device path.  Only the vendor collection  |
        | that implements the 65-byte feature report can be  |
        | controlled, so probe it before creating the        |
        | controller.  A feature read is used so detection   |
        | does not change the keyboard lighting.             |
        \*-------------------------------------------------*/
        unsigned char   feature_buffer[ROYAL_KLUDGE_REPORT_SIZE] = { 0x0A };
        int             feature_length = hid_get_feature_report(dev, feature_buffer, sizeof(feature_buffer));

        if(feature_length >= 0)
        {
            RoyalKludgeController*      controller      = new RoyalKludgeController(dev, info->path, name);
            RGBController_RoyalKludge*  rgb_controller  = new RGBController_RoyalKludge(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            LOG_DEBUG("Skipping Royal Kludge collection at %s: no 65-byte feature report", info->path);
            hid_close(dev);
        }
    }
    else
    {
        LOG_ERROR("Couldn't open Royal Kludge device at %s: %ls", info->path, hid_error(NULL));
    }

    return(detected_controllers);
}

/*---------------------------------------------------------*\
| Linux reports the keyboard usage page for the interface,   |
| while Windows and macOS expose the vendor collection as a  |
| separate path.  Register both and let the feature probe     |
| select the collection that supports the protocol.          |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Royal Kludge RK987", DetectRoyalKludgeKeyboard, ROYAL_KLUDGE_VID, ROYAL_KLUDGE_RK987_2_4G_PID, 1, 0x0001, 0x0080);
REGISTER_HID_DETECTOR_IPU("Royal Kludge RK987", DetectRoyalKludgeKeyboard, ROYAL_KLUDGE_VID, ROYAL_KLUDGE_RK987_2_4G_PID, 1, 0xFF00, 0x0001);

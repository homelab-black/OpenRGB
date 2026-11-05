/*---------------------------------------------------------*\
| RoyalKludgeControllerDetect.cpp                           |
|                                                           |
|   Detector for Royal Kludge keyboards                     |
|                                                           |
|   manumyx                                     03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "DetectionManager.h"
#include "RoyalKludgeS98Controller.h"
#include "RGBController_RoyalKludgeS98.h"

/*---------------------------------------------------------*\
| Royal Kludge (BeiYing / SinoWealth) Vendor ID             |
\*---------------------------------------------------------*/
#define ROYAL_KLUDGE_VID                                    0x258A

/*---------------------------------------------------------*\
| Royal Kludge S98 Wired USB Product IDs                    |
\*---------------------------------------------------------*/
#define ROYAL_KLUDGE_S98_US_PID                             0x01AF
#define ROYAL_KLUDGE_S98_ES_PURPLE_PID                      0x01BF
#define ROYAL_KLUDGE_S98_POR_PID                            0x021E
#define ROYAL_KLUDGE_S98_TH_PID                             0x0223
#define ROYAL_KLUDGE_S98_DE_PID                             0x0224
#define ROYAL_KLUDGE_S98_ES_PID                             0x022B
#define ROYAL_KLUDGE_S98_JP_PID                             0x022F
#define ROYAL_KLUDGE_S98_RU_PID                             0x0230
#define ROYAL_KLUDGE_S98_TC_PID                             0x023F
#define ROYAL_KLUDGE_S98_KR_PID                             0x0240
#define ROYAL_KLUDGE_S98_FR_PID                             0x0241

/*---------------------------------------------------------*\
| Royal Kludge 2.4GHz Wireless Dongle Product ID            |
\*---------------------------------------------------------*/
#define ROYAL_KLUDGE_24G_DONGLE_PID                         0x0150

DetectedControllers DetectRoyalKludgeS98WiredControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        RoyalKludgeS98Controller*     controller     = new RoyalKludgeS98Controller(dev, info->path, name, false);
        RGBController_RoyalKludgeS98* rgb_controller = new RGBController_RoyalKludgeS98(controller);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

DetectedControllers DetectRoyalKludgeS98WirelessControllers(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;
    hid_device*         dev = hid_open_path(info->path);

    if(dev)
    {
        if(RoyalKludgeS98Controller::ProbeDongleS98(dev))
        {
            RoyalKludgeS98Controller*     controller     = new RoyalKludgeS98Controller(dev, info->path, name, true);
            RGBController_RoyalKludgeS98* rgb_controller = new RGBController_RoyalKludgeS98(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            hid_close(dev);
        }
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_US_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_ES_PURPLE_PID, 1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_POR_PID,       1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_TH_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_DE_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_ES_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_JP_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_RU_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_TC_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_KR_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (Wired)",       DetectRoyalKludgeS98WiredControllers,       ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_S98_FR_PID,        1,  0xFF02, 0x02);
REGISTER_HID_DETECTOR_IPU("Royal Kludge S98 (2.4GHz)",      DetectRoyalKludgeS98WirelessControllers,    ROYAL_KLUDGE_VID,   ROYAL_KLUDGE_24G_DONGLE_PID,    1,  0xFF02, 0x02);

/*---------------------------------------------------------*\
| MSIGE76ControllerDetect.cpp                               |
|                                                           |
|   Detector for MSI GE76 Raider SteelSeries KLC/ALC       |
|                                                           |
|   KLC (0x1122) = per-zone keyboard backlight             |
|   ALC (0x1134) = front ambient light bar                 |
|                                                           |
|   Both devices share the same HID protocol.              |
|   LED count is the only difference between them.         |
|                                                           |
|   To determine the exact LED count for your unit, run:   |
|     sudo /tmp/probe_hid                                   |
|   and read back the feature report. Each LED takes       |
|   3 bytes (R, G, B) starting at offset 3 of the report. |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "MSIGE76Controller.h"
#include "MSIGE76KeyboardLayout.h"
#include "RGBController_MSIGE76.h"

#define STEELSERIES_VID             0x1038

/*---------------------------------------------------------*\
| Filter to interface 0 only (vendor-defined, usage page   |
| 0xFFC0, usage 0x0001).  Interface 1 is consumer/media    |
| keys (usage page 0x000C) and must not be opened here —   |
| matching HID_INTERFACE_ANY caused duplicate devices.     |
\*---------------------------------------------------------*/
#define MSIGE76_HID_USAGE_PAGE      0xFFC0
#define MSIGE76_HID_USAGE           0x0001

/*---------------------------------------------------------*\
| KLC: SteelSeries embedded keyboard (GE76 Raider)         |
\*---------------------------------------------------------*/
#define MSIGE76_KLC_PID             0x1122

/*---------------------------------------------------------*\
| ALC: SteelSeries embedded front light bar (GE76 Raider)  |
| Aurora bar: 29 zones; packet slots 0–28, HID KC 1–29.       |
| See SetColorsALC.                                          |
\*---------------------------------------------------------*/
#define MSIGE76_ALC_PID             0x1134

/*---------------------------------------------------------*\
| DetectMSIGE76KLC                                         |
\*---------------------------------------------------------*/
void DetectMSIGE76KLC(hid_device_info* info, const std::string& /*name*/)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        MSIGE76Controller*     controller     = new MSIGE76Controller(dev, info->path, "MSI GE76 Raider Keyboard", MSIGE76_KLC_LED_COUNT);
        RGBController_MSIGE76* rgb_controller = new RGBController_MSIGE76(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

/*---------------------------------------------------------*\
| DetectMSIGE76ALC                                         |
\*---------------------------------------------------------*/
void DetectMSIGE76ALC(hid_device_info* info, const std::string& /*name*/)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        MSIGE76Controller*     controller     = new MSIGE76Controller(dev, info->path, "MSI GE76 Raider Light Bar", MSIGE76_ALC_LED_COUNT);
        RGBController_MSIGE76* rgb_controller = new RGBController_MSIGE76(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR_PU("MSI GE76 Raider Keyboard", DetectMSIGE76KLC, STEELSERIES_VID, MSIGE76_KLC_PID, MSIGE76_HID_USAGE_PAGE, MSIGE76_HID_USAGE);
REGISTER_HID_DETECTOR_PU("MSI GE76 Raider Light Bar", DetectMSIGE76ALC, STEELSERIES_VID, MSIGE76_ALC_PID, MSIGE76_HID_USAGE_PAGE, MSIGE76_HID_USAGE);

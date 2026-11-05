/*---------------------------------------------------------*\
| AkkoKeyboardControllerDetect.cpp                           |
|                                                           |
|   Detector for Akko / RongYuan open-protocol keyboards.   |
|                                                           |
|   Roz                                         20 Jun 2026 |
|                                                           |
|   Protocol by the echtzeit-solutions/monsgeek-akko-linux  |
|   project (docs/PROTOCOL.md) and the SonixQMK community.  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "AkkoKeyboardController.h"
#include "RGBController_AkkoKeyboard.h"

/*-----------------------------------------------------------*\
| The RGB control lives on the vendor config interface:       |
|   interface 2, usage page 0xFFFF, usage 0x0002              |
\*-----------------------------------------------------------*/
void DetectAkkoKeyboardControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        AkkoKeyboardController*       controller     = new AkkoKeyboardController(dev, info->path, info->product_id, info->interface_number, name);
        RGBController_AkkoKeyboard*   rgb_controller = new RGBController_AkkoKeyboard(controller);
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR_IPU("Akko 5075",  DetectAkkoKeyboardControllers,  AKKO_KB_VID,    AKKO_5075_PID,  2,  0xFFFF, 0x0002);

/*---------------------------------------------------------*\
| AkkoControllerDetect.cpp                                  |
|                                                           |
|   Driver for Akko keyboard                                |
|                                                           |
|   Bizzotto Nicola (nicola02nb)                05 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "AkkoController.h"
#include "RGBController_AkkoController.h"

/*-----------------------------------------------------*\
| Akko vendor ID                                        |
\*-----------------------------------------------------*/
#define AKKO_VID                    0x3151

/*-----------------------------------------------------*\
| Akko product ID                                       |
\*-----------------------------------------------------*/
#define AKKO_5075B_USB_PID          0x4015
#define AKKO_5075B_Dongle_PID       0x4011

/******************************************************************************************\
*                                                                                          *
*   DetectAkkoControllers                                                                  *
*                                                                                          *
*       Tests the USB address to see if any Akko Controllers exists there.                 *
*                                                                                          *
\******************************************************************************************/

void DetectAkkoControllers(hid_device_info* info, const std::string&)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        bool isWireless = (info->product_id == AKKO_5075B_Dongle_PID);
        AkkoController*               controller     = new AkkoController(dev, info->path, isWireless, true);
        RGBController_AkkoController* rgb_controller = new RGBController_AkkoController(controller);
        // Constructor sets the name
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}   /* DetectAkkoControllers() */

REGISTER_HID_DETECTOR_I("Akko 5075B Pro (USB Cable)", DetectAkkoControllers, AKKO_VID, AKKO_5075B_USB_PID, 2);
REGISTER_HID_DETECTOR_I("Akko 5075B Pro (USB Dongle)", DetectAkkoControllers, AKKO_VID, AKKO_5075B_Dongle_PID, 2);

/*---------------------------------------------------------*\
| Bluetooth Not implemented                                 |
\*---------------------------------------------------------*/
//REGISTER_HID_DETECTOR("Akko 5075B Pro (Bluetooth)", DetectAkkoControllers, AKKO_VID, AKKO_5075B_BT_PID);

/*---------------------------------------------------------*\
| RoyuanControllerDetect.cpp                                |
|                                                           |
|   Detector for Royuan keyboard (EpoMaker, Yunzii, etc..)  |
|                                                           |
|   Alvaro Munoz (alvaromunoz)                  05 Jun 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"
#include "RoyuanController.h"
#include "RGBController_RoyuanController.h"
#include "YunziiController.h"
#include "RGBController_YunziiController.h"

#define ROYUAN_VID                          0x3151
#define ROYUAN_TH80_PRO_USB_PID             0x4010
#define ATTACKSHARK_K86_USB_PID             0x4015
#define ROYUAN_TH80_PRO_WIRELESS_PID        0x4011 /* Attack shark's Dongle is the same. */
#define ROYUAN_TH80_PRO_BT_PID              0x4013
#define ATTACKSHARK_K86_BT_PID              0x4012
#define YUNZI_YZ75_USB_PID                  0x5025
#define YUNZI_YZ75_WIRELESS_PID             0x5026

/******************************************************************************************\ 
*                                                                                          *
*   DetectRoyuanControllers                                                                *
*                                                                                          *
*       Tests the USB address to see if any Royuan Controllers exists there.               *
*                                                                                          *
\******************************************************************************************/

void DetectRoyuanControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        RoyuanController*                   controller     = new RoyuanController(dev, info, name);
        RGBController_RoyuanController*     rgb_controller = new RGBController_RoyuanController(controller);

        /*---------------------------------------------------------*\
        | Get device name from HID manufacturer and product strings |
        \*---------------------------------------------------------*/
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}   /* DetectRoyuanControllers() */


/******************************************************************************************\ 
*                                                                                          *
*   DetectYunziiControllers                                                                *
*                                                                                          *
*       Tests the USB address to see if any Yunzii Controllers exists there.               *
*                                                                                          *
\******************************************************************************************/

void DetectYunziiControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        YunziiController*                   controller     = new YunziiController(dev, info, name);
        RGBController_YunziiController*     rgb_controller = new RGBController_YunziiController(controller);

        /*---------------------------------------------------------*\
        | Get device name from HID manufacturer and product strings |
        \*---------------------------------------------------------*/
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}   /* DetectYunziiControllers() */



REGISTER_HID_DETECTOR_I("Royuan TH80 Pro", DetectRoyuanControllers, ROYUAN_VID, ROYUAN_TH80_PRO_USB_PID, 2);
REGISTER_HID_DETECTOR_I("Royuan TH80 Pro (Wireless)", DetectRoyuanControllers, ROYUAN_VID, ROYUAN_TH80_PRO_WIRELESS_PID, 2);
REGISTER_HID_DETECTOR_I("Attack Shark K86", DetectRoyuanControllers, ROYUAN_VID, ATTACKSHARK_K86_USB_PID, 2);
REGISTER_HID_DETECTOR_I("Yunzii YZ75", DetectYunziiControllers, ROYUAN_VID, YUNZI_YZ75_USB_PID, 2);
REGISTER_HID_DETECTOR_I("Yunzii YZ75 (Wireless)", DetectYunziiControllers, ROYUAN_VID, YUNZI_YZ75_WIRELESS_PID, 2);

/*---------------------------------------------------------*\
| Bluetooth Not implemented                                 |
\*---------------------------------------------------------*/
//REGISTER_HID_DETECTOR("Royuan TH80 Pro (Bluetooth)", DetectRoyuanControllers, ROYUAN_VID, ROYUAN_TH80_PRO_BT_PID);
//REGISTER_HID_DETECTOR("Attack Shark K86 (Bluetooth)", DetectRoyuanControllers, ROYUAN_VID, ATTACKSHARK_K86_BT_PID);

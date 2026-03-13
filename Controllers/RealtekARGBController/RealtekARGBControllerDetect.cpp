/*---------------------------------------------------------*\
| RealtekARGBControllerDetect.cpp                           |
|                                                           |
|   Detector for Realtek USB ARGB ICs                       |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#ifndef __FreeBSD__

#include "Detector.h"
#include "RGBController.h"
#include "RGBController_RealtekARGB.h"

#define REALTEK_ARGB_HID2SCSI_PG                0xFF00
#define REALTEK_ARGB_HID2SCSI_USAGE             0x0001

/******************************************************************************************\
*                                                                                          *
*   DetectRealtekARGBControllers                                                           *
*                                                                                          *
*       Tests the USB address to see if an Realtek ARGB controller exists there            *
*                                                                                          *
\******************************************************************************************/
void DetectRealtekARGBControllers(hid_device_info* info, const std::string& name)
{
    RGBController_RealtekARGB* rgb_controller = NULL;
    RealtekARGBController* controller = NULL;
    static struct argb_device* dev = NULL;
    argbCtl* argb_ctl = NULL;

    dev = argb_path_open(info->path, REALTEK_ARGB_BRINTF_TYPE_HID, NULL);
    if(!dev)
    {
        goto exit;
    }

    if(!led_argb_get_ic_support_sync_light(dev, REALTEK_ARGB_SYNC_METHOD_OPENRGB))
    {
        goto exit;
    }

    if(bridge_write_unlock(dev))
    {
        goto exit;
    }

    argb_ctl = (argbCtl*)malloc(sizeof(argbCtl));
    if(!argb_ctl)
    {
        goto exit;
    }

    memset(argb_ctl, 0, sizeof(argbCtl));
    if(led_argb_init(dev, argb_ctl))
    {
        free(argb_ctl);
        goto exit;
    }
    argb_ctl->sync_method = REALTEK_ARGB_SYNC_METHOD_OPENRGB;

    controller = new RealtekARGBController(dev, argb_ctl, info);
    rgb_controller = new RGBController_RealtekARGB(controller);
    if(rgb_controller->type != DEVICE_TYPE_UNKNOWN)
    {
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
    else
    {
        delete controller;
        delete rgb_controller;
    }
exit:
    return;
}

REGISTER_HID_DETECTOR_PU   ("RTL9209", DetectRealtekARGBControllers, REALTEK_ARGB_VID, REALTEK_ARGB_PID, REALTEK_ARGB_HID2SCSI_PG, REALTEK_ARGB_HID2SCSI_USAGE);

#endif

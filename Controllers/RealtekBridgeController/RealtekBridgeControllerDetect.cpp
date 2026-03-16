/*---------------------------------------------------------*\
| RealtekBridgeControllerDetect.cpp                         |
|                                                           |
|   Controller for Realtek USB to SSD Bridge ICs            |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#ifndef __FreeBSD__

#include "Detector.h"
#include "RGBController_RealtekBridge.h"

#define REALTEK_BRIDGE_VID                 0x0BDA
#define REALTEK_BRIDGE_PID0                0x9220
#define REALTEK_BRIDGE_PID1                0x9201
#define REALTEK_BRIDGE_PID2                0x9210

#define REALTEK_HID2SCSI_PG                0xFF00
#define REALTEK_HID2SCSI_USAGE             0x0001

/******************************************************************************************\
*                                                                                          *
*   DetectRealtekBridgeControllers                                                         *
*                                                                                          *
*       Tests the USB address to see if an Realtek Bridge controller exists there          *
*                                                                                          *
\******************************************************************************************/
void DetectRealtekBridgeControllers(hid_device_info* info, const std::string& name)
{
    RealtekBridgeController* controller = NULL;
    RGBController_RealtekBridge* rgb_controller = NULL;
    dHandle fd = NULL_DHANDLE;
    struct led_mem* led_mem_dev = NULL;
    char path[256] = {0};

    sprintf(path, "%s:%d", info->path, 0);
    fd = bridge_open_handle(path);
    if(INVALID_DHANDLE(fd)) {
        goto exit;
    }

    if(!led_get_ic_support_sync_light(fd, REALTEK_BRIDGE_SYNC_METHOD_OPENRGB)) {
        goto exit;
    }

    if(!bridge_check_target(fd)) {
        goto exit;
    }

    if(bridge_write_unlock(fd)) {
        goto exit;
    }

    led_mem_dev = (struct led_mem*)malloc(sizeof(struct led_mem));
    if(!led_mem_dev) {
        goto exit;
    }

    memset(led_mem_dev, 0, sizeof(struct led_mem));
    if(led_mem_init(fd, led_mem_dev)) {
        free(led_mem_dev);
        goto exit;
    }
    led_mem_dev->sync_method = REALTEK_BRIDGE_SYNC_METHOD_OPENRGB;

    controller = new RealtekBridgeController(fd, led_mem_dev, info);
    rgb_controller = new RGBController_RealtekBridge(controller);
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

REGISTER_HID_DETECTOR_PU   ("RTL9220", DetectRealtekBridgeControllers, REALTEK_BRIDGE_VID, REALTEK_BRIDGE_PID0, REALTEK_HID2SCSI_PG, REALTEK_HID2SCSI_USAGE);
REGISTER_HID_DETECTOR_PU   ("RTL9201", DetectRealtekBridgeControllers, REALTEK_BRIDGE_VID, REALTEK_BRIDGE_PID1, REALTEK_HID2SCSI_PG, REALTEK_HID2SCSI_USAGE);
REGISTER_HID_DETECTOR_PU   ("RTL9210", DetectRealtekBridgeControllers, REALTEK_BRIDGE_VID, REALTEK_BRIDGE_PID2, REALTEK_HID2SCSI_PG, REALTEK_HID2SCSI_USAGE);
#endif

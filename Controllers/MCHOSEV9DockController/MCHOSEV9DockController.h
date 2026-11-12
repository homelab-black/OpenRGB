/*---------------------------------------------------------*\
| MCHOSEV9DockController.h                                  |
|                                                           |
|   Driver for MCHOSE V9 charging dock                      |
|                                                           |
|   Michael Mano <michael.mano26@gmail.com>     24 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define MCHOSE_V9_DOCK_PACKET_SIZE        65
#define MCHOSE_V9_DOCK_HEADER_SIZE        7
#define MCHOSE_V9_DOCK_MAX_PARAM_LEN      57
#define MCHOSE_V9_DOCK_LED_COUNT          20
#define MCHOSE_V9_DOCK_MODE_COUNT         5
#define MCHOSE_V9_DOCK_START_BYTE         0xAA

/*---------------------------------------------------------*\
| Commands                                                  |
\*---------------------------------------------------------*/
enum
{
    MCHOSE_V9_DOCK_CMD_GET_INFO           = 0x07,
    MCHOSE_V9_DOCK_CMD_SET_EFFECT         = 0x27,
    MCHOSE_V9_DOCK_CMD_SET_LEDS           = 0x29,
};

/*---------------------------------------------------------*\
| Modes                                                     |
\*---------------------------------------------------------*/
enum
{
    MCHOSE_V9_DOCK_MODE_STATIC            = 0x00,
    MCHOSE_V9_DOCK_MODE_BREATHING         = 0x01,
    MCHOSE_V9_DOCK_MODE_COLOR_CYCLE       = 0x03,
    MCHOSE_V9_DOCK_MODE_DIRECT            = 0x05,
};

#define MCHOSE_V9_DOCK_SPEED_MIN          0x00
#define MCHOSE_V9_DOCK_SPEED_MAX          0x04
#define MCHOSE_V9_DOCK_SPEED_DEFAULT      0x02
#define MCHOSE_V9_DOCK_BRIGHTNESS_MIN     0x01
#define MCHOSE_V9_DOCK_BRIGHTNESS_MAX     0x05

class MCHOSEV9DockController
{
public:
    MCHOSEV9DockController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~MCHOSEV9DockController();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();
    std::string GetFirmwareVersion();

    void SetEffect(bool enable, unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color);
    void SetLEDs(std::vector<RGBColor> colors);

private:
    hid_device*         dev;
    std::string         location;
    std::string         name;
    std::string         firmware_version;

    void ReadInfo();
    void SendCommand(unsigned char cmd, unsigned char frame_seq, unsigned char total_frames, unsigned char* params, unsigned char param_len);
};

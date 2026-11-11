/*---------------------------------------------------------*\
| CherryMouseController.h                                   |
|                                                           |
|   Driver for Cherry mouse                                 |
|                                                           |
|   Rik Vanden Boer                             03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

#define CHERRY_MOUSE_PACKET_SIZE                    64
#define CHERRY_MOUSE_READ_TIMEOUT_MS                50

/*-----------------------------------------------------*\
| Cherry mouse product IDs                              |
\*-----------------------------------------------------*/
#define MC_3_1_PID                                  0x01A5

/*-----------------------------------------------------*\
| Commands (shared with Cherry keyboards)               |
\*-----------------------------------------------------*/
enum
{
    CHERRY_MOUSE_COMMAND_BEGIN                      = 0x01,
    CHERRY_MOUSE_COMMAND_END                        = 0x02,
    CHERRY_MOUSE_COMMAND_SET_PARAMETER              = 0x06,
};

enum
{
    CHERRY_MOUSE_BRIGHTNESS_LOWEST                  = 0x00,
    CHERRY_MOUSE_BRIGHTNESS_HIGHEST                 = 0x04,
};

enum
{
    CHERRY_MOUSE_SPEED_SLOWEST                      = 0x04,
    CHERRY_MOUSE_SPEED_NORMAL                       = 0x02,
    CHERRY_MOUSE_SPEED_FASTEST                      = 0x00,
};

class CherryMouseController
{
public:
    CherryMouseController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~CherryMouseController();

    std::string GetDeviceLocation();
    std::string GetDeviceName();
    std::string GetSerialString();

    void        SetMode
                    (
                    unsigned char       mode,
                    unsigned char       brightness,
                    unsigned char       speed,
                    unsigned char       direction,
                    unsigned char       random_flag,
                    unsigned char       red,
                    unsigned char       green,
                    unsigned char       blue
                    );

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;

    void        SendPacket(unsigned char* usb_buf);
    void        SendCommand(unsigned char command);
};

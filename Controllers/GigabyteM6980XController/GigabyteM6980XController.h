/*---------------------------------------------------------*\
| GigabyteM6980XController.h                                |
|                                                           |
|   Driver for the Gigabyte M6980X mouse                    |
|                                                           |
|   Tom Rehm (Tom-8192)                         09 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| The mouse uses an 8 byte feature report without report ID |
| on its mouse interface.  A command is [group, command,    |
| arg0..arg5]; the reply is [status, data0..data6].         |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_REPORT_SIZE         8

/*---------------------------------------------------------*\
| Handshake: wait until the busy bit is clear, send the     |
| command, then poll until the done bit is set              |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_STATUS_BUSY         0x80
#define GIGABYTE_M6980X_STATUS_DONE         0x40
#define GIGABYTE_M6980X_POLL_MS             4
#define GIGABYTE_M6980X_BUSY_TRIES          20
#define GIGABYTE_M6980X_DONE_TRIES          200
#define GIGABYTE_M6980X_WRITE_DELAY_MS      10

/*---------------------------------------------------------*\
| Commands                                                  |
|   00 00           identify                                |
|   01 06 p         read LED color of profile p             |
|   01 86 p R G B   write LED color of profile p            |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_GROUP_SYSTEM        0x00
#define GIGABYTE_M6980X_GROUP_REGISTER      0x01
#define GIGABYTE_M6980X_CMD_IDENTIFY        0x00
#define GIGABYTE_M6980X_REG_LED             0x06
#define GIGABYTE_M6980X_WRITE_BIT           0x80

/*---------------------------------------------------------*\
| One LED color per hardware profile.  Each channel has     |
| three levels: 0 off, 1 half, 2 full.                      |
\*---------------------------------------------------------*/
#define GIGABYTE_M6980X_PROFILES            3
#define GIGABYTE_M6980X_LEVELS              3

class GigabyteM6980XController
{
public:
    GigabyteM6980XController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name);
    ~GigabyteM6980XController();

    std::string         GetDeviceLocation();
    std::string         GetNameString();
    std::string         GetSerialString();

    bool                IsM6980X();
    bool                GetColor(unsigned char profile, RGBColor* color);
    void                SetColor(unsigned char profile, RGBColor color);

private:
    hid_device*         dev;
    std::string         location;
    std::string         name;
    std::mutex          mutex_lock;

    bool                GetLevels(unsigned char profile, unsigned char* levels);
    bool                Transfer(unsigned char group, unsigned char command, unsigned char* args, int arg_count, int delay_ms, unsigned char* reply);
    bool                GetReport(unsigned char* data);
};

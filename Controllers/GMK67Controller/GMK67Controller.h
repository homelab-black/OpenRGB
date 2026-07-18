/*---------------------------------------------------------*\
| GMK67Controller.h                                         |
|                                                           |
|   Driver for the GMK67 (VID 05AC PID 024F) keyboard.      |
|   Protocol ported from the SignalRGB "GMK68" plugin       |
|   by Nollie (Nuonuo). It uses HID feature reports and a   |
|   direct per-key colour streaming mode (command 0x20).    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*-----------------------------------------------------------------*\
| Report layout                                                     |
|                                                                   |
|   Every transfer is an HID feature report of 65 bytes:            |
|   [0]      = report ID (0x00)                                     |
|   [1..64]  = 64 payload bytes                                     |
\*-----------------------------------------------------------------*/
#define GMK67_REPORT_ID             0x00
#define GMK67_PACKET_DATA_LENGTH    64                                  /* payload bytes per report            */
#define GMK67_LED_COUNT             66                                  /* addressable per-key LEDs            */
#define GMK67_COLOR_STREAM_SIZE     (GMK67_LED_COUNT * 4)               /* 66 x (index, R, G, B) = 264 bytes   */
#define GMK67_COLOR_PACKETS         7                                   /* 264 bytes split over 7 x 64-byte    */

/*-----------------------------------------------------------------*\
| Protocol commands (payload byte 0 = 0x04 header, byte 1 = cmd)    |
\*-----------------------------------------------------------------*/
#define GMK67_HEADER                0x04
#define GMK67_CMD_DIRECT_START      0x20                                /* begin direct colour write           */
#define GMK67_CMD_END               0x02                                /* apply / end communication           */

class GMK67Controller
{
public:
    GMK67Controller(hid_device* dev_handle, const char* path, std::string dev_name);
    ~GMK67Controller();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();

    /*-------------------------------------------------------------*\
    | Stream a full frame of per-key colours to the keyboard.       |
    | color_stream must be GMK67_COLOR_STREAM_SIZE bytes laid out   |
    | as 66 x (hardware_index, R, G, B).                            |
    \*-------------------------------------------------------------*/
    void            SendColors(const unsigned char* color_stream, unsigned int size);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;

    void            SendFeature(const unsigned char* data);            /* data = GMK67_PACKET_DATA_LENGTH bytes */
    void            ReadFeature();
    void            StartRefresh();
    void            EndRefresh();
};

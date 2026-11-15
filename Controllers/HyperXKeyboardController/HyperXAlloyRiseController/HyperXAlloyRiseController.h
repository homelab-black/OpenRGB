/*---------------------------------------------------------*\
| HyperXAlloyRiseController.h                               |
|                                                           |
|   Driver for HyperX Alloy Rise keyboard (full size)       |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| Number of LEDs exposed to OpenRGB                         |
\*---------------------------------------------------------*/
#define HYPERX_ALLOY_RISE_KEY_LED_COUNT         110
#define HYPERX_ALLOY_RISE_BAR_LED_COUNT         7

class HyperXAlloyRiseController
{
public:
    HyperXAlloyRiseController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~HyperXAlloyRiseController();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();

    /*-----------------------------------------------------*\
    | Colors are expected in this order:                    |
    |   110 key LEDs, 7 left bar LEDs, 7 right bar LEDs     |
    \*-----------------------------------------------------*/
    void            SetLEDsDirect(const std::vector<RGBColor>& colors);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    std::mutex      write_mutex;

    bool            initialized;
    bool            report_id_mode;
    unsigned int    frames_sent;

    bool            WritePacket(const unsigned char* data);
    void            FlushInput();
    unsigned int    WaitForReplies(unsigned int expected);
    void            SendCommand(unsigned char cmd, unsigned char sub, unsigned char arg2, unsigned char arg3, unsigned char arg4, unsigned int expected_replies);
    void            SendFrame(const unsigned char* slot_colors);
};

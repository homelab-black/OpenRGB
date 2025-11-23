/*---------------------------------------------------------*\
| FnaticStreakController.h                                  |
|                                                           |
|   Driver for Fnatic Streak keyboard                       |
|                                                           |
|   Based on leddy by Hanna Czenczek                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*-----------------------------------------------------*\
| Fnatic vendor ID                                      |
\*-----------------------------------------------------*/
#define FNATIC_VID                      0x2F0E

/*-----------------------------------------------------*\
| Keyboard product IDs                                  |
\*-----------------------------------------------------*/
#define FNATIC_STREAK_PID               0x0101
#define FNATIC_MINISTREAK_PID           0x0102

class FnaticStreakController
{
public:
    FnaticStreakController(hid_device* dev_handle, const char* path, const unsigned short pid, std::string dev_name);
    ~FnaticStreakController();

    std::string     GetLocationString();
    std::string     GetNameString();
    std::string     GetSerialString();
    unsigned short  GetUSBPID();
    bool            IsMini();
    unsigned int    GetLEDCount();
    unsigned int    GetWidth();

    const std::vector<unsigned char>& GetLEDMap();

    void SetProfile(unsigned char profile);
    void SendColors(unsigned char* color_data, unsigned int color_data_size);
    void Initialize();

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;
    unsigned short          usb_pid;
    bool                    mini;
    unsigned char           profile;
    std::vector<unsigned char> color_cmd_prefix;
    unsigned int            led_count;
    unsigned int            width;
    std::vector<unsigned char> ledmap;

    void SendRequest(const std::vector<unsigned char>& prefix, const unsigned char* raw_data, unsigned int data_len);
    void RefreshProfile();
};

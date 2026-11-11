/*---------------------------------------------------------*\
| MaonoController.h                                         |
|                                                           |
|   Driver for Maono microphone lighting                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <map>
#include <mutex>
#include <string>
#include <hidapi.h>
#include "MaonoDevices.h"
#include "MaonoProtocol.h"

#define MAONO_MAX_CHANNELS  2

class MaonoController
{
public:
    MaonoController(hid_device* dev_handle, const char* path, std::string dev_name, const MaonoDeviceInfo* device_info);
    ~MaonoController();

    const MaonoDeviceInfo*  GetInfo();
    std::string             GetDeviceLocation();
    std::string             GetNameString();
    std::string             GetSerialString();

    void    SetPower(unsigned int channel, bool on);
    void    SetBrightness(unsigned int channel, unsigned int brightness);
    void    SetEffect(unsigned int channel, unsigned short effect);
    void    SetColorValue(unsigned int channel, unsigned short value);
    void    SetCustomColor(unsigned int channel, unsigned char red, unsigned char grn, unsigned char blu);

private:
    hid_device*                                 dev;
    std::string                                 location;
    std::string                                 name;
    const MaonoDeviceInfo*                      info;
    std::mutex                                  lock;

    /*-----------------------------------------------------*\
    | Last value written per register, so effects engines   |
    | that update every frame only send what changed        |
    \*-----------------------------------------------------*/
    std::map<unsigned short, unsigned short>    cache;
    bool                                        custom_slot_ready[MAONO_MAX_CHANNELS];

    void    WriteRegister(unsigned int channel, unsigned short reg, unsigned short value);
};

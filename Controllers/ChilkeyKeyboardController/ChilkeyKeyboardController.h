/*---------------------------------------------------------*\
| ChilkeyKeyboardController.h                               |
|                                                           |
|   Driver for Chilkey Slice75 HE keyboard                  |
|                                                           |
|   Aaron Coyco                                 28 Aug 2026 |
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
| Report format                                          |
\*-----------------------------------------------------*/
#define CHILKEY_REPORT_SIZE                0x40
#define CHILKEY_REPORT_ID                  0x00
#define CHILKEY_PROTOCOL_HEADER            0x5C
#define CHILKEY_CHECKSUM_BASE              0x35

/*-----------------------------------------------------*\
| Commands                                               |
\*-----------------------------------------------------*/
#define CHILKEY_COMMAND_SYNC                0x01
#define CHILKEY_COMMAND_LIGHTING            0x18
#define CHILKEY_COMMAND_PER_KEY_RGB         0x2A

#define CHILKEY_SUBCOMMAND_WRITE            0x01

/*-----------------------------------------------------*\
| Lighting parameters                                    |
\*-----------------------------------------------------*/
#define CHILKEY_MAX_LEDS_PER_PACKET         14
#define CHILKEY_PALETTE_COLOR_COUNT         7
#define CHILKEY_BRIGHTNESS_MAX              0x04
#define CHILKEY_PER_KEY_LIGHTING_MODE       0x15

class ChilkeyKeyboardController
{
public:
    ChilkeyKeyboardController(hid_device* dev_handle, hid_device_info* info, std::string dev_name);
    ~ChilkeyKeyboardController();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();

    void            Initialize();
    void            SetColors(const std::vector<RGBColor>& colors, const std::vector<unsigned char>& key_ids);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;

    unsigned char   CalculateChecksum(unsigned char* packet);
    bool            SendPacket(unsigned char* packet);
    bool            SendSync();
    bool            SendPerKeyLightingMode();
};

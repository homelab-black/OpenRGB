/*---------------------------------------------------------*\
| TuyaController.h                                          |
|                                                             |
|   Driver for Tuya-based WiFi RGB lights (local LAN         |
|   protocol, used by Tuya OEM brands such as LEDVANCE/      |
|   SMART+, Feit, Merkury, and generic "Smart Life" strips)  |
|                                                             |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "net_port.h"

class TuyaController
{
public:
    TuyaController(std::string ip, std::string device_id, std::string local_key);
    ~TuyaController();

    std::string GetLocation();
    std::string GetDeviceID();

    void SetColorHSV(unsigned int hue, unsigned int saturation, unsigned int value);
    void SetPower(bool on);

private:
    bool SendPacket(unsigned int command, const std::string& json_payload, bool prepend_version_header);

    std::string ip_address;
    std::string device_id;
    std::string local_key;
    std::string location;
    unsigned int sequence_number;
    net_port     port;
};

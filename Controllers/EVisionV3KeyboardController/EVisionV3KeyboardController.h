/*---------------------------------------------------------*\
| EVisionV3KeyboardController.h                             |
|                                                           |
|   Driver for EVision "RDR" tri-mode keyboards (VIA)       |
|                                                           |
|   Jan Imrich (imrija)                         05 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| EVision (Shenzhen Evision Semiconductor) vendor ID        |
\*---------------------------------------------------------*/
#define EVISION_V3_VID                              0x320F

/*---------------------------------------------------------*\
| Product IDs                                               |
|   0x5055 is shared across the whole "RDR" tri-mode        |
|   firmware family (Eternico MKS7150, Yenkee YKB 30,       |
|   Womier SK75, Zuoya GMK87, ...), so the detector         |
|   verifies the VIA handshake before registering.          |
\*---------------------------------------------------------*/
#define EVISION_V3_RDR_PID                          0x5055

#define EVISION_V3_PACKET_SIZE                      32

/*---------------------------------------------------------*\
| Value limits                                              |
\*---------------------------------------------------------*/
#define EVISION_V3_BRIGHTNESS_MIN                   0
#define EVISION_V3_BRIGHTNESS_MAX                   9
#define EVISION_V3_SPEED_MIN                        0
#define EVISION_V3_SPEED_MAX                        4

class EVisionV3KeyboardController
{
public:
    EVisionV3KeyboardController(hid_device* dev_handle, const char* path);
    ~EVisionV3KeyboardController();

    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerial();
    std::string     GetVersion();
    bool            GetSupported();

    unsigned char   GetBrightness();
    unsigned char   GetEffect();
    unsigned char   GetSpeed();
    void            GetColor(unsigned char* hue, unsigned char* sat);

    void            SetMode(unsigned char mode, unsigned char speed, unsigned char hue, unsigned char sat, unsigned char brightness, bool set_color);
    void            SaveMode();

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    std::string     serial;
    unsigned short  via_protocol_version;

    unsigned char   CmdGetRGBMatrixValue(unsigned char property, unsigned char* extra = NULL);
    void            CmdSetRGBMatrixValue(unsigned char property, unsigned char* values, unsigned char values_size);
    void            CmdGetViaProtocolVersion(unsigned short* version);

    int             ViaSendCommand(unsigned char cmd, unsigned char* data_in, unsigned char data_in_size, unsigned char* data_out, unsigned char data_out_size);
};

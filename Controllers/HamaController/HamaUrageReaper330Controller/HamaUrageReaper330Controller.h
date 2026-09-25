/*---------------------------------------------------------*\
| HamaUrageReaper330Controller.h                            |
|                                                           |
|   Driver for Hama uRage Reaper 330 mouse                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>

/*---------------------------------------------------------*\
| Lighting mode values, as sent in byte 2 of the mode       |
| command.  Decoded from two USB captures of the vendor     |
| software with the modes clicked in a known order.         |
| Values 0x06 and 0x07 have not been observed.              |
\*---------------------------------------------------------*/
enum
{
    HAMA_URAGE_REAPER_330_MODE_OFF              = 0x00,
    HAMA_URAGE_REAPER_330_MODE_STATIC           = 0x01,
    HAMA_URAGE_REAPER_330_MODE_BREATHING        = 0x02,
    HAMA_URAGE_REAPER_330_MODE_GLIDING          = 0x03,
    HAMA_URAGE_REAPER_330_MODE_RAINDROP         = 0x04,
    HAMA_URAGE_REAPER_330_MODE_KEY_REACTION     = 0x05,
    HAMA_URAGE_REAPER_330_MODE_ROLLING          = 0x08,
};

/*---------------------------------------------------------*\
| Effect speed ("Pulsation" in the vendor software), sent   |
| in byte 3 of the mode command.  4 steps.  0 is the FAST   |
| end and 3 is the SLOW end (confirmed on hardware).        |
\*---------------------------------------------------------*/
#define HAMA_URAGE_REAPER_330_SPEED_SLOWEST         3
#define HAMA_URAGE_REAPER_330_SPEED_FASTEST         0

/*---------------------------------------------------------*\
| Brightness, sent in byte 4 of the mode command.  5 steps. |
| The vendor software only offers it in Static mode; every  |
| other mode is sent with the maximum.                      |
\*---------------------------------------------------------*/
#define HAMA_URAGE_REAPER_330_BRIGHTNESS_MIN        0
#define HAMA_URAGE_REAPER_330_BRIGHTNESS_MAX        4

class HamaUrageReaper330Controller
{
public:
    HamaUrageReaper330Controller(hid_device* dev_handle, const char* path, std::string dev_name);
    ~HamaUrageReaper330Controller();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();

    void        SetMode(unsigned char mode, unsigned char speed, unsigned char brightness,
                        unsigned char red, unsigned char green, unsigned char blue);

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;

    void        SendCommand(unsigned char cmd, unsigned char index, unsigned char value);
    void        SendDataPacket(unsigned char red, unsigned char green, unsigned char blue);
};

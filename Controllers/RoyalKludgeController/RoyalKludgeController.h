/*---------------------------------------------------------*\
| RoyalKludgeController.h                                   |
|                                                           |
|   Driver for Royal Kludge RK987 keyboards                 |
|                                                           |
|   johnm                                       01 Nov 2026 |
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

#define ROYAL_KLUDGE_VID                    0x258A
#define ROYAL_KLUDGE_RK987_2_4G_PID         0x01FF

#define ROYAL_KLUDGE_REPORT_SIZE            65
#define ROYAL_KLUDGE_CUSTOM_BUFFERS         7
#define ROYAL_KLUDGE_CUSTOM_DATA_SIZE       431
#define ROYAL_KLUDGE_DIRECT_MODE_VALUE      0xFFFF
#define ROYAL_KLUDGE_RAINBOW_MODE_VALUE     0xFFFE

/*---------------------------------------------------------*\
| Keyboard light modes                                      |
\*---------------------------------------------------------*/
#define ROYAL_KLUDGE_MODE_NEON_STREAM           0x01
#define ROYAL_KLUDGE_MODE_RIPPLES_SHINING       0x02
#define ROYAL_KLUDGE_MODE_ROTATING_WINDMILL     0x03
#define ROYAL_KLUDGE_MODE_SINE_WAVE             0x04
#define ROYAL_KLUDGE_MODE_RAINBOW_ROULETTE      0x05
#define ROYAL_KLUDGE_MODE_STARS_TWINKLE         0x06
#define ROYAL_KLUDGE_MODE_LAYER_UPON_LAYER      0x07
#define ROYAL_KLUDGE_MODE_RICH_AND_HONORED      0x08
#define ROYAL_KLUDGE_MODE_MARQUEE_EFFECT        0x09
#define ROYAL_KLUDGE_MODE_ROTATING_STORM        0x0A
#define ROYAL_KLUDGE_MODE_SERPENTINE_HORSE_RACE 0x0B
#define ROYAL_KLUDGE_MODE_RETRO_SNAKE           0x0C
#define ROYAL_KLUDGE_MODE_DIAGONAL_TRANSFORM    0x0D
#define ROYAL_KLUDGE_MODE_AMBILIGHT             0x0F
#define ROYAL_KLUDGE_MODE_STREAMER              0x10
#define ROYAL_KLUDGE_MODE_STATIC                0x11
#define ROYAL_KLUDGE_MODE_BREATHING             0x12
#define ROYAL_KLUDGE_MODE_NEON                  0x13
#define ROYAL_KLUDGE_MODE_SHADOW_DISAPPEAR      0x14
#define ROYAL_KLUDGE_MODE_FLASH_AWAY            0x15

class RoyalKludgeController
{
public:
    RoyalKludgeController(hid_device* dev_handle, const char* path, const std::string& dev_name);
    ~RoyalKludgeController();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();

    void            SetLEDsDirect(const std::vector<unsigned int>& led_indices, const std::vector<RGBColor>& colors);
    void            SetMode(unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color, bool random_color);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    std::mutex      device_mutex;

    void            SendBuffer(unsigned char* buffer);
    void            SendStandardBuffer(unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color, bool random_color);
};

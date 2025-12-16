/*---------------------------------------------------------*\
| YunziiYZ75KeyboardController.h                            |
|                                                           |
|   Driver for Yunzii YZ75 keyboard                         |
|                                                           |
|   Kosta A. (kostaarvanitis)                   23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include <hidapi.h>
#include "RGBController.h"

enum
{
    YUNZII_MODE_OFF = 0,
    YUNZII_MODE_ALWAYS_ON,
    YUNZII_MODE_DYNAMIC_BREATHING,
    YUNZII_MODE_SPECTRUM_CYCLE,
    YUNZII_MODE_DRIFT,
    YUNZII_MODE_RIPPLE,
    YUNZII_MODE_STARS_TWINKLY,
    YUNZII_MODE_STEADY_STREAM,
    YUNZII_MODE_SHADOWING,
    YUNZII_MODE_PEAKS_RISING,
    YUNZII_MODE_SINE_WAVE,
    YUNZII_MODE_CAISPRING_SURGING,
    YUNZII_MODE_FLOWERS_BLOOMING,
    YUNZII_MODE_LASER,
    YUNZII_MODE_PEAK_TURN,
    YUNZII_MODE_WAVE,
    YUNZII_MODE_NEON,
    YUNZII_MODE_SNAKE,
    YUNZII_MODE_THROUGH_THE_SNOW
};


class YunziiYZ75KeyboardController
{
public:
    YunziiYZ75KeyboardController(hid_device* dev_handle, hid_device_info* dev_info, std::string name);
    ~YunziiYZ75KeyboardController();

    std::string     GetLocation();
    std::string     GetName();
    std::string     GetVendor();
    std::string     GetSerialNumber();
    std::string     GetFirmwareVersion();

    void            SetLightMode(unsigned int mode, RGBColor* color, bool use_random, int speed, int brightness, int direction);
    void            SetLightStripMode(unsigned int mode, RGBColor* color, bool use_random, int speed, int brightness, int direction);

protected:
    void            WriteMode(unsigned char zone, unsigned char mode, unsigned char r, unsigned char g, unsigned char b, unsigned char color_mode, unsigned char speed, unsigned char brightness, unsigned char direction);
    unsigned char   Crc(const unsigned char data[], unsigned int start, unsigned int end);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    std::string     vendor;
    std::string     serial;
    unsigned int    version;
};

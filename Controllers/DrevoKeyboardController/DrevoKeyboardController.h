/*---------------------------------------------------------*\
| DrevoKeyboardController.h                                 |
|                                                           |
|   Driver for DREVO BladeMaster PRO keyboards              |
|                                                           |
|   T-bond                                      27 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| DREVO vendor and product IDs                              |
\*---------------------------------------------------------*/
#define DREVO_VID                                   0x1A2C
#define DREVO_BLADEMASTER_PRO_88_PID                0xB58E

/*---------------------------------------------------------*\
| The keyboard is programmed through 8 byte feature reports |
| (report ID 0x05) of the vendor collection on interface 0; |
| the interface also has keyboard, consumer and mouse       |
| collections                                               |
\*---------------------------------------------------------*/
#define DREVO_INTERFACE                             0
#define DREVO_USAGE_PAGE                            0xFF00
#define DREVO_USAGE                                 0x01
#define DREVO_REPORT_ID                             0x05
#define DREVO_REPORT_SIZE                           8

/*---------------------------------------------------------*\
| Lighting command:                                         |
|   05 FE mode p0 R G B 00                                  |
|   p0: bit 7 use color, bits 4-6 speed, bits 0-3 brightness|
|   Rainbow sends its direction in place of R               |
\*---------------------------------------------------------*/
enum
{
    DREVO_MODE_STATIC                       = 0x01,
    DREVO_MODE_SPECTRUM                     = 0x02,
    DREVO_MODE_RAINBOW                      = 0x03,
    DREVO_MODE_POWER_GAUGE                  = 0x04,
    DREVO_MODE_BREATHING                    = 0x05,
    DREVO_MODE_TWINKLING_STARS              = 0x06,
    DREVO_MODE_REACTIVE                     = 0x07,
    DREVO_MODE_MARQUEE                      = 0x08,
    DREVO_MODE_AURORA                       = 0x09,
    DREVO_MODE_CUSTOM                       = 0x0C,
    DREVO_MODE_OFF                          = 0xFF, /* Static, black, brightness 0  */
};

enum
{
    DREVO_RAINBOW_RIGHT_TO_LEFT             = 0x00,
    DREVO_RAINBOW_LEFT_TO_RIGHT             = 0x01,
    DREVO_RAINBOW_DOWN_TO_UP                = 0x02,
    DREVO_RAINBOW_UP_TO_DOWN                = 0x03,
};

enum
{
    DREVO_BRIGHTNESS_MIN                    = 0x00,
    DREVO_BRIGHTNESS_MAX                    = 0x0F,
    DREVO_SPEED_MIN                         = 0x00,
    DREVO_SPEED_MAX                         = 0x04,
};

/*---------------------------------------------------------*\
| Per-key color packet of the custom mode (408 bytes):      |
|   F3 00 00 layout {R G B} * keys, then the side LEDs      |
| It is sent split into reports: the first one carries 7    |
| bytes (05 d0..d6), the others 6 (05 00 d7..d12, ...)      |
\*---------------------------------------------------------*/
#define DREVO_LED_PACKET_SIZE                       408
#define DREVO_LED_PACKET_HEADER_SIZE                4

/*---------------------------------------------------------*\
| 88 key ISO layout                                         |
\*---------------------------------------------------------*/
#define DREVO_LAYOUT_ISO_88                         0x80
#define DREVO_ISO_88_KEY_COUNT                      88

/*---------------------------------------------------------*\
| Side light bars: 38 LEDs around the keyboard, chained     |
| clockwise from the top left corner. Their data follows    |
| the keys in 41 slots: top 0..13, right 15..19, bottom     |
| 21..34, left 35..39; slots 14, 20 and 40 have no LED.     |
\*---------------------------------------------------------*/
#define DREVO_LIGHT_BAR_TOP_COUNT                   14
#define DREVO_LIGHT_BAR_RIGHT_COUNT                 5
#define DREVO_LIGHT_BAR_BOTTOM_COUNT                14
#define DREVO_LIGHT_BAR_LEFT_COUNT                  5
#define DREVO_LIGHT_BAR_COUNT                       (DREVO_LIGHT_BAR_TOP_COUNT    \
                                                   + DREVO_LIGHT_BAR_RIGHT_COUNT  \
                                                   + DREVO_LIGHT_BAR_BOTTOM_COUNT \
                                                   + DREVO_LIGHT_BAR_LEFT_COUNT)

class DrevoKeyboardController
{
public:
    DrevoKeyboardController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name);
    ~DrevoKeyboardController();

    std::string         GetDeviceLocation();
    std::string         GetNameString();
    std::string         GetSerialString();

    void                SetMode(unsigned char mode_value, bool use_color, unsigned char speed, unsigned char brightness, RGBColor color, unsigned char direction);

    /*-----------------------------------------------------*\
    | Colors of the keys in key index order, followed by    |
    | the side LEDs: top, right, bottom, left bar           |
    \*-----------------------------------------------------*/
    void                SetLEDs(const std::vector<RGBColor>& colors);

private:
    hid_device*         dev;
    std::string         location;
    std::string         name;

    void                SendReport(unsigned char* report);
    void                SendPacket(unsigned char* packet, unsigned int packet_size);
};

/*---------------------------------------------------------*\
| SinowealthController.h                                    |
|                                                           |
|   Driver for Sinowealth mice, including Glorious          |
|                                                           |
|   Niels Westphal (crashniels)                 20 May 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define SINOWEALTH_CONFIG_SIZE         167
#define SINOWEALTH_CONFIG_SIZE_MIN     131
#define SINOWEALTH_CONFIG_REPORT_SIZE  520
#define SINOWEALTH_COMMAND_REPORT_SIZE   6

/*---------------------------------------------------------*\
| Colors of the DPI stages.                                  |
|                                                            |
| On the GXT 970 the logo and the DPI indicator both show    |
| the color of the currently selected stage, so the firmware |
| ties them together and they cannot be driven apart. Every  |
| stage is given the same color, which also keeps them from  |
| changing shade as the user cycles DPI.                     |
|                                                            |
| The block appears to run on to 0x34, but only the six      |
| stages the Trust firmware uses have ever been written, so  |
| the remaining triples are left untouched.                  |
\*---------------------------------------------------------*/
#define SINOWEALTH_DPI_COLOR_OFFSET   0x1D
#define SINOWEALTH_DPI_STAGES            6

/*---------------------------------------------------------*\
| Rebranded variants wire the same controller differently    |
\*---------------------------------------------------------*/
enum sinowealth_variant
{
    /*-----------------------------------------------------*\
    | Glorious, Everest: one LED, packet order is R, B, G    |
    \*-----------------------------------------------------*/
    SINOWEALTH_VARIANT_DEFAULT      = 0,

    /*-----------------------------------------------------*\
    | Trust GXT 970: packet order is R, G, B, and the DPI    |
    | indicator LEDs are separate from the effect LED        |
    \*-----------------------------------------------------*/
    SINOWEALTH_VARIANT_TRUST_GXT970 = 1
};

/*---------------------------------------------------------*\
| LED indices. The effect LED is first so that it keeps the  |
| meaning it has on single LED variants. On the GXT 970 it   |
| is the bottom strip, and the DPI stage colors light the    |
| logo and the DPI indicator together.                       |
\*---------------------------------------------------------*/
enum
{
    SINOWEALTH_LED_EFFECT           = 0,
    SINOWEALTH_LED_DPI              = 1
};

enum
{
    GLORIOUS_MODE_OFF               = 0x00, //does nothing
    GLORIOUS_MODE_RAINBOW           = 0x01,
    GLORIOUS_MODE_STATIC            = 0x02,
    GLORIOUS_MODE_SPECTRUM_BREATING = 0x03,
    GLORIOUS_MODE_TAIL              = 0x04,
    GLORIOUS_MODE_SPECTRUM_CYCLE    = 0x05,
    GLORIOUS_MODE_RAVE              = 0x07,
    GLORIOUS_MODE_EPILEPSY          = 0x08, //not in the official software
    GLORIOUS_MODE_WAVE              = 0x09,
    GLORIOUS_MODE_BREATHING         = 0x0A,
};

enum
{
    GLORIOUS_SPEED_SLOW             = 0x01,
    GLORIOUS_SPEED_NORMAL           = 0x02,
    GLORIOUS_SPEED_FAST             = 0x03,
};

enum
{
    GLORIOUS_BRIGHTNESS_LOW         = 0x01,
    GLORIOUS_BRIGHTNESS_NORMAL      = 0x02,
    GLORIOUS_BRIGHTNESS_HIGH        = 0x04,
};

enum
{
    GLORIOUS_DIRECTION_DOWN         = 0x00,
    GLORIOUS_DIRECTION_UP           = 0x01,
};

class SinowealthController
{
public:
    SinowealthController(hid_device* dev_data_handle, hid_device* dev_cmd_handle, char *_path, std::string dev_name, sinowealth_variant dev_variant = SINOWEALTH_VARIANT_DEFAULT); //RGB, Command, path
    ~SinowealthController();

    unsigned int    GetLEDCount();
    std::string     GetLEDName(unsigned int led_idx);
    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerialString();
    std::string     GetFirmwareVersion();

    void            SetMode(unsigned char mode, unsigned char speed, unsigned char brightness, unsigned char direction, RGBColor* color_buf);
    int             GetProfile();

    /*-----------------------------------------------------*\
    | Read back from the configuration last fetched by       |
    | GetProfile(), so the controller can start out showing  |
    | what the device is already doing                       |
    \*-----------------------------------------------------*/
    unsigned char   GetConfigByte(unsigned int offset);
    RGBColor        GetConfigColor(unsigned int offset);

private:
    void            SetPacketColor(unsigned char* usb_buf, unsigned int offset, RGBColor color);

    hid_device*     dev_cmd;
    hid_device*     dev_data;
    unsigned int    led_count;
    sinowealth_variant variant;
    unsigned char   device_configuration[SINOWEALTH_CONFIG_REPORT_SIZE];
    std::string     location;
    std::string     name;
};

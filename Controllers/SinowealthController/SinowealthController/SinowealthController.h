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

typedef unsigned int RGBColor;

#define SINOWEALTH_CONFIG_SIZE         167
#define SINOWEALTH_CONFIG_SIZE_MIN     131
#define SINOWEALTH_CONFIG_REPORT_SIZE  520
#define SINOWEALTH_COMMAND_REPORT_SIZE   6

enum
{
    GLORIOUS_MODE_OFF               = 0x00, //does nothing
    GLORIOUS_MODE_RAINBOW           = 0x01,
    GLORIOUS_MODE_STATIC_SINGLE     = 0x02,
    GLORIOUS_MODE_SPECTRUM_BREATHING = 0x03,
    GLORIOUS_MODE_TAIL              = 0x04,
    GLORIOUS_MODE_SPECTRUM_CYCLE    = 0x05,
    GLORIOUS_MODE_STATIC            = 0x06, //proper 4-color static mode
    GLORIOUS_MODE_RAVE              = 0x07,
    GLORIOUS_MODE_EPILEPSY          = 0x08, //not in the official software; Everest software calls it "Star Twinkle"
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

enum
{
    GLORIOUS_KEY_ASSIGNMENT_LMB           = 0x1101,
    GLORIOUS_KEY_ASSIGNMENT_RMB           = 0x1102,
    GLORIOUS_KEY_ASSIGNMENT_WHEEL         = 0x1104,
    GLORIOUS_KEY_ASSIGNMENT_FORWARD       = 0x1108,
    GLORIOUS_KEY_ASSIGNMENT_BACK          = 0x1110,

    GLORIOUS_KEY_ASSIGNMENT_SCROLL_UP     = 0x1201,
    GLORIOUS_KEY_ASSIGNMENT_SCROLL_DOWN   = 0x12ff,

    GLORIOUS_KEY_ASSIGNMENT_DPI           = 0x4100, // Cycles DPI presets (along with assigned wheel color)
    GLORIOUS_KEY_ASSIGNMENT_DPI_LOCK      = 0x4200, // 4201 = 200, 4205 = 600 DPI
    GLORIOUS_KEY_ASSIGNMENT_DPI_STEP_UP   = 0x4301,
    GLORIOUS_KEY_ASSIGNMENT_DPI_STEP_DOWN = 0x4302,
    GLORIOUS_KEY_ASSIGNMENT_DPI_UP        = 0x4101,
    GLORIOUS_KEY_ASSIGNMENT_DPI_DOWN      = 0x4102,

    GLORIOUS_KEY_ASSIGNMENT_MODE          = 0x5006, // Switches between 2 internal states
    GLORIOUS_KEY_ASSIGNMENT_RGB           = 0x5007, // Cycles active LED mode
    GLORIOUS_KEY_ASSIGNMENT_NONE          = 0x5001,
};

class SinowealthController
{
public:
    SinowealthController(hid_device* dev_data_handle, hid_device* dev_cmd_handle, char *_path, const std::string &dev_name); //RGB, Command, path
    ~SinowealthController();

    unsigned int    GetPresetCount();
    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerialString();
    std::string     GetFirmwareVersion();

    /*---------------------------------------------------------*\
    | Functions changing the stored PROFILE                     |
    | Note: these do NOT send the updated profile to the device |
    | But they CAN request it if no profile is stored           |
    | or stored profile is invalid                              |
    \*---------------------------------------------------------*/
    void            setMode(unsigned char mode, unsigned char speed, unsigned char brightness, unsigned char direction, RGBColor *color_buf);
    void            setDpiData(unsigned char activeDpiPreset, unsigned char dpiMask, unsigned char* dpiValues, RGBColor* dpiColors); // Receives 6 dpi values

    unsigned int    getBreathingLimit();

    int             requestProfile();
    void            sendProfile();
    bool            isProfileValid() const;
    bool            ensureProfileValid();

    /*------------------------------------------*\
    | This controller does not parse the config  |
    | received from the device aside from        |
    | minimal sanity checks - but it can provide |
    | a copy of the stored config to be parsed   |
    \*------------------------------------------*/
    std::vector<unsigned char> GetRawConfig();

private:
    hid_device*     dev_cmd;
    hid_device*     dev_data;
    std::vector<unsigned char> device_configuration;

    unsigned int    breathing_colors_limit; // 7, but may potentially vary

    std::string     location;
    std::string     name;
};

/*---------------------------------------------------------*\
| RoyuanController.h                                        |
|                                                           |
|   Driver for Royuan keyboards                             |
|                                                           |
|   Alvaro Munoz (alvaromunoz)                  05 Jun 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>

#define ROYUAN_PACKET_LENGTH                      0x40
#define ROYUAN_COMMAND_RGB                        0x07
#define ROYUAN_COMMAND_SET                        0xF60A
#define ROYUAN_COMMAND_PING                       0xF7
#define HID_MAX_STR                               255

enum
{
    ROYUAN_BYTE_COMMAND                           = 0x01,
    ROYUAN_BYTE_MODE                              = 0x02,
    ROYUAN_BYTE_SPEED                             = 0x03,
    ROYUAN_BYTE_BRIGHTNESS                        = 0x04,
    ROYUAN_BYTE_FLAGS                             = 0x05,
    ROYUAN_BYTE_RED                               = 0x06,
    ROYUAN_BYTE_GREEN                             = 0x07,
    ROYUAN_BYTE_BLUE                              = 0x08,
    ROYUAN_BYTE_CRC                               = 0x09
};

enum
{
    ROYUAN_MODE_ALWAYS_ON                         = 0x01,
    ROYUAN_MODE_DYNAMIC_BREATHING                 = 0x02,
    ROYUAN_MODE_SPECTRUM_CYCLE                    = 0x03,
    ROYUAN_MODE_DRIFT                             = 0x04,
    ROYUAN_MODE_WAVES_RIPPLE                      = 0x05,
    ROYUAN_MODE_STARS_TWINKLE                     = 0x06,
    ROYUAN_MODE_STEADY_STREAM                     = 0x07,
    ROYUAN_MODE_SHADOWING                         = 0x08,
    ROYUAN_MODE_PEAKS_RISING_ONE_AFTER_ANOTHER    = 0x09,
    ROYUAN_MODE_SINE_WAVE                         = 0x0A,
    ROYUAN_MODE_CAISPRING_SURGING                 = 0x0B,
    ROYUAN_MODE_FLOWERS_BLOOMING                  = 0x0C,
    ROYUAN_MODE_LASER                             = 0x0E,
    ROYUAN_MODE_PEAK_TURN                         = 0x0F,
    ROYUAN_MODE_INCLINED_RAIN                     = 0x10,
    ROYUAN_MODE_SNOW                              = 0x11,
    ROYUAN_MODE_METEOR                            = 0x12,
    ROYUAN_MODE_THROUGH_THE_SNOW_NON_TRACE        = 0x13,
    ROYUAN_MODE_LIGHT_SHADOW                      = 0x15
};

enum
{
    ROYUAN_SPEED_MIN                              = 0x00,
    ROYUAN_SPEED_MAX                              = 0x05,
    ROYUAN_SPEED_MAX_SPECIAL                      = 0x04,
    ROYUAN_SPEED_DEFAULT                          = 0x04
};

enum
{
    ROYUAN_BRIGHTNESS_MIN                         = 0x00,
    ROYUAN_BRIGHTNESS_MAX                         = 0x04,
    ROYUAN_BRIGHTNESS_DEFAULT                     = 0x04
};

enum
{
    ROYUAN_OPTION_DEFAULT                         = 0x00,
    ROYUAN_OPTION_DAZZLE_OFF                      = 0x07,
    ROYUAN_OPTION_DAZZLE_ON                       = 0x08,
    ROYUAN_OPTION_DRIFT_RIGHT                     = 0X00,
    ROYUAN_OPTION_DRIFT_LEFT                      = 0X10,
    ROYUAN_OPTION_DRIFT_DOWN                      = 0X20,
    ROYUAN_OPTION_DRIFT_UP                        = 0X30,
    ROYUAN_OPTION_STEADY_STREAM_ZIG_ZAG           = 0x00,
    ROYUAN_OPTION_STEADY_STREAM_RETURN            = 0x10,
    ROYUAN_OPTION_CAISPRING_SURGING_OUT           = 0x00,
    ROYUAN_OPTION_CAISPRING_SURGING_IN            = 0x10,
    ROYUAN_OPTION_FLOWERS_BLOOMING_RIGHT          = 0x00,
    ROYUAN_OPTION_FLOWERS_BLOOMING_LEFT           = 0x10,
    ROYUAN_OPTION_PEAK_TURN_ANTI_CLOCKWISE        = 0x00,
    ROYUAN_OPTION_PEAK_TURN_CLOCKWISE             = 0x10,
};

class RoyuanController
{
public:
    RoyuanController(hid_device* dev_handle, hid_device_info* dev_info, std::string dev_name);
    virtual ~RoyuanController();

    std::string GetLocation();
    std::string GetName();
    std::string GetVendor();
    std::string GetSerialNumber();
    std::string GetFirmwareVersion();
    std::string GetDescription();

    void SetDazzle(bool is_dazzle);
    void SetOption(unsigned char option);

    void SetMode(unsigned char mode, unsigned char speed, unsigned char brightness);
    void SetColor(unsigned char red, unsigned char green, unsigned char blue);

protected:
    virtual unsigned char Crc(const unsigned char data[], unsigned int start, unsigned int end);

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;
    std::string             vendor;
    std::string             product;
    std::string             serial;
    unsigned int            version;

    unsigned char           current_mode;
    unsigned char           current_speed;
    unsigned char           current_brightness;
    unsigned char           current_dazzle;
    unsigned char           current_option;

    unsigned char           current_red;
    unsigned char           current_green;
    unsigned char           current_blue;

    void SendUpdate();
};

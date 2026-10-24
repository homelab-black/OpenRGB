/*---------------------------------------------------------*\
| MonsGeekMG75SHEController.h                               |
|                                                           |
|   Driver for the MonsGeek MG75S HE keyboard               |
|                                                           |
|   Sergey Tsai (mustang651)                 14 Sep 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstddef>
#include <string>
#include <hidapi.h>
#include "RGBController.h"

#define MONSGEEK_MG75S_HE_PACKET_SIZE           64
#define MONSGEEK_MG75S_HE_REPORT_SIZE           (MONSGEEK_MG75S_HE_PACKET_SIZE + 1)
#define MONSGEEK_MG75S_HE_LED_COUNT             83
#define MONSGEEK_MG75S_HE_MATRIX_LED_COUNT      128
#define MONSGEEK_MG75S_HE_MATRIX_DATA_SIZE      (MONSGEEK_MG75S_HE_MATRIX_LED_COUNT * 3)

enum
{
    MONSGEEK_MG75S_HE_MODE_OFF                  = 0x00,
    MONSGEEK_MG75S_HE_MODE_SOLID                = 0x01,
    MONSGEEK_MG75S_HE_MODE_BREATHING            = 0x02,
    MONSGEEK_MG75S_HE_MODE_SPECTRUM             = 0x03,
    MONSGEEK_MG75S_HE_MODE_WAVE                 = 0x04,
    MONSGEEK_MG75S_HE_MODE_RIPPLE               = 0x05,
    MONSGEEK_MG75S_HE_MODE_RAIN                 = 0x06,
    MONSGEEK_MG75S_HE_MODE_SNAKE                = 0x07,
    MONSGEEK_MG75S_HE_MODE_REACTIVE             = 0x08,
    MONSGEEK_MG75S_HE_MODE_CONVERGENCE          = 0x09,
    MONSGEEK_MG75S_HE_MODE_SINE_WAVE            = 0x0A,
    MONSGEEK_MG75S_HE_MODE_KALEIDOSCOPE         = 0x0B,
    MONSGEEK_MG75S_HE_MODE_LINE_WAVE            = 0x0C,
    MONSGEEK_MG75S_HE_MODE_USER_PICTURE         = 0x0D,
    MONSGEEK_MG75S_HE_MODE_LASER                = 0x0E,
    MONSGEEK_MG75S_HE_MODE_CIRCLE_WAVE          = 0x0F,
    MONSGEEK_MG75S_HE_MODE_DAZZLING             = 0x10,
    MONSGEEK_MG75S_HE_MODE_RAIN_DOWN            = 0x11,
    MONSGEEK_MG75S_HE_MODE_METEOR               = 0x12,
    MONSGEEK_MG75S_HE_MODE_REACTIVE_OFF         = 0x13,
    MONSGEEK_MG75S_HE_MODE_TRAIN                = 0x17,
    MONSGEEK_MG75S_HE_MODE_FIREWORKS            = 0x18
};

enum
{
    MONSGEEK_MG75S_HE_SPEED_MIN                 = 0,
    MONSGEEK_MG75S_HE_SPEED_MAX                 = 4,
    MONSGEEK_MG75S_HE_SPEED_DEFAULT             = 2,
    MONSGEEK_MG75S_HE_BRIGHTNESS_MIN            = 0,
    MONSGEEK_MG75S_HE_BRIGHTNESS_MAX            = 4,
    MONSGEEK_MG75S_HE_BRIGHTNESS_DEFAULT        = 4
};

enum
{
    MONSGEEK_MG75S_HE_OPTION_RANDOM             = 0x07,
    MONSGEEK_MG75S_HE_OPTION_SOLID              = 0x08
};

class MonsGeekMG75SHEController
{
public:
    MonsGeekMG75SHEController(hid_device* device, const char* path);
    ~MonsGeekMG75SHEController();

    std::string GetDeviceName();
    std::string GetSerial();
    std::string GetLocation();

    bool SetMode(unsigned char mode, unsigned char speed, unsigned char brightness, unsigned char option, RGBColor color);
    bool SetStatic(const RGBColor* colors, const unsigned char* hardware_indices, size_t count);
    bool SetStaticLED(unsigned char hardware_index, RGBColor color);

private:
    static void ApplyChecksum(unsigned char* payload, size_t checksum_index);
    bool SendFeatureReport(const unsigned char* payload);

    hid_device* device;
    std::string location;
};

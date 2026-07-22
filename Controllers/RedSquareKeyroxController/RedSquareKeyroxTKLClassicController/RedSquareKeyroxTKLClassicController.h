/*---------------------------------------------------------*\
| RedSquareKeyroxTKLClassicController.h                     |
|                                                           |
|   Driver for Red Square Keyrox TKL Classic                |
|                                                           |
|   vlack                                       03 May 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include "KeyboardLayoutManager.h"

#define CLASSIC_PACKET_DATA_LENGTH      64

/*---------------------------------------------------------*\
| The device info request answers with a string such as     |
|                                                           |
|     2M252,01,KB,JC,GK8120SKRGB,V1.04.04                   |
|                                                           |
| where the fifth field is the model and the last is the    |
| firmware version.  Only the full size board reports the   |
| GK8120 model, so any other answer keeps the TKL layout.   |
\*---------------------------------------------------------*/
#define CLASSIC_INFO_COMMAND            0x0D
#define CLASSIC_INFO_RETRIES            10
#define CLASSIC_INFO_TIMEOUT            150
#define CLASSIC_FULL_SIZE_MODEL         "GK8120"

/*---------------------------------------*\
|  Modes                                  |
\*---------------------------------------*/
enum
{
    CLASSIC_CONST_MODE_VALUE                  = 0x00, // static
    CLASSIC_BREATHE_MODE_VALUE                = 0x01, // breath
    CLASSIC_WAVE_MODE_VALUE                   = 0x02, // wave
    CLASSIC_FADE_MODE_VALUE                   = 0x03, // neon
    CLASSIC_RADAR_MODE_VALUE                  = 0x04, // radar
    CLASSIC_STAR_MODE_VALUE                   = 0x06, // интерактив
    CLASSIC_LINE_MODE_VALUE                   = 0x07, // сияние
    CLASSIC_RIPPLE_MODE_VALUE                 = 0x08, // рябь интерактив
    CLASSIC_STARS_MODE_VALUE                  = 0x09, // мерцание
    CLASSIC_CUSTOM_MODE_VALUE                 = 0x0A,
    CLASSIC_CROSS_MODE_VALUE                  = 0x0B, // скрещивание
    CLASSIC_WTF_MODE_VALUE                    = 0x0C, // быстрый отклик
    CLASSIC_RIPPLE_RANDOM_MODE_VALUE          = 0x0E, // рябь
    CLASSIC_RUNNING_LINE_MODE_VALUE           = 0x0F, // бегущая строка
    CLASSIC_FIREWORK_MODE_VALUE               = 0x10, // firework
};

/*-----------------------------*\
|  Other settings               |
\*-----------------------------*/
enum
{
    CLASSIC_KEYROX_BRIGHTNESS_MIN = 0x00,
    CLASSIC_KEYROX_BRIGHTNESS_MAX = 0x04,
    CLASSIC_KEYROX_SPEED_MIN      = 0x00,
    CLASSIC_KEYROX_SPEED_MAX      = 0x04,
};


class RedSquareKeyroxTKLClassicController
{
public:
    RedSquareKeyroxTKLClassicController(hid_device *dev_handle, const hid_device_info &info, std::string dev_name);
    ~RedSquareKeyroxTKLClassicController();

    std::string         GetDeviceLocation();
    std::string         GetNameString();
    std::string         GetSerialString();
    std::string         GetVendorString();
    std::string         GetVersionString();
    KEYBOARD_SIZE       GetKeyboardSize();

    int                 GetDirection(int direction);
    int                 GetDirectionRound(int direction);

    void                SetMode(mode m);
    void                SetLEDsData(std::vector<RGBColor> colors, std::vector<led> leds);

protected:
    hid_device*         dev;

private:
    void                        ReadDeviceInfo();

    std::string                 location;
    std::string                 name;
    std::string                 serial_number;
    std::string                 vendor;
    std::string                 version;
    KEYBOARD_SIZE               keyboard_size;
    std::vector<unsigned int>   led_sequence_positions;
};

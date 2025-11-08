/*---------------------------------------------------------*\
| CMARGBStripController.h                                   |
|                                                           |
|   Driver for Cooler Master ARGB light strip devices       |
|                                                           |
|   Morgan Guimard (morg)                       18 Sep 2023 |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

#define CM_ARGB_STRIP_PACKET_LENGTH         65
#define CM_ARGB_STRIP_FIRST_PACKET_DATA_SIZE 58   // CM_ARGB_STRIP_PACKET_LENGTH - 7 (header bytes)
#define CM_ARGB_STRIP_PACKET_DATA_SIZE      63   // CM_ARGB_STRIP_PACKET_LENGTH - 2 (header bytes)

enum
{
    CM_ARGB_STRIP_DIRECT_MODE         = 0xFF,
    CM_ARGB_STRIP_CUSTOM_MODE         = 0xFE,
    CM_ARGB_STRIP_SPECTRUM_MODE       = 0x00,
    CM_ARGB_STRIP_RELOAD_MODE         = 0x01,
    CM_ARGB_STRIP_RECOIL_MODE         = 0x02,
    CM_ARGB_STRIP_BREATHING_MODE      = 0x03,
    CM_ARGB_STRIP_REFILL_MODE         = 0x04,
    CM_ARGB_STRIP_OFF_MODE            = 0x06
};

enum
{
    CM_ARGB_STRIP_BRIGHTNESS_MAX      = 0xFF,
    CM_ARGB_STRIP_BRIGHTNESS_MIN      = 0x00,
    CM_ARGB_STRIP_SPEED_MAX           = 0x04,
    CM_ARGB_STRIP_SPEED_MIN           = 0x00,
};

struct CMARGBStripConfig
{
    unsigned int    leds_per_strip;
    unsigned int    num_strips;
    const uint8_t*  strip_ids;
};

class CMARGBStripController
{
public:
    CMARGBStripController(hid_device* dev_handle, const hid_device_info& info, const std::string& name, const CMARGBStripConfig& config, bool reset, bool broadcast);
    ~CMARGBStripController();

    std::string     GetDeviceName();
    std::string     GetSerialString();
    std::string     GetDeviceLocation();
    unsigned int    GetLEDCount();
    unsigned int    GetStripCount();

    void            SetMode(uint8_t mode_value, uint8_t speed, uint8_t brightness, const RGBColor& color, bool random);
    void            SendColorData(const std::vector<RGBColor>& colors, uint8_t command, uint8_t mode_byte, uint8_t brightness, bool enable_software_mode);

private:
    std::string     device_name;
    std::string     location;
    hid_device*     dev;
    CMARGBStripConfig config;
    bool            is_software_mode_enabled;

    void            SetControlMode(bool value);
    void            ResetDevice();
};

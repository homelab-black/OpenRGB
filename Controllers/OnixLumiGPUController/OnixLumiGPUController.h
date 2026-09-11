/*---------------------------------------------------------*\
| OnixLumiGPUController.h                                   |
|                                                           |
|   Driver for ONIX LUMI Intel Arc B580                     |
|                                                           |
|   Reverse engineered from ONIX LUMI v2.1                  |
|                                                           |
|   Jan Dvořák                                  11 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "i2c_smbus.h"

typedef unsigned char onix_lumi_dev_id;

enum
{
    ONIX_LUMI_REG_MODE          = 0x10,
    ONIX_LUMI_REG_RUNWAY_RESPONSE  = 0x11,
    ONIX_LUMI_REG_RUNWAY_INTERVAL  = 0x12,
    ONIX_LUMI_REG_ONE_COLOR_RESPONSE = 0x13,
    ONIX_LUMI_REG_DIRECTION        = 0x14,
    ONIX_LUMI_REG_SERIAL_RESPONSE = 0x16,
    ONIX_LUMI_REG_SERIAL_INTERVAL = 0x17,
    ONIX_LUMI_REG_BLOCK_STACKING_SPEED = 0x20,
    ONIX_LUMI_REG_RAINBOW_RESPONSE = 0x18,
    ONIX_LUMI_REG_RAINBOW_PATTERN = 0x19,
    ONIX_LUMI_REG_RED           = 0x1A,
    ONIX_LUMI_REG_GREEN         = 0x1B,
    ONIX_LUMI_REG_BLUE          = 0x1C,
    ONIX_LUMI_REG_RUNWAY_BOUNCE    = 0x29,
    ONIX_LUMI_REG_BRIGHTNESS    = 0x3E,

    ONIX_LUMI_REG_BREATHING_TEMPO = 0xC8,
    ONIX_LUMI_REG_BREATHING_RED   = 0xC9,
    ONIX_LUMI_REG_BREATHING_GREEN = 0xCA,
    ONIX_LUMI_REG_BREATHING_BLUE  = 0xCB,
};

enum
{
    ONIX_LUMI_HW_MODE_RAINBOW   = 0x00,
    ONIX_LUMI_HW_MODE_STATIC    = 0x01,
    ONIX_LUMI_HW_MODE_BREATHING = 0x02,
    ONIX_LUMI_HW_MODE_SERIAL    = 0x03,
    ONIX_LUMI_HW_MODE_RUNWAY    = 0x04,
    ONIX_LUMI_HW_MODE_COLOR_CYCLE = 0x05,
    ONIX_LUMI_HW_MODE_BLOCK_STACKING = 0x06,
};

enum
{
    ONIX_LUMI_MODE_OFF          = 0xFF,
    ONIX_LUMI_MODE_STATIC       = 0x01,
    ONIX_LUMI_MODE_RAINBOW      = 0x00,
    ONIX_LUMI_MODE_BREATHING    = 0x02,
    ONIX_LUMI_MODE_SERIAL       = 0x03,
    ONIX_LUMI_MODE_RUNWAY       = 0x04,
    ONIX_LUMI_MODE_COLOR_CYCLE    = 0x05,
    ONIX_LUMI_MODE_BLOCK_STACKING = 0x06,
    ONIX_LUMI_MODE_RUNWAY_BOUNCE = 0x104,
};

class OnixLumiGPUController
{
public:
    OnixLumiGPUController(i2c_smbus_interface* bus,
                          onix_lumi_dev_id dev,
                          std::string name);
    ~OnixLumiGPUController();

    std::string GetDeviceName();
    std::string GetDeviceLocation();

    void SetOff();
    void SetStatic(unsigned char red,
                   unsigned char green,
                   unsigned char blue,
                   unsigned char brightness);
    void SetRainbow(unsigned char brightness,
                    unsigned char speed,
                    unsigned char direction);
    void SetBreathing(unsigned char red,
                      unsigned char green,
                      unsigned char blue,
                      unsigned char brightness,
                      unsigned char speed);    void SetColorCycle(unsigned char brightness,
                       unsigned char speed);    void SetBlockStacking(unsigned char brightness,
                          unsigned char speed,
                          unsigned char direction);

    void SetSerial(unsigned char brightness,
                   unsigned char speed,
                   unsigned char direction);


    void SetRunway(unsigned char brightness,
                   unsigned char speed,
                   unsigned char direction,
                   bool bounce);

private:
    i2c_smbus_interface* bus;
    onix_lumi_dev_id     dev;
    std::string          name;

    void WriteRegister(u8 reg, u8 value);
};

#define ONIX_LUMI_BREATHING_SPEED_MAX 15

#define ONIX_LUMI_RAINBOW_PATTERN_DEFAULT 0x05

/*---------------------------------------------------------*\
|   GamepowerEva5Controller.h                               |
|                                                           |
|   Driver for Gamepower Eva 5 keyboard                     |
|                                                           |
|   Made by FeusX  (23 Aug 2026)                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#ifndef GAMEPOWEREVA5CONTROLLER_H
#define GAMEPOWEREVA5CONTROLLER_H

#include <string>
#include <hidapi.h>
#include "RGBController.h"

enum
{
    GAMEPOWER_EVA5_MODE_STEADY = 0x00,
    GAMEPOWER_EVA5_MODE_SPECIAL_KEY = 0x01,
    GAMEPOWER_EVA5_MODE_BREATHING = 0x02,
    GAMEPOWER_EVA5_MODE_PRESS_AND_DESTROY = 0x03,
    GAMEPOWER_EVA5_MODE_NEON_STREAM = 0x04,
    GAMEPOWER_EVA5_MODE_STREAMER = 0x05,
    GAMEPOWER_EVA5_MODE_FLOWING_LIGHT_AND_COLOR = 0x06,
    GAMEPOWER_EVA5_MODE_DRIPPING_RIPPLES = 0x07,
    GAMEPOWER_EVA5_MODE_BRILLIANT_POINT = 0x08,
    GAMEPOWER_EVA5_MODE_FLASH_AWAY = 0x09,
    GAMEPOWER_EVA5_MODE_SHADOW_DISAPPER = 0x0A,
    GAMEPOWER_EVA5_MODE_RIPPLES_SHINING = 0x0B,
    GAMEPOWER_EVA5_MODE_RICH_AND_HONOURED = 0x0C,
    GAMEPOWER_EVA5_MODE_MARQUEE_EFFECT = 0x0D,
    GAMEPOWER_EVA5_MODE_ROTATING_STORM = 0x0E,
    GAMEPOWER_EVA5_MODE_SERPENTINE_HORSE_RACE = 0x0F,
    GAMEPOWER_EVA5_MODE_STARS_TWINKLE = 0x10,
    GAMEPOWER_EVA5_MODE_RETRO_SNAKE = 0x11,
    GAMEPOWER_EVA5_MODE_DIAGONAL_TRANSFORMATION = 0x12,
    GAMEPOWER_EVA5_MODE_SINE_WAVE = 0x13
};

enum
{
    GAMEPOWER_EVA5_BRIG_LVL0 = 0x00,
    GAMEPOWER_EVA5_BRIG_LVL1 = 0x01,
    GAMEPOWER_EVA5_BRIG_LVL2 = 0x02,
    GAMEPOWER_EVA5_BRIG_LVL3 = 0x03,
    GAMEPOWER_EVA5_BRIG_LVL4 = 0x04,
    GAMEPOWER_EVA5_BRIG_LVL5 = 0x05
};

enum
{
    GAMEPOWER_EVA5_SPEED_LVL0 = 0x00,
    GAMEPOWER_EVA5_SPEED_LVL1 = 0x01,
    GAMEPOWER_EVA5_SPEED_LVL2 = 0x02
};

class GamepowerEva5Controller
{
public:
    GamepowerEva5Controller(hid_device* dev_handle, const char* path, std::string dev_name);
    ~GamepowerEva5Controller();

    std::string GetDeviceLocation();
    std::string GetDeviceName();
    std::string GetSerialString();

    void SetMode(unsigned char mode, unsigned char brig, unsigned char speed);

private:
    hid_device* dev;
    std::string location;
    std::string name;

    void SendPayload(const unsigned char* data, short length);
};

#endif

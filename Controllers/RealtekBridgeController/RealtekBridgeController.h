/*---------------------------------------------------------*\
| RealtekBridgeController.h                                 |
|                                                           |
|   Controller for Realtek USB to SSD Bridge ICs            |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#define CONFIG_HID
#define CONFIG_HID_ALL

#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <cstring>
#include <vector>

#include "RGBController.h"
#include <QDebug>
#include "platform.h"
#include "bridge_dev.h"
#include "bridge_func.h"
#include "led.h"
#include "led_effect.h"

#define REALTEK_BRIDGE_SYNC_METHOD_OPENRGB       6
#define REALTEK_BRIDGE_COLOR_DEPTH               3

enum REALTEK_BRIDGE_LED_EFF
{
    REALTEK_BRIDGE_LED_EFF_NONE,
    REALTEK_BRIDGE_LED_EFF_ALWAYS,
    REALTEK_BRIDGE_LED_EFF_BLINK,
    REALTEK_BRIDGE_LED_EFF_BREATHE,
    REALTEK_BRIDGE_LED_EFF_SPECTRUM,
    REALTEK_BRIDGE_LED_EFF_SCROLL,
    REALTEK_BRIDGE_LED_EFF_RAINBOW_SCROLL,
    REALTEK_BRIDGE_LED_EFF_RUNNING_WATER,
    REALTEK_BRIDGE_LED_EFF_SLIDING,
    REALTEK_BRIDGE_LED_EFF_NEWTON_CRADLE,
    REALTEK_BRIDGE_LED_EFF_METEOR,
    REALTEK_BRIDGE_NUMBER_OF_LED_EFF_MODE,
};

enum REALTEK_BRIDGE_SPEED
{
    REALTEK_BRIDGE_SPEED_MIN                     = 1,
    REALTEK_BRIDGE_SPEED_NORMAL                  = 50,
    REALTEK_BRIDGE_SPEED_MAX                     = 100,
};

enum REALTEK_BRIDGE_CYCLE_MS
{
    REALTEK_BRIDGE_CYCLE_MIN                     = 200,
    REALTEK_BRIDGE_CYCLE_NORMAL                  = 2000,
    REALTEK_BRIDGE_CYCLE_MAX                     = 10000,
};

class RealtekBridgeController
{
public:
    RealtekBridgeController(dHandle fd, struct led_mem* led_mem_dev, hid_device_info* info);
    ~RealtekBridgeController();

    std::string get_manu_name();
    std::string get_product_name();
    std::string get_sn();
    std::string get_dev_loc();
    std::string get_fw_ver();
    int get_argb_num();
    int get_argb_brightness();

    int set_argb_direct(std::vector<RGBColor> color_buf, unsigned short brightness);
    int set_argb_effect(uint8_t mode, std::vector<RGBColor> color_buf, int speed, unsigned short brightness);

private:
    dHandle fd = NULL_DHANDLE;
    struct led_mem* led_mem_dev = NULL;
    hid_device_info* hidinfo = NULL;
};

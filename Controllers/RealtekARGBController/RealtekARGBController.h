/*---------------------------------------------------------*\
| RealtekARGBController.h                                   |
|                                                           |
|   Controller for Realtek USB ARGB ICs                     |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#define CONFIG_HID
#define CONFIG_HID_ALL
#define CONFIG_ARGB_DEV

#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <cstring>
#include <vector>
#include <thread>

#include "RGBController.h"
#include <QDebug>
#include <QThread>
#include "platform.h"
#include "bridge_func.h"
#include "argb_dev.h"
#include "argb_ctl.h"
#include "fwupdate.h"

#define REALTEK_ARGB_VID                       0x0BDA
#define REALTEK_ARGB_PID                       0x9209

#define REALTEK_ARGB_NUM_ARGB_GRP              5
#define REALTEK_ARGB_BRINTF_TYPE_HID           1
#define REALTEK_ARGB_SYNC_METHOD_OPENRGB       6
#define REALTEK_ARGB_COLOR_DEPTH               3
#define REALTEK_ARGB_MAX                       400

enum REALTEK_ARGB_LED_CTL
{
    REALTEK_ARGB_LED_CTL_FW,
    REALTEK_ARGB_LED_CTL_APP,
    REALTEK_ARGB_LED_CTL_LAMP,
};

enum REALTEK_ARGB_EFFECT_ID {
    REALTEK_ARGB_EFF_NULL,
    REALTEK_ARGB_EFF_ALWAYS_ON,
    REALTEK_ARGB_EFF_BLINK,
    REALTEK_ARGB_EFF_BREATH,
    REALTEK_ARGB_EFF_SPECTRUM,
    REALTEK_ARGB_EFF_SCROLL,
    REALTEK_ARGB_EFF_RAINBOW_SCROLL,
    REALTEK_ARGB_EFF_RUNNING_WATER,
    REALTEK_ARGB_EFF_SLIDING,
    REALTEK_ARGB_EFF_NEWTON_CRADLE,
    REALTEK_ARGB_EFF_METEOR,
    REALTEK_ARGB_EFF_RAINBOW_SLIDING,
    REALTEK_ARGB_EFF_RAINBOW_FADE_SLIDING,
    REALTEK_ARGB_EFF_WIDE_SLIDING,
    REALTEK_ARGB_EFF_DOT_MATRIX,                    // reserved: Not supported by OpenRGB
    REALTEK_ARGB_EFF_DOT_MATRIX_BREATH,             // reserved: Not supported by OpenRGB
    REALTEK_ARGB_EFF_ZIGZAG,
    REALTEK_ARGB_EFF_STARRY_NIGHT,
    REALTEK_ARGB_EFF_STACK,
};

enum REALTEK_ARGB_SPEED
{
    REALTEK_ARGB_SPEED_MIN                     = 1,
    REALTEK_ARGB_SPEED_NORMAL                  = 50,
    REALTEK_ARGB_SPEED_MAX                     = 100,
};

enum REALTEK_ARGB_CYCLE_MS
{
    REALTEK_ARGB_CYCLE_MIN                     = 200,
    REALTEK_ARGB_CYCLE_NORMAL                  = 2000,
    REALTEK_ARGB_CYCLE_MAX                     = 10000,
};

enum REALTEK_ARGB_CUST_DEVNAME
{
    REALTEK_ARGB_CUST_DEVNAME_NULL             = 0x0,
    REALTEK_ARGB_CUST_DEVNAME_MANU_UUID        = 0x1,
};

struct RealtekARGBControllerSetEffParam
{
    unsigned int speed;
    unsigned short brightness;
    unsigned char dir;
    unsigned char random_color;
};

class RealtekARGBController
{
public:
    RealtekARGBController(struct argb_device* dev, argbCtl* argbctl, hid_device_info* info);
    ~RealtekARGBController();

    void argb_reinit();
    std::string get_manu_name();
    std::string get_product_name();
    std::string get_sn();
    std::string get_dev_loc();
    std::string get_fw_ver();
    std::string get_ic_uuid();
    std::string get_dev_name();
    int get_fix_grps();
    int get_argb_num(int grp_num);
    int get_argb_brightness(int grp_num);

    int set_argb_num(int grp_num, int new_num);
    int set_argb_direct(int grp_num, std::vector<RGBColor> color_buf, unsigned short brightness);
    int set_argb_effect(int grp_num, uint8_t mode, std::vector<RGBColor> color_buf, struct RealtekARGBControllerSetEffParam* param);

    argbCtl* argb_ctl = NULL;
private:
    struct argb_device* adev;
    std::mutex my_mutex;
    hid_device_info* hidinfo = NULL;
    int appctl[REALTEK_ARGB_NUM_ARGB_GRP] = {REALTEK_ARGB_LED_CTL_FW};
};

/*---------------------------------------------------------*\
| RealtekBridgeController.cpp                               |
|                                                           |
|   Controller for Realtek USB to SSD Bridge ICs            |
|                                                           |
|   Jerry Fan (JerryFan0612)                    13 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#ifndef __FreeBSD__

#include "RealtekBridgeController.h"
#include <StringUtils.h>
#include <MathUtils.h>


RealtekBridgeController::RealtekBridgeController(dHandle fd, struct led_mem* led_mem_dev, hid_device_info* info)
{
    this->fd = fd;
    this->led_mem_dev = led_mem_dev;
    this->hidinfo = info;

    led_set_appctl(fd, true);
}

RealtekBridgeController::~RealtekBridgeController()
{
    led_set_appctl(fd, false);

    if(!INVALID_DHANDLE(fd))
        bridge_close_handle(fd);

    if(led_mem_dev) {
        led_mem_deinit(led_mem_dev);
        free(led_mem_dev);
        led_mem_dev = NULL;
    }
}

std::string RealtekBridgeController::get_manu_name()
{
    return StringUtils::wchar_to_char(hidinfo->manufacturer_string);
}

std::string RealtekBridgeController::get_product_name()
{
    return StringUtils::wchar_to_char(hidinfo->product_string);
}

std::string RealtekBridgeController::get_sn()
{
    return StringUtils::wchar_to_char(hidinfo->serial_number);
}

std::string RealtekBridgeController::get_dev_loc()
{
    return hidinfo->path;
}

std::string RealtekBridgeController::get_fw_ver()
{
    struct bridge_fw_version fw_ver;
    std::string ver = "";

    bridge_get_fw_ver(fd, &fw_ver);
    ver += std::to_string(fw_ver.fw_major_ver) + "." +
           std::to_string(fw_ver.fw_minor_ver) + "." +
           std::to_string(fw_ver.fw_extra_ver) + "." +
           std::to_string(fw_ver.fw_build_date);
    return ver;
}

int RealtekBridgeController::get_argb_num()
{
    int ret = -1;
    int num_rgb = 0;

    ret = led_mem_get_rgb_num(led_mem_dev, &num_rgb);
    if(ret)
    {
        num_rgb = 0;
        goto exit;
    }
exit:
    return num_rgb;
}

int RealtekBridgeController::get_argb_brightness()
{
    int ret = -1;
    uint16_t bright = 0;

    ret = led_mem_get_bright(fd, led_mem_dev, &bright);
    if(ret)
        bright = 0;

    return bright;
}

int RealtekBridgeController::set_argb_direct(std::vector<RGBColor> color_buf, unsigned short brightness)
{
    int ret = -1;
    int color_num = color_buf.size();
    int buf_len = color_num * REALTEK_BRIDGE_COLOR_DEPTH;
    static unsigned short prev_bright = 0xFFFF;
    uint8_t* buf;

    if(color_num <= 0)
        goto exit;

    ret = led_set_appctl(fd, true);
    if(ret)
        goto exit;

    if(prev_bright != brightness)
    {
        prev_bright = brightness;
        ret = led_mem_set_bright(fd, led_mem_dev, brightness << 8, true);
        if(ret)
            goto exit;
    }

    buf = (uint8_t*)malloc(buf_len);
    memset(buf, 0, buf_len);
    for(int i = 0; i < color_num; i++)
    {
        buf[i * REALTEK_BRIDGE_COLOR_DEPTH + 0] = RGBGetRValue(color_buf[i]);
        buf[i * REALTEK_BRIDGE_COLOR_DEPTH + 1] = RGBGetGValue(color_buf[i]);
        buf[i * REALTEK_BRIDGE_COLOR_DEPTH + 2] = RGBGetBValue(color_buf[i]);
    }
    ret = led_mem_write_rgb_direct(fd, led_mem_dev, buf, color_num);
    free(buf);
    if(ret)
        goto exit;
exit:
    return ret;
}

int RealtekBridgeController::set_argb_effect(uint8_t mode, std::vector<RGBColor> color_buf, int speed, unsigned short brightness)
{
    int ret = -1;
    int cycle = MathUtils::int_interpolate(REALTEK_BRIDGE_CYCLE_MAX, REALTEK_BRIDGE_CYCLE_MIN, 0, REALTEK_BRIDGE_SPEED_MAX, speed);
    RGBQUAD rgb = { 0, 0, 0, 0 };
    static unsigned short prev_bright = 0xFFFF;

    if(color_buf.size() >= 1)
    {
        rgb.rgbRed = color_buf[0] & 0xFF;
        rgb.rgbGreen = (color_buf[0] & 0xFF00) >> 8;
        rgb.rgbBlue = (color_buf[0] & 0xFF0000) >> 16;
    }

    if(prev_bright != brightness)
    {
        prev_bright = brightness;
        ret = led_mem_set_bright(fd, led_mem_dev, brightness << 8, false);
        if(ret)
            goto exit;
    }

    switch(mode)
    {
    case REALTEK_BRIDGE_LED_EFF_ALWAYS:
        ret = led_eff_set_always_on(fd, led_mem_dev, rgb);
        break;
    case REALTEK_BRIDGE_LED_EFF_BLINK:
        ret = led_eff_set_blink(fd, led_mem_dev, rgb, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_BREATHE:
        ret = led_eff_set_breathe(fd, led_mem_dev, rgb, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_SPECTRUM:
        ret = led_eff_set_spectrum(fd, led_mem_dev, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_SCROLL:
        ret = led_eff_set_scroll(fd, led_mem_dev, rgb, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_RAINBOW_SCROLL:
        ret = led_eff_set_rainbow_scroll(fd, led_mem_dev, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_RUNNING_WATER:
        ret = led_eff_set_running_water(fd, led_mem_dev, rgb, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_SLIDING:
        ret = led_eff_set_sliding(fd, led_mem_dev, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_NEWTON_CRADLE:
        ret = led_eff_set_newton_cradle(fd, led_mem_dev, cycle);
        break;
    case REALTEK_BRIDGE_LED_EFF_METEOR:
        ret = led_eff_set_meteor(fd, led_mem_dev, rgb, cycle);
        break;
    default:
        break;
    }
exit:
    return ret;
}

#endif

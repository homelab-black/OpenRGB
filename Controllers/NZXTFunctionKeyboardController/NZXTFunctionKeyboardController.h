/*---------------------------------------------------------*\
| NZXTFunctionKeyboardController.h                          |
|                                                           |
|   Driver for NZXT Function per-key RGB keyboard (ITE 8291)|
|                                                           |
|   P4tchW0rk (aekira@proton.me)          June 2026         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "libusb.h"
#include <vector>
#include <tuple>
#include <map>

/*-----------------------------------------------------*\
| ITE 8291 keyboard defines                             |
\*-----------------------------------------------------*/
#define NZXT_FUNCTION_KEYBOARD_RGB_SEND_ENDPOINT     2
#define NZXT_FUNCTION_KEYBOARD_RGB_RECV_ENDPOINT 0x82
#define NZXT_FUNCTION_KEYBOARD_DATA_SIZE 64
#define NZXT_FUNCTION_KEYBOARD_RGB_INTERFACE_ID 1
#define NZXT_FUNCTION_KEYBOARD_MODE_DIRECT 1
#define NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MIN 0
#define NZXT_FUNCTION_KEYBOARD_BRIGHTNESS_MAX 0xff
#define NZXT_FUNCTION_KEYBOARD_MODE_RAINBOW 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_WAVE 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_BREATH 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_REACTIVE 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_RIPPLE 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_MARQUEE 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_RAINDROP 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_AURORA 0x0
#define NZXT_FUNCTION_KEYBOARD_MODE_SPARK 0x0
#define NZXT_FUNCTION_KEYBOARD_SPEED_MAX 0xf
#define NZXT_FUNCTION_KEYBOARD_SPEED_MIN 0x0
#define NZXT_FUNCTION_KEYBOARD_NUM_ROWS        6
#define NZXT_FUNCTION_KEYBOARD_NUM_COLS        21
#define NZXT_FUNCTION_KEYBOARD_NUM_LEDS    106//    (NZXT_FUNCTION_KEYBOARD_NUM_ROWS * NZXT_FUNCTION_KEYBOARD_NUM_COLS)

class NZXTFunctionKeyboardController
{
public:
    NZXTFunctionKeyboardController(const hid_device_info& info);
    ~NZXTFunctionKeyboardController();
    void setStaticColor(unsigned char red, unsigned char blue, unsigned char green);
    std::string     GetDeviceLocation();
    std::string     GetSerialString();
    std::string     GetFirmwareVersion();
    void            SendColors(std::map<std::tuple<int, int, int>, std::vector<unsigned int>> leds_states_by_color, unsigned char brightness);
private:
    libusb_context* ctx = nullptr;
    libusb_device_handle*     handle = nullptr;
    void send(unsigned char* rgb_data);
    void receive();
};

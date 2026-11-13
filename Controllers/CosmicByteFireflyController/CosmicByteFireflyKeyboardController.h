/*---------------------------------------------------------*\
| CosmicByteFireflyKeyboardController.h                     |
|                                                           |
|   Driver for Cosmic Byte Firefly TKL Keyboard             |
|   (Configured for Animations, Static RGB & Per-Key RGB)   |
|                                                           |
|   Akankshya Das                              25 Aug 2026  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>
#include <hidapi.h>
#include "RGBController.h"

class CosmicByteFireflyKeyboardController
{
public:
    CosmicByteFireflyKeyboardController(hid_device* dev_handle, const char* path, unsigned short pid, std::string name);
    ~CosmicByteFireflyKeyboardController();

    std::string GetLocation();
    std::string GetName();

    void SendInitialize();
    void SendControlInstruction(unsigned char brightness, unsigned char speed, unsigned char direction, unsigned char animation_type, unsigned char slider_no);
    void SendStaticColorPayload(unsigned char red, unsigned char green, unsigned char blue);
    void SendStaticColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char brightness = 0x3F);
    void SendTerminate();

    void SendChannelSetup(unsigned char channel_id);
    void SendKeyboardColors(const std::vector<RGBColor>& colors);

private:
    hid_device*    dev;
    std::string    location;
    std::string    device_name;
    unsigned short product_id;

    
  void SendDataPayload(const unsigned char* channel_bytes, size_t length);
};
/*---------------------------------------------------------*\
| AkkoMU02Controller.h                                      |
|                                                           |
|   Driver for Akko MU02 VIA keyboards                      |
|                                                           |
|   OpenRGB contributors                        11 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include <string>

struct AkkoMU02State
{
    unsigned char brightness;
    unsigned char effect;
    unsigned char speed;
    unsigned char hue;
    unsigned char saturation;
};

struct AkkoMU02KeyColor
{
    unsigned char hue;
    unsigned char saturation;
};

class AkkoMU02Controller
{
public:
    AkkoMU02Controller(hid_device* device, const char* path);
    ~AkkoMU02Controller();

    bool GetSupported() const;
    const std::string& GetLocation() const;
    const AkkoMU02State& GetState() const;
    const AkkoMU02KeyColor* GetKeyColors() const;
    bool SetKeyColor(unsigned char row, unsigned char col, unsigned char hue, unsigned char saturation);
    bool SaveKeyColors();
    bool Apply(unsigned char effect, const unsigned char* speed, const unsigned char* color,
               const unsigned char* brightness = nullptr);

private:
    bool Exchange(unsigned char command, const unsigned char* args, unsigned int size,
                  unsigned char* response);
    bool ReadState();
    bool ReadKeyColor(unsigned char row, unsigned char col, AkkoMU02KeyColor& color);
    bool SetValue(unsigned char value, const unsigned char* data, unsigned int size);

    hid_device* dev;
    std::string location;
    AkkoMU02State state{};
    AkkoMU02KeyColor key_colors[90]{};
    bool supported;
};

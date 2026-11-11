/*---------------------------------------------------------*\
| AkkoMU02Controller.cpp                                    |
|                                                           |
|   Driver for Akko MU02 VIA keyboards                      |
|                                                           |
|   OpenRGB contributors                        11 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "AkkoMU02Controller.h"
#include "QMKViaCommands.h"

AkkoMU02Controller::AkkoMU02Controller(hid_device* device, const char* path)
    : dev(device), location(std::string("HID: ") + path), supported(false)
{
    unsigned char reply[32];
    supported = Exchange(QMK_VIA_CMD_GET_PROTOCOL_VERSION, nullptr, 0, reply)
             && reply[1] == 0 && reply[2] == 12 && ReadState();
    for(unsigned int index = 0; supported && index < sizeof(key_colors) / sizeof(key_colors[0]); index++)
    {
        supported = ReadKeyColor(index / 15, index % 15, key_colors[index]);
    }
}

AkkoMU02Controller::~AkkoMU02Controller()
{
    hid_close(dev);
}

bool AkkoMU02Controller::GetSupported() const
{
    return supported;
}

const std::string& AkkoMU02Controller::GetLocation() const
{
    return location;
}

const AkkoMU02State& AkkoMU02Controller::GetState() const
{
    return state;
}

const AkkoMU02KeyColor* AkkoMU02Controller::GetKeyColors() const
{
    return key_colors;
}

bool AkkoMU02Controller::ReadKeyColor(unsigned char row, unsigned char col, AkkoMU02KeyColor& color)
{
    unsigned char args[] = {0, 3, 0, row, col};
    unsigned char reply[32];
    if(!Exchange(QMK_VIA_CMD_CUSTOM_GET_VALUE, args, sizeof(args), reply))
    {
        return false;
    }
    color = {reply[7], reply[6]};
    return true;
}

bool AkkoMU02Controller::SetKeyColor(unsigned char row, unsigned char col,
                                    unsigned char hue, unsigned char saturation)
{
    if(!supported || row >= 6 || col >= 15)
    {
        return false;
    }
    unsigned char args[] = {0, 3, 0, row, col, saturation, hue};
    AkkoMU02KeyColor actual;
    if(!Exchange(QMK_VIA_CMD_CUSTOM_SET_VALUE, args, sizeof(args), nullptr)
    || !ReadKeyColor(row, col, actual) || actual.hue != hue || actual.saturation != saturation)
    {
        return false;
    }
    key_colors[row * 15 + col] = actual;
    return true;
}

bool AkkoMU02Controller::SaveKeyColors()
{
    const unsigned char args[] = {0, 2, 0};
    return supported && state.effect == 45
        && Exchange(QMK_VIA_CMD_CUSTOM_SET_VALUE, args, sizeof(args), nullptr);
}

bool AkkoMU02Controller::Exchange(unsigned char command, const unsigned char* args,
                                  unsigned int size, unsigned char* response)
{
    unsigned char packet[33] = {};
    if(size > 30)
    {
        return false;
    }
    packet[1] = command;
    if(size)
    {
        std::memcpy(packet + 2, args, size);
    }
    if(hid_write(dev, packet, sizeof(packet)) != static_cast<int>(sizeof(packet)))
    {
        return false;
    }

    unsigned char reply[32];
    if(hid_read_timeout(dev, reply, sizeof(reply), 1000) != static_cast<int>(sizeof(reply))
    || reply[0] != command || (size && std::memcmp(reply + 1, args, size)))
    {
        return false;
    }
    if(response)
    {
        std::memcpy(response, reply, sizeof(reply));
    }
    return true;
}

bool AkkoMU02Controller::ReadState()
{
    unsigned char values[5];
    for(unsigned char id = QMK_VIA_RGB_MATRIX_BRIGHTNESS; id <= QMK_VIA_RGB_MATRIX_COLOR; id++)
    {
        unsigned char args[] = {QMK_VIA_RGB_MATRIX_CHANNEL, id};
        unsigned char reply[32];
        if(!Exchange(QMK_VIA_CMD_CUSTOM_GET_VALUE, args, sizeof(args), reply))
        {
            return false;
        }
        values[id - 1] = reply[3];
        if(id == QMK_VIA_RGB_MATRIX_COLOR)
        {
            values[4] = reply[4];
        }
    }
    if(values[1] > 45)
    {
        return false;
    }
    state = {values[0], values[1], values[2], values[3], values[4]};
    return true;
}

bool AkkoMU02Controller::SetValue(unsigned char value, const unsigned char* data, unsigned int size)
{
    if(value < QMK_VIA_RGB_MATRIX_BRIGHTNESS || value > QMK_VIA_RGB_MATRIX_COLOR
    || size != (value == QMK_VIA_RGB_MATRIX_COLOR ? 2u : 1u))
    {
        return false;
    }
    unsigned char args[4] = {QMK_VIA_RGB_MATRIX_CHANNEL, value, data[0], 0};
    if(size == 2)
    {
        args[3] = data[1];
    }
    return Exchange(QMK_VIA_CMD_CUSTOM_SET_VALUE, args, size + 2, nullptr);
}

bool AkkoMU02Controller::Apply(unsigned char effect, const unsigned char* speed, const unsigned char* color,
                              const unsigned char* brightness)
{
    if(!supported || effect > 45 || !ReadState())
    {
        return false;
    }
    const unsigned char expected_speed      = speed ? *speed : state.speed;
    const unsigned char expected_hue        = color ? color[0] : state.hue;
    const unsigned char expected_sat        = color ? color[1] : state.saturation;
    const unsigned char original_brightness = state.brightness;
    const bool set_brightness               = brightness && *brightness != original_brightness;

    if((effect != state.effect && !SetValue(QMK_VIA_RGB_MATRIX_EFFECT, &effect, 1))
    || (expected_speed != state.speed && !SetValue(QMK_VIA_RGB_MATRIX_EFFECT_SPEED, &expected_speed, 1))
    || ((expected_hue != state.hue || expected_sat != state.saturation)
        && !SetValue(QMK_VIA_RGB_MATRIX_COLOR, color, 2))
    || (set_brightness && !SetValue(QMK_VIA_RGB_MATRIX_BRIGHTNESS, brightness, 1)))
    {
        return false;
    }
    // Brightness SET is quantized by firmware (255 reads as 253); use the actual readback.
    return ReadState() && state.effect == effect && state.speed == expected_speed
        && state.hue == expected_hue && state.saturation == expected_sat
        && (set_brightness || state.brightness == original_brightness);
}

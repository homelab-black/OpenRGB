/*---------------------------------------------------------*\
| AulaF2067Controller.cpp                                   |
|                                                           |
|   Driver for Aula Aegis 2067usb (F2067) keyboard          |
|                                                           |
|   Antipov Kostiantyn (dev@diablosat.cc)      21.05.2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AulaF2067Controller.h"

AulaF2067Controller::AulaF2067Controller(hid_device* dev_handle, const char* dt_path)
{
    dev = dev_handle;
    location = dt_path;
}

AulaF2067Controller::~AulaF2067Controller()
{
    hid_close(dev);
}

std::string AulaF2067Controller::GetDeviceLocation()
{
    return location;
}

void AulaF2067Controller::setSpeed(unsigned char new_speed)
{
    speed = new_speed;
}

void AulaF2067Controller::setAnimationEffect(unsigned char new_animation_effect)
{
    animation_effect = new_animation_effect;
}

void AulaF2067Controller::setBrightness(unsigned char new_brightness)
{
    brightness = new_brightness;
}

void AulaF2067Controller::SendPacket()
{
    unsigned char report[] =
    {
        0x00,
        0x07,
        0xFF,
        0xFF,
        animation_effect,
        brightness,
        speed,
        0x00,
        0x00,
    };

    hid_write(dev, report, sizeof(report));
}
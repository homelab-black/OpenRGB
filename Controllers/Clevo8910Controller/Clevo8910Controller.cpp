
/*---------------------------------------------------------*\
|   Clevo8910Controller.cpp                                 |
|                                                           |
|   Driver for Clevo Color per key keyboard                 |
|                                                           |
|   EzyUser     9 Jan 2026                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string.h>
#include "Clevo8910Controller.h"

Clevo8910Controller::Clevo8910Controller(hid_device* dev_handle, const hid_device_info& info, const unsigned short pid, std::string dev_name)
{
    dev                 = dev_handle;
    location            = info.path;
    usb_pid             = pid;
    name                = dev_name;
}

Clevo8910Controller::~Clevo8910Controller()
{
    hid_close(dev);
}

std::string Clevo8910Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string Clevo8910Controller::GetDeviceName()
{
    return(name);
}

uint16_t Clevo8910Controller::GetUSBPID()
{
    return(usb_pid);
}

void Clevo8910Controller::TurnOff()
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    memset(usb_buf, 0x00, CLEVO8910_REPORT_SIZE);
    usb_buf[0]  = CLEVO8910_CLASS_BYTE;
    usb_buf[2]  = CLEVO8910_OFF_CMD;
    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

void Clevo8910Controller::SaveBios(bool save)
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    memset(usb_buf+3, 0x00, CLEVO8910_REPORT_SIZE-3);
    usb_buf[0]  = CLEVO8910_CLASS_BYTE;
    usb_buf[1]  = CLEVO8910_BIOS_CMD;
    usb_buf[2]  = (save ? (uint8_t)1 : (uint8_t) 0);
    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

void Clevo8910Controller::SetParams(uint8_t brightness, uint8_t speed)
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];

    memset(usb_buf, 0x00, CLEVO8910_REPORT_SIZE);
    usb_buf[0]  = CLEVO8910_CLASS_BYTE;
    usb_buf[1]  = CLEVO8910_BRT_SPD_CMD;
    usb_buf[2]  = brightness;
    usb_buf[3]  = speed;
    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

void Clevo8910Controller::SetSimpleMode(uint8_t effect, uint8_t bright, uint8_t speed)
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    SetParams(bright, speed);

    memset(usb_buf, 0x00, CLEVO8910_REPORT_SIZE);
    usb_buf[0]  = CLEVO8910_CLASS_BYTE;
    usb_buf[2]  = effect;
    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

void Clevo8910Controller::SetMode(uint8_t effect, uint8_t brightness, uint8_t speed, uint8_t key, RGBColor color )
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    SetParams(brightness, speed);

    memset(usb_buf, 0x00, CLEVO8910_REPORT_SIZE);
    usb_buf[0]  = CLEVO8910_CLASS_BYTE;
    usb_buf[1]  = effect;
    usb_buf[2]  = key;
    usb_buf[3]  = RGBGetRValue(color);
    usb_buf[4]  = RGBGetGValue(color);
    usb_buf[5]  = RGBGetBValue(color);

    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

void Clevo8910Controller::SetColors(uint8_t mode, std::vector<led> array_leds, bool rnd_flag, std::vector<RGBColor> array_colors)
{
    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    usb_buf[0]          = CLEVO8910_CLASS_BYTE;
    usb_buf[1]          = mode;
    usb_buf[6]          = 0;
    if (!rnd_flag)
    {
        for(size_t i = 0; i < array_leds.size(); i++)
        {
            usb_buf[2]  = (uint8_t)(array_leds[i].value);
            usb_buf[3]  = RGBGetRValue(array_colors[i]);
            usb_buf[4]  = RGBGetGValue(array_colors[i]);
            usb_buf[5]  = RGBGetBValue(array_colors[i]);
            hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
        }
    }
    else
    {
        usb_buf[2]      = 0xFF;
        usb_buf[4]      = 0;
        usb_buf[5]      = 0;
        for(size_t i = 0; i < array_leds.size(); i++)
        {
            usb_buf[3]  = (uint8_t)(array_leds[i].value);
            hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
        }
    }
}

void Clevo8910Controller::SetSingleLED(uint8_t effect, uint8_t key, RGBColor color)
{
    const RGBColor rndcolor     = ToRGBColor(0, 0, 0);

    uint8_t usb_buf[CLEVO8910_REPORT_SIZE];
    usb_buf[0]          = CLEVO8910_CLASS_BYTE;
    usb_buf[1]          = effect;

    if (color == rndcolor && effect != (uint8_t )(CLEVO8910_MODE_DIRECT & 0xFF))
    {
        usb_buf[2]      = 0xFF; //This is used for random color mode with the key in the next byte
        usb_buf[3]      = key;
        usb_buf[4]      = 0;
        usb_buf[5]      = 0;
    }
    else
    {
        usb_buf[2]      = key;
        usb_buf[3]      = RGBGetRValue(color);
        usb_buf[4]      = RGBGetGValue(color);
        usb_buf[5]      = RGBGetBValue(color);
    }
    hid_send_feature_report(dev, usb_buf, CLEVO8910_REPORT_SIZE);
}

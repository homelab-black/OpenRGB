/*---------------------------------------------------------*\
| CreativePebbleXController.cpp                             |
|                                                           |
|   Driver for Creative Pebble X                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "CreativePebbleXController.h"

CreativePebbleXController::CreativePebbleXController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
}

CreativePebbleXController::~CreativePebbleXController()
{
    hid_close(dev);
}

std::string CreativePebbleXController::GetDeviceLocation()
{
    return("HID " + location);
}


void CreativePebbleXController::UpdateDeviceMode(unsigned char mode)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x03;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x29;
    usb_buf[0x06]           = 0x01;
    usb_buf[0x07]           = mode;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::UpdateModeSpecificLEDs(unsigned char red, unsigned char green, unsigned char blue)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x08;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x2B;
    usb_buf[0x06]           = 0x01;
    usb_buf[0x07]           = 0x01;
    usb_buf[0x08]           = 0x01;
    usb_buf[0x09]           = 0xFF;

    usb_buf[0x10]           = blue;
    usb_buf[0x11]           = green;
    usb_buf[0x12]           = red;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::UpdateMorphMode(unsigned char red, unsigned char green, unsigned char blue, unsigned char red2, unsigned char green2, unsigned char blue2)
{
    unsigned char usb_buf[64];
    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x08;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x2B;
    usb_buf[0x06]           = 0x01;
    usb_buf[0x07]           = 0x01;
    usb_buf[0x08]           = 0x01;
    usb_buf[0x09]           = 0xFF;

    usb_buf[0x10]           = blue;
    usb_buf[0x11]           = green;
    usb_buf[0x12]           = red;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x08;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x2B;
    usb_buf[0x06]           = 0x01;
    usb_buf[0x07]           = 0x02;
    usb_buf[0x08]           = 0x01;
    usb_buf[0x09]           = 0xFF;

    usb_buf[0x10]           = blue2;
    usb_buf[0x11]           = green2;
    usb_buf[0x12]           = red2;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::UpdateDeviceLEDs(std::vector<RGBColor> led_colors)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    //03 6A 3A 18 00 2B 01 01 01 FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR
    usb_buf[0x00] = 0x03;
    usb_buf[0x01] = 0x6A;
    usb_buf[0x02] = 0x3A;
    usb_buf[0x03] = 0x18;
    usb_buf[0x04] = 0x00;
    usb_buf[0x05] = 0x2B;
    usb_buf[0x06] = 0x01;
    usb_buf[0x07] = 0x01;
    usb_buf[0x08] = 0x00;

    int offset = 9;
    for (int i = 0; i < 5; i++) {
        usb_buf[offset++] = 0xFF;
        usb_buf[offset++] = RGBGetBValue(led_colors[i]);     // Blue
        usb_buf[offset++] = RGBGetGValue(led_colors[i]);     // Green
        usb_buf[offset++] = RGBGetRValue(led_colors[i]);     // Red
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::SetBrightness (unsigned char brightness)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Port State packet                              |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x03;
    usb_buf[0x01]   = 0x6A;
    usb_buf[0x02]   = 0x3A;
    usb_buf[0x03]   = 0x02;
    usb_buf[0x04]   = 0x00;
    usb_buf[0x05]   = 0x27;
    usb_buf[0x06]   = brightness;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::SetSpeed (unsigned char speed)
{
    unsigned char usb_buf[64];
    speed *= 500;

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Port State packet                              |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x03;
    usb_buf[0x01]   = 0x6A;
    usb_buf[0x02]   = 0x3A;
    usb_buf[0x03]   = 0x05;
    usb_buf[0x04]   = 0x00;
    usb_buf[0x05]   = 0x2b;
    usb_buf[0x06]   = 0x01;
    usb_buf[0x07]   = 0x03;
    usb_buf[0x08]   = speed & 0xFF;
    usb_buf[0x09]   = speed >> 8;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::SetDirection (unsigned char direction)
{
    unsigned char usb_buf[64];
    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Port State packet                              |
    \*-----------------------------------------------------*/
    usb_buf[0x00]   = 0x03;
    usb_buf[0x01]   = 0x6A;
    usb_buf[0x02]   = 0x3A;
    usb_buf[0x03]   = 0x05;
    usb_buf[0x04]   = 0x00;
    usb_buf[0x05]   = 0x2B;
    usb_buf[0x06]   = 0x01;
    usb_buf[0x07]   = 0x04;
    usb_buf[0x08]   = direction;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

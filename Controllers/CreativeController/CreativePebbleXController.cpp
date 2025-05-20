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

void CreativePebbleXController::UpdateDeviceLEDs(std::vector<RGBColor> led_colors, unsigned char data_packet_length, unsigned char mode_specific_led_num)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    //03 6A 3A PACK_LEN 00 2B 01 01 01 FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR
    usb_buf[0x00] = 0x03;
    usb_buf[0x01] = 0x6A;
    usb_buf[0x02] = 0x3A;
    usb_buf[0x03] = data_packet_length;
    usb_buf[0x04] = 0x00;
    usb_buf[0x05] = 0x2B;
    usb_buf[0x06] = 0x01;
    usb_buf[0x07] = mode_specific_led_num;
    usb_buf[0x08] = 0x01;

    if(data_packet_length == 0x08)
    {
        /*--------------------------------------------------------------*\
        | Device is expecting data for a single mode specific color      |
        \*--------------------------------------------------------------*/
        usb_buf[0x09] = 0xFF;
        usb_buf[0x0A] = RGBGetBValue(led_colors[mode_specific_led_num-1]);     // Blue
        usb_buf[0x0B] = RGBGetGValue(led_colors[mode_specific_led_num-1]);     // Green
        usb_buf[0x0C] = RGBGetRValue(led_colors[mode_specific_led_num-1]);     // Red
    }
    else
    {
        /*-----------------------------------------------------*\
        | Device is expecting color data for all LEDs           |
        \*-----------------------------------------------------*/
        int offset = 9;
        for(int i = 0; i < led_colors.size(); i++)
        {
            usb_buf[offset++] = 0xFF;
            usb_buf[offset++] = RGBGetBValue(led_colors[i]);     // Blue
            usb_buf[offset++] = RGBGetGValue(led_colors[i]);     // Green
            usb_buf[offset++] = RGBGetRValue(led_colors[i]);     // Red
        }
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::SetBrightness(unsigned char brightness)
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

void CreativePebbleXController::SetSpeed(unsigned char speed)
{
    unsigned char usb_buf[64];
    unsigned char speed_val1;
    unsigned char speed_val2;

    switch (speed)
    {
        case 0x00:
            // Handle speed 0x00
            speed_val1 = 0x70;
            speed_val2 = 0x17;
            break;
        case 0x01:
            // Handle speed 0x01
            speed_val1 = 0xa0;
            speed_val2 = 0x0f;
            break;
        case 0x02:
            // Handle speed 0x02
            speed_val1 = 0xc4;
            speed_val2 = 0x09;
            break;
        case 0x03:
            // Handle speed 0x03
            speed_val1 = 0x35;
            speed_val2 = 0x05;
            break;
        case 0x04:
            // Handle speed 0x04
            speed_val1 = 0xee;
            speed_val2 = 0x02;
            break;
        case 0x05:
            // Handle speed 0x05
            speed_val1 = 0x77;
            speed_val2 = 0x01;
            break;
        case 0x06:
            // Handle speed 0x06
            speed_val1 = 0xfa;
            speed_val2 = 0x00;
            break;
        default:
            // Handle invalid speed values
            speed_val1 = 0x70;
            speed_val2 = 0x17;
            break;
    }

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
    usb_buf[0x08]   = speed_val1;
    usb_buf[0x09]   = speed_val2;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void CreativePebbleXController::SetDirection(unsigned char direction)
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

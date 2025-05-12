/*---------------------------------------------------------*\
| CreativePebbleXController.cpp                             |
|                                                           |
|   Driver for Creative Pebble X                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "CreativePebbleXController.h"
#include "LogManager.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;


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
    hid_write(dev, (unsigned char *)usb_buf, 64);

    // hid_close(dev);
    // dev = hid_open(0x041E, 0x3299, NULL);
}

void CreativePebbleXController::UpdateDeviceLEDs (const std::vector<RGBColor>& led_colors, unsigned char brightness)
{


    unsigned char usb_buf[64];
    // unsigned char usb_buf2[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x02;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x2A;
    usb_buf[0x06]           = 0x01;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, (unsigned char *)usb_buf, 64);

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    //5A 3A 06 04 00 03 01 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    usb_buf[0x00]           = 0x03;
    usb_buf[0x01]           = 0x6A;
    usb_buf[0x02]           = 0x3A;
    usb_buf[0x03]           = 0x01;
    usb_buf[0x04]           = 0x00;
    usb_buf[0x05]           = 0x21;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, (unsigned char *)usb_buf, 64);

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
    usb_buf[0x06] = 0x04;
    usb_buf[0x07] = 0x01;
    usb_buf[0x08] = 0x01;

    int offset = 9;
    for (int i = 0; i < 5; i++) {
        usb_buf[offset++] = 0xFF;     // Possibly a control/enable byte
        usb_buf[offset++] = RGBGetBValue(led_colors[i]);     // Blue
        usb_buf[offset++] = RGBGetGValue(led_colors[i]);     // Green
        usb_buf[offset++] = RGBGetRValue(led_colors[i]);     // Red
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, (unsigned char *)usb_buf, 64);


    // this_thread::sleep_for(chrono::milliseconds(200));


    // memset(usb_buf2, 0x00, sizeof(usb_buf2));

    // //03 6A 3A 18 00 2B 01 01 01 FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR

    // usb_buf2[0x00] = 0x03;
    // usb_buf2[0x01] = 0x6A;
    // usb_buf2[0x02] = 0x3A;
    // usb_buf2[0x03] = 0x18;
    // usb_buf2[0x04] = 0x00;
    // usb_buf2[0x05] = 0x2B;
    // usb_buf2[0x06] = 0x04;
    // usb_buf2[0x07] = 0x01;
    // usb_buf2[0x08] = 0x01;

    // int offset_new = 9;
    // for (int i = 0; i < 5; i++) {
    //     usb_buf2[offset_new++] = 0xFF;     // Possibly a control/enable byte
    //     usb_buf2[offset_new++] = RGBGetBValue(led_colors[i]);     // Blue
    //     usb_buf2[offset_new++] = RGBGetGValue(led_colors[i]);     // Green
    //     usb_buf2[offset_new++] = RGBGetRValue(led_colors[i]);     // Red
    // }

    // /*-----------------------------------------------------*\
    // | Send packet                                           |
    // \*-----------------------------------------------------*/
    // hid_write(dev, (unsigned char *)usb_buf2, 64);
}

void CreativePebbleXController::SetBrightness(unsigned char brightness)
{
    SendBrightness(brightness);
}


void CreativePebbleXController::SendBrightness(unsigned char brightness)
{
    unsigned char usb_buf[64];
    // unsigned char usb_buf2[64];
    /*-----------------------------------------------------*\
    | Brightness goes from 0-100                            |
    \*-----------------------------------------------------*/
    // if(brightness > 0xFF)
    // {
    //     brightness = 0xFF;
    // }

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
    hid_write(dev, (unsigned char *)usb_buf, 64);


    // this_thread::sleep_for(chrono::milliseconds(200));
    // /*-----------------------------------------------------*\
    // | Zero out buffer                                       |
    // \*-----------------------------------------------------*/
    // memset(usb_buf2, 0x00, sizeof(usb_buf2));

    // /*-----------------------------------------------------*\
    // | Set up Port State packet                              |
    // \*-----------------------------------------------------*/
    // usb_buf2[0x00]   = 0x03;
    // usb_buf2[0x01]   = 0x6A;
    // usb_buf2[0x02]   = 0x3A;
    // usb_buf2[0x03]   = 0x02;
    // usb_buf2[0x04]   = 0x00;
    // usb_buf2[0x05]   = 0x2d;
    // usb_buf2[0x06]   = 0x01;

    // /*-----------------------------------------------------*\
    // | Send packet                                           |
    // \*-----------------------------------------------------*/
    // hid_write(dev, (unsigned char *)usb_buf2, 64);

    //     this_thread::sleep_for(chrono::milliseconds(200));
    //     /*-----------------------------------------------------*\
    //     | Zero out buffer                                       |
    //     \*-----------------------------------------------------*/
    //     memset(usb_buf2, 0x00, sizeof(usb_buf2));

    //     /*-----------------------------------------------------*\
    //     | Set up Port State packet                              |
    //     \*-----------------------------------------------------*/
    //     usb_buf2[0x00]   = 0x03;
    //     usb_buf2[0x01]   = 0x6A;
    //     usb_buf2[0x02]   = 0x3A;
    //     usb_buf2[0x03]   = 0x02;
    //     usb_buf2[0x04]   = 0x00;
    //     usb_buf2[0x05]   = 0x27;
    //     usb_buf2[0x06]   = brightness;

    //     /*-----------------------------------------------------*\
    //     | Send packet                                           |
    //     \*-----------------------------------------------------*/
    //     hid_write(dev, (unsigned char *)usb_buf2, 64);
}

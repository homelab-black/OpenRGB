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

void CreativePebbleXController::UpdateDeviceLEDs (const std::vector<RGBColor>& led_colors)
{
    unsigned char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    //6a 3a 18 00 2b 01 01 01 FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR FF BB GG RR

    // Header
    usb_buf[0x01] = 0x6A;
    usb_buf[0x02] = 0x3A;
    usb_buf[0x03] = 0x18;
    usb_buf[0x04] = 0x00;
    usb_buf[0x05] = 0x2B;
    usb_buf[0x06] = 0x01;
    usb_buf[0x07] = 0x01;
    usb_buf[0x08] = 0x01;

    // Start writing LED data at index 9
    int offset = 9;
    for (int i = 0; i < 6; i++) {
        usb_buf[offset++] = 0xFF;     // Possibly a control/enable byte
        usb_buf[offset++] = RGBGetBValue(led_colors[i]);     // Blue
        usb_buf[offset++] = RGBGetGValue(led_colors[i]);     // Green
        usb_buf[offset++] = RGBGetRValue(led_colors[i]);     // Red
    }

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, (unsigned char *)usb_buf, 64);
}

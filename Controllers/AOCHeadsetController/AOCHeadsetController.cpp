/*---------------------------------------------------------*\
| AOCHeadsetController.cpp                                  |
|                                                           |
|   Driver for AOC headset                                  |
|                                                           |
|   Adam Honse (CalcProgrammer1)                12 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstring>
#include "AOCHeadsetController.h"
#include "StringUtils.h"

AOCHeadsetController::AOCHeadsetController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
}

AOCHeadsetController::~AOCHeadsetController()
{
    hid_close(dev);
}

std::string AOCHeadsetController::GetDeviceLocation()
{
    return("HID " + location);
}

std::string AOCHeadsetController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void AOCHeadsetController::SendDirect
    (
    RGBColor*       color_data
    )
{
    SendStartPacket();
    SendModePacket(0);
    SendColorPacket(color_data);
    SendApplyPacket();
}

void AOCHeadsetController::SendStartPacket()
{
    unsigned char usb_buf[16];

    memset(usb_buf, 0, sizeof(usb_buf));

    usb_buf[0] = 0xFF;
    usb_buf[1] = 0x02;

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

void AOCHeadsetController::SendModePacket(unsigned char mode)
{
    unsigned char usb_buf[16];

    memset(usb_buf, 0, sizeof(usb_buf));

    usb_buf[0] = 0xFF;
    usb_buf[1] = 0x03;
    usb_buf[2] = mode;

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

void AOCHeadsetController::SendColorPacket(RGBColor* color_data)
{
    unsigned char usb_buf[16];

    memset(usb_buf, 0, sizeof(usb_buf));

    usb_buf[0] = 0xFF;
    usb_buf[1] = 0x04;

    usb_buf[4] = RGBGetRValue(color_data[0]);
    usb_buf[5] = RGBGetGValue(color_data[0]);
    usb_buf[6] = RGBGetBValue(color_data[0]);

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

void AOCHeadsetController::SendApplyPacket()
{
    unsigned char usb_buf[16];

    memset(usb_buf, 0, sizeof(usb_buf));

    usb_buf[0] = 0xFF;
    usb_buf[1] = 0x01;

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

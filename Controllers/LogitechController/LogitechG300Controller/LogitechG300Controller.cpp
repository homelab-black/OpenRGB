/*---------------------------------------------------------*\
| LogitechG300Controller.cpp                                |
|                                                           |
|   Driver for Logitech G300s Optical Gaming Mouse          |
|                                                           |
|   Aman Ali                                  08 Sep 2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "LogitechG300Controller.h"
#include "StringUtils.h"

LogitechG300Controller::LogitechG300Controller(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev       = dev_handle;
    location  = path;
    name      = dev_name;
}

LogitechG300Controller::~LogitechG300Controller()
{
    if(dev != nullptr)
    {
        hid_close(dev);
    }
}

std::string LogitechG300Controller::GetNameString()
{
    return(name);
}

std::string LogitechG300Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*---------------------------------------------------------*\
| Each channel is a single bit, so an arbitrary color is     |
| snapped to the nearest corner of the RGB cube.             |
\*---------------------------------------------------------*/
unsigned char LogitechG300Controller::ColorToBits(RGBColor color)
{
    unsigned char bits = 0x00;

    if(RGBGetRValue(color) > 127)
    {
        bits |= LOGITECH_G300_LED_RED;
    }

    if(RGBGetGValue(color) > 127)
    {
        bits |= LOGITECH_G300_LED_GREEN;
    }

    if(RGBGetBValue(color) > 127)
    {
        bits |= LOGITECH_G300_LED_BLUE;
    }

    return(bits);
}

/*---------------------------------------------------------*\
| Writes the volatile LED register.  The onboard profiles    |
| are not touched, so this does not wear the profile memory  |
| and the color reverts on the next power cycle.             |
\*---------------------------------------------------------*/
void LogitechG300Controller::SetLED(RGBColor color)
{
    unsigned char usb_buf[LOGITECH_G300_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = LOGITECH_G300_REPORT_ID_LED;
    usb_buf[0x01] = ColorToBits(color);

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

/*---------------------------------------------------------*\
| Writes the color into the active onboard profile so that   |
| it survives a power cycle.  The profile record also holds  |
| the button mappings and DPI levels, so it is read back and |
| only the three color bits are replaced.                    |
\*---------------------------------------------------------*/
void LogitechG300Controller::Save(RGBColor color)
{
    unsigned char usb_buf[LOGITECH_G300_REPORT_SIZE];
    unsigned char profile_index;
    int           ret;

    /*-----------------------------------------------------*\
    | Byte 1 of report 0xF0 holds the active resolution in   |
    | bits 1 - 3 and the active profile index in bits 4 - 7. |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = LOGITECH_G300_REPORT_ID_ACTIVE;

    ret = hid_get_feature_report(dev, usb_buf, sizeof(usb_buf));

    if(ret < 0)
    {
        return;
    }

    profile_index = (usb_buf[0x01] >> 4) & 0x0F;

    if(profile_index > LOGITECH_G300_PROFILE_MAX)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Read the whole profile record back before patching it  |
    | so that the user's button and DPI configuration is     |
    | preserved.                                             |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = LOGITECH_G300_REPORT_ID_PROFILE_0 + profile_index;

    ret = hid_get_feature_report(dev, usb_buf, sizeof(usb_buf));

    if(ret < 0)
    {
        return;
    }

    usb_buf[0x01] = (unsigned char)((usb_buf[0x01] & LOGITECH_G300_LED_PRESERVE_MASK) | ColorToBits(color));

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

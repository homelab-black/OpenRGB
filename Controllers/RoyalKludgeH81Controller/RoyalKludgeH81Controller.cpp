/*---------------------------------------------------------*\
| RoyalKludgeH81Controller.cpp                              |
|                                                           |
|   Driver for Royal Kludge RK-H81 keyboard                 |
|                                                           |
|   eclecticBelletrist                          04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "LogManager.h"
#include "RoyalKludgeH81Controller.h"
#include "StringUtils.h"

/*---------------------------------------------------------------*\
| per-LED slot offsets into the report-ID-6 buffer (see            |
| PROTOCOL_DOSSIER.md for how these were mapped).  physical keys    |
| skip several slots, unclear why, see dossier                     |
\*---------------------------------------------------------------*/
static const unsigned char rkh81_led_offsets[RKH81_LED_COUNT] =
{
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
     10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
     20,  21,  22,  24,  25,  26,  27,  28,  30,  31,
     32,  33,  34,  35,  36,  37,  38,  39,  40,  42,
     43,  44,  45,  46,  48,  49,  50,  51,  52,  54,
     55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
     66,  67,  68,  69,  72,  73,  74,  77,  79,  80,
     81,  82,  83,  84,  88,  89,  91,  92,  93,  95,
};

/*---------------------------------------------------------------*\
| hardware "lights off" report on the same ID-6 channel as         |
| SetLEDsDirect, different header/payload, turns off per-key       |
| backlight on its own, no keepalive, status LEDs untouched        |
\*---------------------------------------------------------------*/
static const unsigned char rkh81_off_command[] =
{
    0x06, 0x04, 0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x00, 0x03, 0x03, 0x04, 0x00, 0x00, 0x04, 0x04,
    0x07, 0x00, 0x00, 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0xFF,
    0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47,
    0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47,
    0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x04, 0x47, 0x07, 0x47, 0x07, 0x47, 0x07, 0x44,
    0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x04, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x5A, 0xA5,
};

RoyalKludgeH81Controller::RoyalKludgeH81Controller(hid_device* dev_handle, char* path)
{
    dev      = dev_handle;
    location = path;
}

RoyalKludgeH81Controller::~RoyalKludgeH81Controller()
{
    if(dev)
    {
        hid_close(dev);
    }
}

std::string RoyalKludgeH81Controller::GetLocation()
{
    return("HID: " + location);
}

const std::string& RoyalKludgeH81Controller::GetPath()
{
    return(location);
}

std::string RoyalKludgeH81Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int     ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool RoyalKludgeH81Controller::SetLEDsDirect(std::vector<RGBColor> colors)
{
    unsigned char buf[RKH81_REPORT_SIZE];
    memset(buf, 0x00, RKH81_REPORT_SIZE);

    buf[0x00] = 0x06;
    buf[0x01] = 0x08;
    buf[0x04] = 0x01;
    buf[0x06] = 0x7A;
    buf[0x07] = 0x01;

    for(unsigned int led_idx = 0; (led_idx < colors.size()) && (led_idx < RKH81_LED_COUNT); led_idx++)
    {
        unsigned int  offset = 0x08 + (rkh81_led_offsets[led_idx] * 3);

        // channel order is G,R,B, not the R,G,B of the related Sinowealth 10c chipset
        buf[offset]     = RGBGetGValue(colors[led_idx]);
        buf[offset + 1] = RGBGetRValue(colors[led_idx]);
        buf[offset + 2] = RGBGetBValue(colors[led_idx]);
    }

    int result = hid_send_feature_report(dev, buf, RKH81_REPORT_SIZE);

    // no spacing between sends drops the USB connection for a few seconds, 1ms keeps well above 100 FPS
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if(result != RKH81_REPORT_SIZE)
    {
        LOG_WARNING("[Royal Kludge H81] SetLEDsDirect: hid_send_feature_report wrote %d of %d bytes", result, RKH81_REPORT_SIZE);
    }

    return(result == RKH81_REPORT_SIZE);
}

bool RoyalKludgeH81Controller::SetLightsOff()
{
    unsigned char buf[RKH81_REPORT_SIZE];
    memset(buf, 0x00, RKH81_REPORT_SIZE);
    memcpy(buf, rkh81_off_command, sizeof(rkh81_off_command));

    int result = hid_send_feature_report(dev, buf, RKH81_REPORT_SIZE);

    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    if(result != RKH81_REPORT_SIZE)
    {
        LOG_WARNING("[Royal Kludge H81] SetLightsOff: hid_send_feature_report wrote %d of %d bytes", result, RKH81_REPORT_SIZE);
    }

    return(result == RKH81_REPORT_SIZE);
}

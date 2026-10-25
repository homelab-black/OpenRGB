/*---------------------------------------------------------*\
| AsusMonitorController.cpp                                 |
|                                                           |
|   Driver for Asus monitors                                |
|                                                           |
|   Morgan Guimard (morg)                       19 oct 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string.h>
#include "AsusMonitorController.h"
#include "LogManager.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| ALB01 front white / CCT lamp (vendor EP0, 64-byte block)  |
\*---------------------------------------------------------*/
#define ASUS_ROG_AURA_MONITOR_LIGHT_BAR_PID            0x1AC8
#define ASUS_ALB01_FRONT_WHITE_PACKET_SIZE             64
#define ASUS_ALB01_FRONT_WHITE_GET_TYPE                0xC0
#define ASUS_ALB01_FRONT_WHITE_GET_REQUEST             0x81
#define ASUS_ALB01_FRONT_WHITE_SET_TYPE                0x40
#define ASUS_ALB01_FRONT_WHITE_SET_REQUEST             0x80
#define ASUS_ALB01_FRONT_WHITE_TIMEOUT_MS              300
#define ASUS_ALB01_FRONT_WHITE_CCT_MIN                 2700
#define ASUS_ALB01_FRONT_WHITE_CCT_MAX                 6500
#define ASUS_ALB01_FRONT_WHITE_BRIGHT_MAX              127
#define ASUS_ALB01_FRONT_WHITE_CCT_LO                  5
#define ASUS_ALB01_FRONT_WHITE_EN1                     20
#define ASUS_ALB01_FRONT_WHITE_BR1                     22
#define ASUS_ALB01_FRONT_WHITE_EN2                     36
#define ASUS_ALB01_FRONT_WHITE_BR2                     38

AsusMonitorController::AsusMonitorController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name)
{
    dev         = dev_handle;
    location    = info.path;
    name        = dev_name;
    pid         = info.product_id;
    usb         = nullptr;

    /*-----------------------------------------------------*\
    | ALB01 front white lamp is vendor EP0, not the Aura    |
    | HID report. Open the USB device for control xfers.    |
    | Do not claim interface 0.                             |
    \*-----------------------------------------------------*/
    if(pid == ASUS_ROG_AURA_MONITOR_LIGHT_BAR_PID)
    {
        libusb_init(NULL);

#ifdef _WIN32
        libusb_set_option(NULL, LIBUSB_OPTION_USE_USBDK);
#endif

        usb = libusb_open_device_with_vid_pid(NULL, info.vendor_id, pid);

        if(!usb)
        {
            LOG_WARNING("[AsusMonitorController] Could not open 0B05:1AC8 for front white lamp (need usbfs access)");
        }
    }
}

AsusMonitorController::~AsusMonitorController()
{
    if(usb)
    {
        libusb_close(usb);
    }

    hid_close(dev);
}

std::string AsusMonitorController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string AsusMonitorController::GetNameString()
{
    return(name);
}

std::string AsusMonitorController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

unsigned int AsusMonitorController::GetNumberOfLEDs()
{
    uint8_t usb_buf[ASUS_MONITOR_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xEC;
    usb_buf[0x01] = 0xB0;

    hid_write(dev, usb_buf, sizeof(usb_buf));

    memset(usb_buf, 0x00, sizeof(usb_buf));

    int bytes = hid_read(dev, usb_buf, sizeof(usb_buf));

    return bytes > 0 ? usb_buf[32] : 0;
}

void AsusMonitorController::SendInit()
{
    uint8_t usb_buf[ASUS_MONITOR_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xEC;
    usb_buf[0x01] = 0x35;
    usb_buf[0x05] = 0xFF;
    usb_buf[0x08] = 0x01;

    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void AsusMonitorController::SetDirect(std::vector<RGBColor> colors)
{
    uint8_t usb_buf[ASUS_MONITOR_REPORT_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00] = 0xEC;
    usb_buf[0x01] = 0x40;
    usb_buf[0x02] = 0x84;
    usb_buf[0x04] = (uint8_t)colors.size();

    for(size_t i = 0; i < colors.size(); i++)
    {
        usb_buf[0x05 + (3 * i)]     = RGBGetRValue(colors[i]);
        usb_buf[0x05 + (3 * i + 1)] = RGBGetGValue(colors[i]);
        usb_buf[0x05 + (3 * i + 2)] = RGBGetBValue(colors[i]);
    }

    hid_write(dev, usb_buf, sizeof(usb_buf));
}

bool AsusMonitorController::HasFrontWhite()
{
    return(usb != nullptr);
}

void AsusMonitorController::SetFrontWhite(RGBColor color)
{
    unsigned char usb_buf[ASUS_ALB01_FRONT_WHITE_PACKET_SIZE];

    if(!usb)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | GET 0xC0/0x81 must use wValue=0. Any other value      |
    | STALLs EP0. Do not claim interface 0.                 |
    \*-----------------------------------------------------*/
    int ret = libusb_control_transfer(usb,
                                      ASUS_ALB01_FRONT_WHITE_GET_TYPE,
                                      ASUS_ALB01_FRONT_WHITE_GET_REQUEST,
                                      0,
                                      0,
                                      usb_buf,
                                      ASUS_ALB01_FRONT_WHITE_PACKET_SIZE,
                                      ASUS_ALB01_FRONT_WHITE_TIMEOUT_MS);

    if(ret != ASUS_ALB01_FRONT_WHITE_PACKET_SIZE)
    {
        LOG_VERBOSE("[AsusMonitorController] Front white GET 0xC0/0x81 failed: %d", ret);
        return;
    }

    unsigned char red   = RGBGetRValue(color);
    unsigned char green = RGBGetGValue(color);
    unsigned char blue  = RGBGetBValue(color);
    unsigned char peak  = red;

    if(green > peak)
    {
        peak = green;
    }

    if(blue > peak)
    {
        peak = blue;
    }

    unsigned char br = (unsigned char)((peak * ASUS_ALB01_FRONT_WHITE_BRIGHT_MAX + 127) / 255);
    unsigned char en = (br == 0) ? 0 : 1;
    unsigned int  rb = (unsigned int)red + blue;
    unsigned int  cct;

    if(rb)
    {
        cct = ASUS_ALB01_FRONT_WHITE_CCT_MIN + ((unsigned int)blue * (ASUS_ALB01_FRONT_WHITE_CCT_MAX - ASUS_ALB01_FRONT_WHITE_CCT_MIN)) / rb;
    }
    else
    {
        cct = 4000;
    }

    if(cct < ASUS_ALB01_FRONT_WHITE_CCT_MIN)
    {
        cct = ASUS_ALB01_FRONT_WHITE_CCT_MIN;
    }

    if(cct > ASUS_ALB01_FRONT_WHITE_CCT_MAX)
    {
        cct = ASUS_ALB01_FRONT_WHITE_CCT_MAX;
    }

    usb_buf[ASUS_ALB01_FRONT_WHITE_CCT_LO]     = (unsigned char)(cct & 0xFF);
    usb_buf[ASUS_ALB01_FRONT_WHITE_CCT_LO + 1] = (unsigned char)((cct >> 8) & 0xFF);
    usb_buf[ASUS_ALB01_FRONT_WHITE_EN1]        = en;
    usb_buf[ASUS_ALB01_FRONT_WHITE_BR1]        = br;
    usb_buf[ASUS_ALB01_FRONT_WHITE_EN2]        = en;
    usb_buf[ASUS_ALB01_FRONT_WHITE_BR2]        = br;

    libusb_control_transfer(usb,
                            ASUS_ALB01_FRONT_WHITE_SET_TYPE,
                            ASUS_ALB01_FRONT_WHITE_SET_REQUEST,
                            0,
                            0,
                            usb_buf,
                            ASUS_ALB01_FRONT_WHITE_PACKET_SIZE,
                            ASUS_ALB01_FRONT_WHITE_TIMEOUT_MS);
}

/*---------------------------------------------------------*\
| AkkoKeyboardController.cpp                                 |
|                                                           |
|   Driver for Akko / RongYuan (ROYUAN) keyboards.          |
|                                                           |
|   Wire format (64 data bytes, report id 0 prepended ->    |
|   65 bytes on the wire):                                   |
|                                                           |
|     SET (Bit8 checksum at data[8]):                       |
|       [0]=CMD [1]=mode [2]=speed [3]=bri [4]=options      |
|       [5]=R [6]=G [7]=B [8]=checksum                      |
|                                                           |
|     GET (Bit7 checksum at data[7]):                       |
|       [0]=CMD [7]=checksum ; response echoes CMD at [0]   |
|                                                           |
|     checksum(N) = 255 - (sum(data[0..N-1]) & 255)         |
|                                                           |
|   Roz                                         20 Jun 2026 |
|                                                           |
|   Protocol by the echtzeit-solutions/monsgeek-akko-linux  |
|   project (docs/PROTOCOL.md) and the SonixQMK community.  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include "AkkoKeyboardController.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

/*-----------------------------------------------------------*\
| 64 data bytes; the wire buffer prepends report id 0.        |
\*-----------------------------------------------------------*/
#define AKKO_DATA_LEN       64
#define AKKO_WIRE_LEN       (AKKO_DATA_LEN + 1)

#define AKKO_GET_USB_VERSION    0x8F

static const unsigned char AKKO_SET_CMD[AKKO_ZONE_COUNT] = { 0x07, 0x08 };
static const unsigned char AKKO_GET_CMD[AKKO_ZONE_COUNT] = { 0x87, 0x88 };

/*-----------------------------------------------------------*\
| Fill data[at] with the checksum covering data[0..at-1].     |
| data points at the 64-byte payload (NOT the report id).     |
\*-----------------------------------------------------------*/
static void akko_checksum(unsigned char* data, int at)
{
    unsigned int sum = 0;

    for(int i = 0; i < at; i++)
    {
        sum += data[i];
    }

    data[at] = (unsigned char)((255 - (sum & 0xFF)) & 0xFF);
}

AkkoKeyboardController::AkkoKeyboardController(hid_device* dev_handle, const char* path, const unsigned short pid, const int interface, std::string dev_name)
{
    dev             = dev_handle;
    location        = path;
    name            = dev_name;
    usb_pid         = pid;
    interface_num   = interface;
}

AkkoKeyboardController::~AkkoKeyboardController()
{
    hid_close(dev);
}

std::string AkkoKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string AkkoKeyboardController::GetNameString()
{
    return(name);
}

std::string AkkoKeyboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

unsigned short AkkoKeyboardController::GetUSBPID()
{
    return(usb_pid);
}

int AkkoKeyboardController::GetInterfaceNum()
{
    return(interface_num);
}

void AkkoKeyboardController::SetLED
    (
    int             zone,
    unsigned char   mode,
    unsigned char   speed,
    unsigned char   brightness,
    unsigned char   options,
    unsigned char   red,
    unsigned char   green,
    unsigned char   blue
    )
{
    if(zone < 0 || zone >= AKKO_ZONE_COUNT)
    {
        return;
    }

    unsigned char wire[AKKO_WIRE_LEN];
    memset(wire, 0x00, sizeof(wire));

    /*-------------------------------------------------------*\
    | wire[0] is the report id (0); data starts at wire[1].   |
    \*-------------------------------------------------------*/
    unsigned char* data = &wire[1];
    data[0] = AKKO_SET_CMD[zone];
    data[1] = mode;
    data[2] = speed;
    data[3] = brightness;
    data[4] = options;
    data[5] = red;
    data[6] = green;
    data[7] = blue;
    akko_checksum(data, 8); /* Bit8: checksum at data[8] over data[0..7] */

    hid_send_feature_report(dev, wire, AKKO_WIRE_LEN);
}

bool AkkoKeyboardController::GetLED(int zone, AkkoLedParam& out)
{
    if(zone < 0 || zone >= AKKO_ZONE_COUNT)
    {
        return(false);
    }

    unsigned char wire[AKKO_WIRE_LEN];
    memset(wire, 0x00, sizeof(wire));
    unsigned char* data = &wire[1];
    data[0] = AKKO_GET_CMD[zone];
    akko_checksum(data, 7); /* Bit7: checksum at data[7] over data[0..6] */

    if(hid_send_feature_report(dev, wire, AKKO_WIRE_LEN) != (int)AKKO_WIRE_LEN)
    {
        return(false);
    }

    /*-------------------------------------------------------*\
    | Responses are buffered; poll a few times. The report id |
    | (0) may or may not be present, so locate the echoed CMD.|
    \*-------------------------------------------------------*/
    for(int attempt = 0; attempt < 4; attempt++)
    {
        std::this_thread::sleep_for(80ms);

        unsigned char resp[AKKO_WIRE_LEN];
        memset(resp, 0x00, sizeof(resp));
        resp[0] = 0x00;

        int got = hid_get_feature_report(dev, resp, AKKO_WIRE_LEN);
        if(got <= 0)
        {
            continue;
        }

        const unsigned char* body = resp;
        if(resp[0] == 0x00 && got > 1 && resp[1] == AKKO_GET_CMD[zone])
        {
            body = &resp[1]; /* strip leading report id */
        }
        else if(resp[0] != AKKO_GET_CMD[zone])
        {
            continue; /* not our echo yet */
        }

        out.mode       = body[1];
        out.speed      = body[2];
        out.brightness = body[3];
        out.options    = body[4];
        out.red        = body[5];
        out.green      = body[6];
        out.blue       = body[7];
        return(true);
    }

    return(false);
}

std::string AkkoKeyboardController::GetFirmwareVersion()
{
    unsigned char wire[AKKO_WIRE_LEN];
    memset(wire, 0x00, sizeof(wire));
    unsigned char* data = &wire[1];
    data[0] = AKKO_GET_USB_VERSION;
    akko_checksum(data, 7);

    if(hid_send_feature_report(dev, wire, AKKO_WIRE_LEN) != (int)AKKO_WIRE_LEN)
    {
        return("");
    }

    for(int attempt = 0; attempt < 4; attempt++)
    {
        std::this_thread::sleep_for(80ms);

        unsigned char resp[AKKO_WIRE_LEN];
        memset(resp, 0x00, sizeof(resp));
        resp[0] = 0x00;

        int got = hid_get_feature_report(dev, resp, AKKO_WIRE_LEN);
        if(got <= 0)
        {
            continue;
        }

        const unsigned char* body = resp;
        if(resp[0] == 0x00 && got > 1 && resp[1] == AKKO_GET_USB_VERSION)
        {
            body = &resp[1];
        }
        else if(resp[0] != AKKO_GET_USB_VERSION)
        {
            continue;
        }

        /*---------------------------------------------------*\
        | [0x8F, device_id[4], ?, ?, version[2]] -> bytes 7,8 |
        \*---------------------------------------------------*/
        char buf[16];
        snprintf(buf, sizeof(buf), "%u.%u", body[7], body[8]);
        return(std::string(buf));
    }

    return("");
}

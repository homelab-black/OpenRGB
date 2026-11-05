/*---------------------------------------------------------*\
| AkkoKeyboardController.h                                   |
|                                                           |
|   Driver for Akko / RongYuan (ROYUAN) keyboards using the |
|   open vendor HID protocol (USB VID 0x3151).              |
|                                                           |
|   Roz                                         20 Jun 2026 |
|                                                           |
|   Protocol by the echtzeit-solutions/monsgeek-akko-linux  |
|   project (docs/PROTOCOL.md) and the SonixQMK community.  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*-----------------------------------------------------------*\
| Akko / RongYuan vendor ID                                   |
\*-----------------------------------------------------------*/
#define AKKO_KB_VID                 0x3151

/*-----------------------------------------------------------*\
| Keyboard product IDs                                        |
\*-----------------------------------------------------------*/
#define AKKO_5075_PID               0x5002  /* Akko 5075 (label K0055), "Multi-modes Keyboard-B" */

/*-----------------------------------------------------------*\
| LED zones. The index doubles as the OpenRGB zone/LED index. |
|   main keys -> SET 0x07 / GET 0x87                          |
|   side bars -> SET 0x08 / GET 0x88                          |
\*-----------------------------------------------------------*/
enum
{
    AKKO_ZONE_MAIN  = 0,
    AKKO_ZONE_SIDE  = 1,
    AKKO_ZONE_COUNT
};

/*-----------------------------------------------------------*\
| options low nibble = color source                           |
\*-----------------------------------------------------------*/
#define AKKO_OPT_CUSTOM_RGB         0x08    /* uses the R/G/B bytes        */
#define AKKO_OPT_DAZZLE             0x07    /* rainbow cycle (ignores RGB) */

#define AKKO_MAX_SPEED              4
#define AKKO_MAX_BRIGHTNESS         4

/*-----------------------------------------------------------*\
| Decoded LED parameters as read back from the keyboard.      |
\*-----------------------------------------------------------*/
struct AkkoLedParam
{
    unsigned char   mode;
    unsigned char   speed;
    unsigned char   brightness;
    unsigned char   options;
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
};

class AkkoKeyboardController
{
public:
    AkkoKeyboardController(hid_device* dev_handle, const char* path, const unsigned short pid, const int interface, std::string dev_name);
    ~AkkoKeyboardController();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();
    std::string     GetFirmwareVersion();
    unsigned short  GetUSBPID();
    int             GetInterfaceNum();

    /*-------------------------------------------------------*\
    | Write one zone's effect/color (SET_LEDPARAM, Bit8       |
    | checksum). speed/brightness are device-native bytes;    |
    | the caller handles the per-zone speed inversion.        |
    \*-------------------------------------------------------*/
    void            SetLED
                        (
                        int             zone,
                        unsigned char   mode,
                        unsigned char   speed,
                        unsigned char   brightness,
                        unsigned char   options,
                        unsigned char   red,
                        unsigned char   green,
                        unsigned char   blue
                        );

    /*-------------------------------------------------------*\
    | Read one zone's current params (GET_LEDPARAM, Bit7).    |
    | Best-effort: returns false if the device never echoes.  |
    \*-------------------------------------------------------*/
    bool            GetLED(int zone, AkkoLedParam& out);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    unsigned short  usb_pid;
    int             interface_num;
};

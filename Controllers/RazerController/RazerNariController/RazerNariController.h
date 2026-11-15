/*---------------------------------------------------------*\
| RazerNariController.h                                     |
|                                                           |
|   Driver for Razer Nari devices                           |
|                                                           |
|   Greg Sandstrom (superstrom)                 22 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include "RGBController.h"
#include "DeviceGuardManager.h"

/*---------------------------------------------------------*\
| Struct packing macro for GCC and MSVC                     |
\*---------------------------------------------------------*/
#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#define RAZER_NARI_REPORT_ID 0xFF

enum
{
    RAZER_NARI_CMD_LIGHTING_SET_BRIGHTNESS = 0x71,
    RAZER_NARI_CMD_LIGHTING_SET_COLOR      = 0x72,
};

PACK(struct razer_nari_request_report
{
    unsigned char report_id;     // usb_buf[0]  ff
    unsigned char hdr1;          // usb_buf[1]  0a
    unsigned char hdr2;          // usb_buf[2]  00
    unsigned char hdr3;          // usb_buf[3]  ff
    unsigned char hdr4;          // usb_buf[4]  04
    unsigned char hdr5;          // usb_buf[5]  12
    unsigned char hdr6;          // usb_buf[6]  f1
    unsigned char hdr7;          // usb_buf[7]  03
    unsigned char command_id;    // usb_buf[8]  71
    unsigned char arguments[56]; // usb_buf[9...63]
});

PACK(struct razer_nari_response_report
{
    unsigned char report_id;
    unsigned char arguments[63];
});

class RazerNariController
{
public:
    RazerNariController(hid_device* dev_handle, const char* path, unsigned short pid, std::string dev_name);
    ~RazerNariController();

    unsigned int            GetDeviceIndex();
    device_type             GetDeviceType();
    std::string             GetDeviceLocation();
    std::string             GetFirmwareString();
    std::string             GetName();
    std::string             GetSerialString();
    unsigned char           GetMaxBrightness();

    void                    SetDirect(RGBColor* colors);
    void                    SetBrightness(unsigned char brightness);

private:
    hid_device*             dev;
    unsigned short          dev_pid;

    /*---------------------------------------------------------*\
    | Device-specific protocol settings                         |
    \*---------------------------------------------------------*/
    unsigned char           dev_transaction_id;
    unsigned char           dev_led_id;

    /*---------------------------------------------------------*\
    | Device information strings                                |
    \*---------------------------------------------------------*/
    std::string             firmware_version;
    std::string             location;
    std::string             name;

    /*---------------------------------------------------------*\
    | Index of device in Razer device list                      |
    \*---------------------------------------------------------*/
    unsigned int            device_index;

    /*---------------------------------------------------------*\
    | HID report index for request and response                 |
    \*---------------------------------------------------------*/
    unsigned char           report_index;
    unsigned char           response_index;

    /*---------------------------------------------------------*\
    | Matrix type                                               |
    \*---------------------------------------------------------*/
    unsigned char           matrix_type;

    /*---------------------------------------------------------*\
    | Private functions based on OpenRazer                      |
    \*---------------------------------------------------------*/
    std::string             razer_get_firmware();
    std::string             razer_get_serial();

    razer_nari_request_report razer_nari_create_report();

    int razer_usb_receive(razer_nari_response_report* report);
    int razer_usb_send(razer_nari_request_report* report);
};

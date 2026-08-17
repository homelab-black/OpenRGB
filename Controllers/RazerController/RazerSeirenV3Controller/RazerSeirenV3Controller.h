/*---------------------------------------------------------*\
| RazerSeirenV3Controller.h                                  |
|                                                           |
|   Driver for Razer Seiren V3 Chroma (64-byte report)      |
|                                                           |
|   Reverse-engineered 2026-08-17 on real hardware          |
|   (PID 0x056F, no Synapse installed)                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include "RGBController.h"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

/*---------------------------------------------------------*\
| Struct packing macro for GCC and MSVC                     |
\*---------------------------------------------------------*/
#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

/*---------------------------------------------------------*\
| 64-byte feature report. Unlike the classic 90-byte        |
| Razer report, the Seiren V3 Chroma uses a 64-byte report  |
| with report id 0x07 and transaction id 0x0A. The command  |
| set is the classic extended-matrix protocol (class 0x0F). |
|                                                           |
| Layout (verified on hardware):                            |
|   report_id 0x07, status, transaction_id 0x0A,           |
|   remaining_packets (BE), protocol_type, data_size,       |
|   command_class, command_id, arguments[53], crc, reserved  |
|                                                           |
| CRC = XOR of bytes 2..61 (payload bytes 1..60, includes  |
| the transaction id, excludes status and crc).            |
\*---------------------------------------------------------*/
PACK(struct razer_seiren_v3_report
{
    unsigned char               report_id;
    unsigned char               status;
    unsigned char               transaction_id;
    unsigned short              remaining_packets;
    unsigned char               protocol_type;
    unsigned char               data_size;
    unsigned char               command_class;
    unsigned char               command_id;
    unsigned char               arguments[53];
    unsigned char               crc;
    unsigned char               reserved;
});

class RazerSeirenV3Controller
{
public:
    RazerSeirenV3Controller(hid_device* dev_handle, const char* path, unsigned short pid, std::string dev_name);
    ~RazerSeirenV3Controller();

    unsigned int            GetDeviceIndex();
    device_type             GetDeviceType();
    std::string             GetDeviceLocation();
    std::string             GetFirmwareString();
    std::string             GetName();
    std::string             GetSerialString();
    unsigned char           GetMaxBrightness();

    void                    SetDirect(RGBColor* colors);
    void                    SetModeCustom();
    void                    SetModeOff();
    void                    SetModeStatic(unsigned char red, unsigned char grn, unsigned char blu);
    void                    SetModeBreathingOneColor(unsigned char red, unsigned char grn, unsigned char blu);
    void                    SetModeBreathingTwoColors(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2);
    void                    SetModeSpectrumCycle();
    void                    SetModeAudioReactive();

    void                    KeepaliveThread();
    void                    UpdateKeepaliveColors(RGBColor* colors);
    void                    StopKeepalive();

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
    | Keepalive thread state (Direct mode needs continuous      |
    | frame pushing — the Seiren does not latch frames)         |
    \*---------------------------------------------------------*/
    std::atomic<bool>       keepalive_thread_run;
    std::thread*            keepalive_thread;
    std::mutex              keepalive_mutex;
    std::vector<RGBColor>   keepalive_colors;
    bool                    keepalive_active;

    /*---------------------------------------------------------*\
    | Private functions                                         |
    \*---------------------------------------------------------*/
    unsigned char           razer_seiren_v3_calculate_crc(razer_seiren_v3_report* report);
    razer_seiren_v3_report  razer_seiren_v3_create_report(unsigned char command_class, unsigned char command_id, unsigned char data_size);
    razer_seiren_v3_report  razer_seiren_v3_create_response();

    void                    razer_set_device_mode(unsigned char mode);

    std::string             razer_get_firmware();
    std::string             razer_get_serial();

    int                     razer_usb_receive(razer_seiren_v3_report* report);
    int                     razer_usb_send(razer_seiren_v3_report* report);
};

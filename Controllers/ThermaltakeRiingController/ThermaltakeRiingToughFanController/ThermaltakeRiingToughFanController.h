/*---------------------------------------------------------*\
| ThermaltakeRiingToughFanController.h                      |
|                                                           |
|   Driver for Thermaltake ToughFan 14 RGB                  |
|                                                           |
|   Omar Jaber (Bewinxed)                       18 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define THERMALTAKE_TOUGHFAN_PACKET_SIZE        193
#define THERMALTAKE_TOUGHFAN_INTERRUPT_TIMEOUT  250
#define THERMALTAKE_TOUGHFAN_DETECT_TIMEOUT     500
#define THERMALTAKE_TOUGHFAN_KEEPALIVE          3
#define THERMALTAKE_TOUGHFAN_LEDS_PER_FAN       24
#define THERMALTAKE_TOUGHFAN_NUM_CHANNELS       5
#define HID_MAX_STR                             255

enum
{
    THERMALTAKE_TOUGHFAN_COMMAND_BYTE   = 1,
    THERMALTAKE_TOUGHFAN_FUNCTION_BYTE  = 2,
    THERMALTAKE_TOUGHFAN_ZONE_BYTE      = 3,
    THERMALTAKE_TOUGHFAN_MODE_BYTE      = 4,
    THERMALTAKE_TOUGHFAN_DATA_BYTE      = 5,
};

enum
{
    THERMALTAKE_TOUGHFAN_MODE_DIRECT    = 0x24
};

class ThermaltakeRiingToughFanController
{
public:
    ThermaltakeRiingToughFanController(hid_device* dev_handle, const char* path);
    ~ThermaltakeRiingToughFanController();

    std::string             GetDeviceName();
    std::string             GetDeviceLocation();
    std::string             GetSerial();
    std::string             GetFirmwareVersion();

    unsigned int            GetLEDsPerFan();
    bool                    IsChannelActive(unsigned int channel);

    void                    SetChannelLEDs(unsigned char channel, RGBColor* colors, unsigned int num_colors);
    void                    SetMode(unsigned char mode, unsigned char speed);

private:
    hid_device*             dev;
    std::mutex              dev_mutex;

    unsigned char           current_mode;
    unsigned char           current_speed;
    std::string             device_name;
    std::string             firmware_version;
    std::string             location;

    bool                    channel_active[THERMALTAKE_TOUGHFAN_NUM_CHANNELS];

    uint8_t                 tt_buffer[THERMALTAKE_TOUGHFAN_NUM_CHANNELS][THERMALTAKE_TOUGHFAN_PACKET_SIZE];
    std::thread*            keepalive_thread;
    std::atomic<bool>       keepalive_thread_run;
    std::chrono::time_point<std::chrono::steady_clock> last_commit_time;

    void                    SendBuffer();
    void                    KeepaliveThread();
    void                    SendInit();
    void                    DetectChannels();
    void                    DrainRead();
};

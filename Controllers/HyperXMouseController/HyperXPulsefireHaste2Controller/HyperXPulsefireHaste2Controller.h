/*---------------------------------------------------------*\
| HyperXPulsefireHaste2Controller.h                         |
|                                                           |
|   Driver for HyperX Pulsefire Haste 2 Wireless            |
|                                                           |
|   Plamen Marinov (PMarinov1994)               14 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| Every report is 64 bytes: report ID (= command), sub      |
| command, payload.  The dongle answers each command with   |
| an ACK report (0xFF) and the mouse answers some with a    |
| reply report whose ID is the command + 1                  |
\*---------------------------------------------------------*/
#define HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE            64
#define HYPERX_PULSEFIRE_HASTE_2_PACKET_INTERVAL_MS     10
#define HYPERX_PULSEFIRE_HASTE_2_REPLY_TIMEOUT_MS       200

/*---------------------------------------------------------*\
| A live color frame holds for about 1.5 to 2 seconds, so   |
| the last frame is re-sent on this interval in Direct mode |
\*---------------------------------------------------------*/
#define HYPERX_PULSEFIRE_HASTE_2_KEEPALIVE_INTERVAL_MS  500
#define HYPERX_PULSEFIRE_HASTE_2_KEEPALIVE_POLL_MS      50

/*---------------------------------------------------------*\
| While the mouse is asleep, the connection status is read  |
| on this interval in case the wake up event was missed     |
\*---------------------------------------------------------*/
#define HYPERX_PULSEFIRE_HASTE_2_LINK_POLL_INTERVAL_MS  2000

#define HYPERX_PULSEFIRE_HASTE_2_DEVICE_ID_OFFSET       2
#define HYPERX_PULSEFIRE_HASTE_2_DEVICE_ID_LENGTH       12

enum
{
    HYPERX_PULSEFIRE_HASTE_2_CMD_DEVICE_INFO            = 0x10,     /* Device information request           */
    HYPERX_PULSEFIRE_HASTE_2_CMD_BRIGHTNESS             = 0x40,     /* LED brightness                       */
    HYPERX_PULSEFIRE_HASTE_2_CMD_PRESET                 = 0x42,     /* Lighting preset mode                 */
    HYPERX_PULSEFIRE_HASTE_2_CMD_DIRECT                 = 0x44,     /* Live color and FrameLight upload     */
    HYPERX_PULSEFIRE_HASTE_2_CMD_CONNECTION             = 0x52,     /* Wireless connection status           */
    HYPERX_PULSEFIRE_HASTE_2_REPORT_EVENT               = 0xFB,     /* Unsolicited event report             */
};

enum
{
    HYPERX_PULSEFIRE_HASTE_2_SUB_DEVICE_INFO            = 0x01,     /* 10 01: device information            */
    HYPERX_PULSEFIRE_HASTE_2_SUB_BRIGHTNESS_SET         = 0x01,     /* 40 01: set brightness                */
    HYPERX_PULSEFIRE_HASTE_2_SUB_PRESET_SET             = 0x02,     /* 42 02: set preset mode               */
    HYPERX_PULSEFIRE_HASTE_2_SUB_DIRECT_COLOR           = 0x02,     /* 44 02: live color frame              */
    HYPERX_PULSEFIRE_HASTE_2_SUB_FRAME_HEADER           = 0x03,     /* 44 03: FrameLight header             */
    HYPERX_PULSEFIRE_HASTE_2_SUB_FRAME_DATA             = 0x04,     /* 44 04: FrameLight frame              */
    HYPERX_PULSEFIRE_HASTE_2_SUB_CONNECTION             = 0x01,     /* 52 01: connection status             */
    HYPERX_PULSEFIRE_HASTE_2_EVENT_LINK                 = 0x0A,     /* FB 0A: mouse linked / link lost      */
};

enum
{
    HYPERX_PULSEFIRE_HASTE_2_PROFILE_LIVE               = 0x00,     /* Live settings, lost on power cycle   */
    HYPERX_PULSEFIRE_HASTE_2_PROFILE_ONBOARD            = 0x01,     /* Onboard memory, loaded on power up   */
};

enum
{
    HYPERX_PULSEFIRE_HASTE_2_MODE_OFF                   = 0x00,
    HYPERX_PULSEFIRE_HASTE_2_MODE_STATIC                = 0x01,
    HYPERX_PULSEFIRE_HASTE_2_MODE_BREATHING             = 0x02,
    HYPERX_PULSEFIRE_HASTE_2_MODE_SPECTRUM_CYCLE        = 0x04,
    HYPERX_PULSEFIRE_HASTE_2_MODE_FRAMELIGHT            = 0x06,
    HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT                = 0xFF,     /* OpenRGB only, streamed live frames   */
};

enum
{
    HYPERX_PULSEFIRE_HASTE_2_COLOR_TYPE_SINGLE          = 0x01,
    HYPERX_PULSEFIRE_HASTE_2_COLOR_TYPE_SPECTRUM        = 0xFE,
};

/*---------------------------------------------------------*\
| The preset duration is one byte, lower is faster          |
\*---------------------------------------------------------*/
enum
{
    HYPERX_PULSEFIRE_HASTE_2_SPEED_SLOWEST              = 0xFF,
    HYPERX_PULSEFIRE_HASTE_2_SPEED_FASTEST              = 0x0A,
    HYPERX_PULSEFIRE_HASTE_2_SPEED_DEFAULT              = 0x32,
    HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MIN             = 0x00,
    HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX             = 0xFF,
};

class HyperXPulsefireHaste2Controller
{
public:
    HyperXPulsefireHaste2Controller(hid_device* dev_handle, const char* path, std::string dev_name);
    ~HyperXPulsefireHaste2Controller();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();

    void SetMode(unsigned char mode, unsigned char speed, RGBColor color, unsigned char brightness);
    void SetDirectColor(RGBColor color);

    void SaveMode(unsigned char mode, unsigned char speed, RGBColor color, unsigned char brightness);

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;
    std::string             serial;

    std::mutex              dev_mutex;
    std::thread             keepalive_thread;
    std::atomic<bool>       keepalive_running;

    /*-----------------------------------------------------*\
    | Last state requested by OpenRGB, re-applied when the  |
    | mouse wakes up.  Protected by dev_mutex               |
    \*-----------------------------------------------------*/
    bool                    mode_applied;
    bool                    link_up;
    bool                    reapply_pending;
    unsigned char           active_mode;
    unsigned char           active_speed;
    unsigned char           active_brightness;
    RGBColor                active_color;

    std::chrono::steady_clock::time_point   last_packet_time;
    std::chrono::steady_clock::time_point   last_direct_time;
    std::chrono::steady_clock::time_point   last_link_poll_time;

    void KeepaliveThreadFunction();

    void ApplyMode();
    void ReadDeviceInformation();
    void ReadConnectionStatus();

    void SendBrightness(unsigned char profile, unsigned char brightness);
    void SendPreset(unsigned char profile, unsigned char mode, unsigned char speed, RGBColor color);
    void SendDirectFrame(RGBColor color);
    void SendFrameLight(unsigned char profile, RGBColor color);
    bool SendPacket(unsigned char* buf, unsigned char* reply);

    void DrainInputReports();
    void HandleInputReport(unsigned char* buf);
};

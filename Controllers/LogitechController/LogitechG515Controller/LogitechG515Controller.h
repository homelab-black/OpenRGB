/*---------------------------------------------------------*\
| LogitechG515Controller.h                                  |
|                                                           |
|   Driver for Logitech G515 LIGHTSPEED TKL                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <cstdint>
#include <mutex>
#include <hidapi.h>

/* Captures show Output reports (SET_REPORT, ReportType=0x02) on interface 2. */
#define LOGITECH_G515_REPORT_ID_INIT          0x10
#define LOGITECH_G515_REPORT_ID_MAIN          0x11
#define LOGITECH_G515_REPORT_LEN_INIT         7
#define LOGITECH_G515_REPORT_LEN_MAIN         20

enum
{
    LOGITECH_G515_MODE_OFF            = 0x00,
    LOGITECH_G515_MODE_STATIC         = 0x01,
    LOGITECH_G515_MODE_BREATHING      = 0x02,
    LOGITECH_G515_MODE_CYCLE          = 0x03,
    LOGITECH_G515_MODE_WAVE           = 0x04, /* OpenRGB value. */
};

/* Speed in captures: 1000..20000 (BE16). */
enum
{
    LOGITECH_G515_SPEED_FASTEST_MS    = 1000,
    LOGITECH_G515_SPEED_NORMAL_MS     = 5000,
    LOGITECH_G515_SPEED_SLOWEST_MS    = 20000,
};

class LogitechG515Controller
{
public:
    LogitechG515Controller(hid_device* dev_handle, uint16_t pid, std::string dev_name);
    ~LogitechG515Controller();

    std::string GetNameString();
    std::string GetSerialString();

    void        SetOff();
    void        SetStaticColor(unsigned char red, unsigned char green, unsigned char blue);
    void        SetBreathing(unsigned short speed_ms, unsigned char brightness, unsigned char red, unsigned char green, unsigned char blue);
    void        SetSpectrumCycle(unsigned short speed_ms, unsigned char brightness);
    void        SetRainbowWave(unsigned short speed_ms, unsigned char brightness);

private:
    typedef struct
    {
        uint16_t        pid;
        const char*     name;
        uint8_t         device_index;
        uint8_t         init_feature_index;     /* report 0x10 */
        uint8_t         mode_feature_index;     /* report 0x11, command 0x1D */
        uint8_t         direct_feature_index;   /* report 0x11, commands 0x5D/0x7D */
        uint8_t         system_feature_index;   /* report 0x10, seen in software-control bootstrap */
    } g515_transport_profile;

    hid_device*                          dev_handle;
    std::string                          name;
    g515_transport_profile               profile;
    std::mutex                           io_mutex;

    bool            trace_enabled;
    bool            software_control_ready;

    bool            SendReport(const uint8_t* report, size_t length, const char* context);
    bool            SendInit(const char* context);
    bool            SendModeApplyPacket(const char* context);
    bool            SendShortCommand(uint8_t feature_index, uint8_t function, uint8_t p4, uint8_t p5, uint8_t p6, const char* context);
    bool            EnsureSoftwareControl();

    void            ApplyOff();
    void            ApplyStaticColor(unsigned char red, unsigned char green, unsigned char blue);
    void            ApplyBreathing(unsigned short speed_ms, unsigned char brightness, unsigned char red, unsigned char green, unsigned char blue);
    void            ApplySpectrumCycle(unsigned short speed_ms, unsigned char brightness);
    void            ApplyRainbowWave(unsigned short speed_ms, unsigned char brightness);

    static unsigned short ClampSpeedMs(unsigned short speed_ms);
    static unsigned char ClampBrightness(unsigned char brightness);
};

/*---------------------------------------------------------*\
| LogitechG515Controller.cpp                                |
|                                                           |
|   Driver for Logitech G515 LIGHTSPEED TKL                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdlib>
#include <iomanip>
#include <sstream>
#include "LogManager.h"
#include "LogitechG515Controller.h"
#include "StringUtils.h"

static std::string BytesToHex(const uint8_t* data, size_t len)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for(size_t i = 0; i < len; i++)
    {
        oss << std::setw(2) << (unsigned int)data[i];
    }
    return oss.str();
}

LogitechG515Controller::LogitechG515Controller(hid_device* dev_handle, uint16_t pid, std::string dev_name)
{
    this->dev_handle              = dev_handle;
    this->name                    = dev_name;
    this->trace_enabled           = false;
    this->software_control_ready  = false;

    const char* env = std::getenv("OPENRGB_LOGITECH_G515_TRACE");
    trace_enabled = (env != nullptr && env[0] != '\0' && env[0] != '0');

    /* Capture-validated profile mapping. */
    if(pid == 0xC547)
    {
        profile = { pid, "wireless", 0x01, 0x10, 0x09, 0x0A, 0x0F };
    }
    else
    {
        /* PIDs 0xC355 (USB) and 0xC358 (wired-only model, if present) behave as wired in captures. */
        profile = { pid, "wired", 0xFF, 0x0F, 0x08, 0x09, 0x0E };
    }

    LOG_DEBUG("[%s] Using %s profile (pid=0x%04X idx=0x%02X init=0x%02X mode=0x%02X direct=0x%02X sys=0x%02X)",
              "LogitechG515Controller",
              profile.name,
              profile.pid,
              profile.device_index,
              profile.init_feature_index,
              profile.mode_feature_index,
              profile.direct_feature_index,
              profile.system_feature_index);
}

LogitechG515Controller::~LogitechG515Controller()
{

}

std::string LogitechG515Controller::GetNameString()
{
    return(name);
}

std::string LogitechG515Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev_handle, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool LogitechG515Controller::SendReport(const uint8_t* report, size_t length, const char* context)
{
    if(trace_enabled)
    {
        LOG_DEBUG("[%s] %s OUT[%zu]=%s",
                  "LogitechG515Controller",
                  context,
                  length,
                  BytesToHex(report, length).c_str());
    }

    int write_result = hid_write(dev_handle, report, length);
    if(write_result < 0)
    {
        LOG_WARNING("[%s] %s hid_write failed: %ls",
                    "LogitechG515Controller",
                    context,
                    hid_error(dev_handle));
        return false;
    }

    if(trace_enabled)
    {
        LOG_DEBUG("[%s] %s hid_write=%d", "LogitechG515Controller", context, write_result);

        uint8_t response[64] = { 0 };
        int read_result = hid_read_timeout(dev_handle, response, sizeof(response), 2);
        if(read_result > 0)
        {
            LOG_DEBUG("[%s] %s IN[%d]=%s",
                      "LogitechG515Controller",
                      context,
                      read_result,
                      BytesToHex(response, (size_t)read_result).c_str());
        }
        else if(read_result < 0)
        {
            LOG_DEBUG("[%s] %s hid_read_timeout failed: %ls",
                      "LogitechG515Controller",
                      context,
                      hid_error(dev_handle));
        }
    }

    return true;
}

bool LogitechG515Controller::SendShortCommand(uint8_t feature_index, uint8_t function, uint8_t p4, uint8_t p5, uint8_t p6, const char* context)
{
    uint8_t buf[LOGITECH_G515_REPORT_LEN_INIT] = { 0 };
    buf[0] = LOGITECH_G515_REPORT_ID_INIT;
    buf[1] = profile.device_index;
    buf[2] = feature_index;
    buf[3] = function;
    buf[4] = p4;
    buf[5] = p5;
    buf[6] = p6;
    return SendReport(buf, sizeof(buf), context);
}

bool LogitechG515Controller::SendInit(const char* context)
{
    return SendShortCommand(profile.init_feature_index, 0x1D, 0x00, 0x00, 0x00, context);
}

bool LogitechG515Controller::SendModeApplyPacket(const char* context)
{
    /* Observed during brightness/speed sweeps on mode feature 0x08/0x09. */
    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x7D;
    buf[4]  = 0x01;
    buf[8]  = 0x3C;
    buf[9]  = 0x01;
    buf[10] = 0x0E;
    return SendReport(buf, sizeof(buf), context);
}

bool LogitechG515Controller::EnsureSoftwareControl()
{
    if(software_control_ready)
    {
        return true;
    }

    /* Capture-validated bootstrap sequence from *_report-descriptor.pcapng (tail section)
     * that precedes stable mode operation and avoids firmware fallback.
     */
    bool ok = true;

    ok &= SendShortCommand(profile.mode_feature_index, 0x8D, 0x00, 0x00, 0x00, "SWCTRL(8D)");

    {
        uint8_t p[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
        p[0] = LOGITECH_G515_REPORT_ID_MAIN;
        p[1] = profile.device_index;
        p[2] = profile.mode_feature_index;
        p[3] = 0x7D;
        ok &= SendReport(p, sizeof(p), "SWCTRL(7D-0)");
    }

    ok &= SendShortCommand(profile.mode_feature_index, 0x3D, 0x00, 0x20, 0x00, "SWCTRL(3D-20)");
    ok &= SendShortCommand(profile.mode_feature_index, 0x5D, 0x00, 0x00, 0x00, "SWCTRL(5D-0)");
    ok &= SendShortCommand(profile.mode_feature_index, 0x5D, 0x01, 0x03, 0x07, "SWCTRL(5D-010307)");

    {
        uint8_t p[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
        p[0]  = LOGITECH_G515_REPORT_ID_MAIN;
        p[1]  = profile.device_index;
        p[2]  = profile.mode_feature_index;
        p[3]  = 0x7D;
        p[4]  = 0x01;
        p[8]  = 0x3C;
        p[9]  = 0x01;
        p[10] = 0x2C;
        ok &= SendReport(p, sizeof(p), "SWCTRL(7D-012C)");
    }

    ok &= SendShortCommand(profile.system_feature_index, 0x6D, 0x05, 0x00, 0x00, "SWCTRL(SYS-6D-05)");

    {
        uint8_t p[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
        p[0]  = LOGITECH_G515_REPORT_ID_MAIN;
        p[1]  = profile.device_index;
        p[2]  = profile.mode_feature_index;
        p[3]  = 0x7D;
        p[4]  = 0x01;
        p[8]  = 0x3C;
        p[9]  = 0x01;
        p[10] = 0x0E;
        ok &= SendReport(p, sizeof(p), "SWCTRL(7D-010E)");
    }

    ok &= SendShortCommand(profile.mode_feature_index, 0x5D, 0x01, 0x03, 0x05, "SWCTRL(5D-010305)");

    software_control_ready = ok;
    LOG_DEBUG("[%s] Software control bootstrap %s", "LogitechG515Controller", ok ? "ok" : "failed");
    return ok;
}

unsigned short LogitechG515Controller::ClampSpeedMs(unsigned short speed_ms)
{
    if(speed_ms < LOGITECH_G515_SPEED_FASTEST_MS)
    {
        return LOGITECH_G515_SPEED_FASTEST_MS;
    }
    if(speed_ms > LOGITECH_G515_SPEED_SLOWEST_MS)
    {
        return LOGITECH_G515_SPEED_SLOWEST_MS;
    }
    return speed_ms;
}

unsigned char LogitechG515Controller::ClampBrightness(unsigned char brightness)
{
    if(brightness < 1)
    {
        return 1;
    }
    if(brightness > 100)
    {
        return 100;
    }
    return brightness;
}

void LogitechG515Controller::ApplyOff()
{
    /* OFF sequence from dedicated off captures. */
    (void)SendInit("Init(Off)");

    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x1D;
    buf[4]  = 0x00;
    buf[5]  = 0x00;
    buf[16] = 0x01;

    (void)SendReport(buf, sizeof(buf), "Off");
}

void LogitechG515Controller::ApplyStaticColor(unsigned char red, unsigned char green, unsigned char blue)
{
    /* Use capture-observed mode-static family (all-modes capture), not direct 0x5D frames,
     * to preserve firmware mode ownership.
     */
    (void)SendInit("Init(Static)");

    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x1D;
    buf[4]  = 0x00;
    buf[5]  = 0x01;
    buf[6]  = red;
    buf[7]  = green;
    buf[8]  = blue;
    buf[9]  = 0x02;
    buf[16] = 0x01;

    (void)SendReport(buf, sizeof(buf), "Static");
}

void LogitechG515Controller::ApplyBreathing(unsigned short speed_ms, unsigned char brightness, unsigned char red, unsigned char green, unsigned char blue)
{
    /* Capture shows two init packets before breathing. */
    (void)SendInit("Init(Breathing-1)");
    (void)SendInit("Init(Breathing-2)");

    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x1D;
    buf[4]  = 0x00;
    buf[5]  = 0x02;
    buf[6]  = red;
    buf[7]  = green;
    buf[8]  = blue;
    buf[9]  = (uint8_t)((speed_ms >> 8) & 0xFF);
    buf[10] = (uint8_t)(speed_ms & 0xFF);
    buf[12] = brightness;
    buf[16] = 0x01;

    (void)SendReport(buf, sizeof(buf), "Breathing");
}

void LogitechG515Controller::ApplySpectrumCycle(unsigned short speed_ms, unsigned char brightness)
{
    (void)SendInit("Init(Cycle)");

    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x1D;
    buf[4]  = 0x00;
    buf[5]  = 0x03;
    /* all-modes capture uses 00 FF 00 marker when selecting cycle */
    buf[6]  = 0x00;
    buf[7]  = 0xFF;
    buf[8]  = 0x00;
    buf[12] = (uint8_t)((speed_ms >> 8) & 0xFF);
    buf[13] = (uint8_t)(speed_ms & 0xFF);
    buf[14] = brightness;
    buf[16] = 0x01;

    (void)SendReport(buf, sizeof(buf), "SpectrumCycle");
    (void)SendModeApplyPacket("SpectrumCycle(Apply)");
}

void LogitechG515Controller::ApplyRainbowWave(unsigned short speed_ms, unsigned char brightness)
{
    (void)SendInit("Init(Wave)");

    /* Inference validated against all-modes captures:
     * mode family uses byte[5]=0x02 with subtype byte[4]=0xFF and variant in byte[15].
     * Variant 0x01 is used as default wave profile.
     */
    uint8_t buf[LOGITECH_G515_REPORT_LEN_MAIN] = { 0 };
    buf[0]  = LOGITECH_G515_REPORT_ID_MAIN;
    buf[1]  = profile.device_index;
    buf[2]  = profile.mode_feature_index;
    buf[3]  = 0x1D;
    buf[4]  = 0xFF;
    buf[5]  = 0x02;
    buf[7]  = 0xFF;
    buf[12] = (uint8_t)((speed_ms >> 8) & 0xFF);
    buf[13] = (uint8_t)(speed_ms & 0xFF);
    buf[14] = brightness;
    buf[15] = 0x01;
    buf[16] = 0x01;

    (void)SendReport(buf, sizeof(buf), "RainbowWave");
    (void)SendModeApplyPacket("RainbowWave(Apply)");
}

void LogitechG515Controller::SetOff()
{
    std::lock_guard<std::mutex> lock(io_mutex);
    (void)EnsureSoftwareControl();
    ApplyOff();
}

void LogitechG515Controller::SetStaticColor(unsigned char red, unsigned char green, unsigned char blue)
{
    std::lock_guard<std::mutex> lock(io_mutex);
    (void)EnsureSoftwareControl();
    ApplyStaticColor(red, green, blue);
}

void LogitechG515Controller::SetBreathing(unsigned short speed_ms, unsigned char brightness, unsigned char red, unsigned char green, unsigned char blue)
{
    speed_ms   = ClampSpeedMs(speed_ms);
    brightness = ClampBrightness(brightness);

    std::lock_guard<std::mutex> lock(io_mutex);
    (void)EnsureSoftwareControl();
    ApplyBreathing(speed_ms, brightness, red, green, blue);
}

void LogitechG515Controller::SetSpectrumCycle(unsigned short speed_ms, unsigned char brightness)
{
    speed_ms   = ClampSpeedMs(speed_ms);
    brightness = ClampBrightness(brightness);

    std::lock_guard<std::mutex> lock(io_mutex);
    (void)EnsureSoftwareControl();
    ApplySpectrumCycle(speed_ms, brightness);
}

void LogitechG515Controller::SetRainbowWave(unsigned short speed_ms, unsigned char brightness)
{
    speed_ms   = ClampSpeedMs(speed_ms);
    brightness = ClampBrightness(brightness);

    std::lock_guard<std::mutex> lock(io_mutex);
    (void)EnsureSoftwareControl();
    ApplyRainbowWave(speed_ms, brightness);
}

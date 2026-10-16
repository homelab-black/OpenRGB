/*---------------------------------------------------------*\
| AntEsportsMK4100Controller.h                              |
|                                                           |
| Native HID protocol adapter for Ant Esports MK4100        |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>
#include <string>
#include <mutex>
#include <hidapi.h>
#include "mk4100_protocol.h"

class AntEsportsMK4100Controller
{
public:
    AntEsportsMK4100Controller(hid_device* dev_handle, const std::string& path, const std::string& name);
    ~AntEsportsMK4100Controller();

    bool IsInitialized() const;
    std::string GetErrorString() const;
    std::string GetDeviceLocation() const;
    std::string GetNameString() const;
    std::string GetSerialString() const;

    unsigned int GetActiveHardwareMode() const;
    void CopyCurrentRGB(uint8_t rgb[MK4100_RGB_BYTES]) const;
    void CopyCurrentConfig(uint8_t config[MK4100_CONFIG_BYTES]) const;

    bool ReadConfig(uint8_t config[MK4100_CONFIG_BYTES]);
    bool ReadEffectConfigForMode(unsigned int mode, uint8_t config[MK4100_CONFIG_BYTES]);
    bool WriteConfig(const uint8_t config[MK4100_CONFIG_BYTES]);
    bool SelectCustomMode();
    bool SetSingleKeyRGB(unsigned int slot, uint8_t red, uint8_t green, uint8_t blue);
    bool UpdateRGBFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write = false);
    bool ApplyDirectFrame(const uint8_t rgb[MK4100_RGB_BYTES], bool verify_after_write = false);
    bool RestoreStartupState();

private:
    static int WriteReport(void* context, const uint8_t report[MK4100_REPORT_BYTES]);
    static int ReadReport(void* context, uint8_t report[MK4100_REPORT_BYTES], unsigned timeout_ms);

    hid_device* dev;
    std::string location;
    std::string name;
    std::string error;
    mk4100_device protocol;
    bool initialized;
    mutable std::mutex protocol_mutex;
};

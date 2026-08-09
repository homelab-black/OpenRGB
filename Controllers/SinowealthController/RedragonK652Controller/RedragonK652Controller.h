/*---------------------------------------------------------*\
| RedragonK652Controller.h                                 |
|                                                          |
|   Experimental controller for the wired Redragon K652    |
|                                                          |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <hidapi.h>

#include "RGBController.h"
#include "RedragonK652DeviceRegistry.h"

namespace redragon_k652
{
    static constexpr std::size_t    REPORT_SIZE = 1032;
    static constexpr unsigned int   LED_COUNT   = 83;
    static constexpr unsigned char  MODE_STATIC = 0x01;
    static constexpr unsigned char  MODE_DIRECT = 0x15;

    extern const std::array<unsigned char, LED_COUNT> KEY_INDICES;
}

struct RedragonK652HIDOperations
{
    int  (*send_feature_report)(hid_device*, const unsigned char*, std::size_t);
    int  (*get_feature_report)(hid_device*, unsigned char*, std::size_t);
    void (*close)(hid_device*);
    void (*sleep_ms)(unsigned int);
};

class RedragonK652Controller
{
public:
    RedragonK652Controller(hid_device* dev_cmd_handle, hid_device* dev_data_handle, const char* path, std::string dev_name,
                          RedragonK652DeviceClaim device_claim = {}, const RedragonK652HIDOperations* operations = nullptr);
    ~RedragonK652Controller();

    /* Active validation; call only after the complete passive K652 fingerprint succeeds. */
    static bool     ValidateConfiguration(hid_device* dev_cmd_handle, hid_device* dev_data_handle,
                                          const RedragonK652HIDOperations* operations = nullptr);

    std::string     GetLocation();
    std::string     GetName();
    unsigned int    GetLEDCount();

    /* Permanently enables user/API lighting control for this instance. */
    void            EnableUserLightingControl();
    bool            ApplyStatic(RGBColor color);
    bool            ApplyDirect(const std::vector<RGBColor>& colors);

private:
    bool            QueryConfiguration();
    bool            SendReport(const std::array<unsigned char, redragon_k652::REPORT_SIZE>& report);

    hid_device*     dev_cmd;
    hid_device*     dev_data;
    RedragonK652DeviceClaim device_claim;
    RedragonK652HIDOperations operations;
    std::atomic<bool> user_lighting_control_enabled;
    std::mutex      access_mutex;
    std::string     location;
    std::string     name;
};

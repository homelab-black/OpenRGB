/*---------------------------------------------------------*\
| DasKeyboardX50Controller.h                                |
|                                                           |
|   Driver for the Das Keyboard X50 RGB                    |
|                                                           |
|   Sergio Presa                                15 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "RGBController.h"
#include "hidapi_wrapper.h"

struct DasKeyboardX50Led
{
    unsigned short  position;
    unsigned short  extra_position;
};

class DasKeyboardX50Controller
{
public:
    DasKeyboardX50Controller(hidapi_wrapper hid_wrapper, hid_device* device_handle, const std::string& path, const std::string& serial_number);
    ~DasKeyboardX50Controller();

    std::string GetLocationString() const;
    std::string GetSerialString() const;
    void SendColors(const std::vector<RGBColor>& colors);

    static unsigned int LedCount();

private:
    hidapi_wrapper wrapper;
    hid_device* device;
    std::string location;
    std::string serial;
    std::mutex update_mutex;
    std::condition_variable update_condition;
    std::thread update_thread;
    std::vector<RGBColor> pending_colors;
    std::chrono::steady_clock::time_point preserve_lighting_until;
    bool update_pending;
    bool update_thread_running;

    void RecoverDevice();
    bool TransferReport(const unsigned char* report, unsigned char command, unsigned char page);
    bool SendReport(unsigned char command, unsigned char page, const unsigned char* payload);
    bool SendColorPages(unsigned char command, const std::vector<RGBColor>& colors);
    void UpdateThreadFunction();
};

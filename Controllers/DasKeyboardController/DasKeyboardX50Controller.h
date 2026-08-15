/* Das Keyboard X50 RGB controller - SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <libusb.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "RGBController.h"

struct DasKeyboardX50Led
{
    const char*     name;
    unsigned short  position;
    unsigned short  extra_position;
};

class DasKeyboardX50Controller
{
public:
    DasKeyboardX50Controller(libusb_context* usb_context, libusb_device_handle* device_handle, const std::string& path);
    ~DasKeyboardX50Controller();

    std::string GetLocationString() const;
    std::string GetSerialString() const;
    void SendColors(const std::vector<RGBColor>& colors);

    static const DasKeyboardX50Led* LedTable();
    static unsigned int LedCount();

private:
    libusb_context* context;
    libusb_device_handle* device;
    std::string location;
    std::mutex update_mutex;
    std::condition_variable update_condition;
    std::thread update_thread;
    std::vector<RGBColor> pending_colors;
    std::chrono::steady_clock::time_point preserve_lighting_until;
    bool update_pending;
    bool update_thread_running;
    bool interface_claimed;
    bool device_healthy;
    bool failure_warning_logged;

    bool Initialize();
    bool EnsureInterfaceClaimed();
    bool SendReport(unsigned char command, unsigned char page, const unsigned char* payload);
    bool SendZeroPages(unsigned char command, unsigned int pages);
    bool SendColorPages(unsigned char command, const std::vector<RGBColor>& colors);
    void UpdateThreadFunction();
};

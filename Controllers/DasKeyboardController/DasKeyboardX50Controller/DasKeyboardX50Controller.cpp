/*---------------------------------------------------------*\
| DasKeyboardX50Controller.cpp                              |
|                                                           |
|   Driver for the Das Keyboard X50 RGB                    |
|                                                           |
|   Sergio Presa                                15 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DasKeyboardX50Controller.h"
#include "LogManager.h"
#include "StringUtils.h"
#include <chrono>
#include <cstring>
#include <thread>

using namespace std::chrono_literals;

#define X50_REPORT_SIZE              64
#define X50_PAYLOAD_SIZE             60
#define X50_CHANNEL_STRIDE           144
#define X50_INITIAL_UPDATE_GUARD_MS  100
#define X50_REPORT_COOLDOWN_MS       5
#define X50_LAYER_COOLDOWN_MS        25
#define X50_RECOVERY_COOLDOWN_MS     1000
#define X50_TRANSFER_RETRIES         2
/* Sustained updates at the original 5 FPS test rate eventually
   caused firmware timeouts.  Keep the continuous update rate below
   4 FPS and allow additional quiet time between individual reports. */
#define X50_FRAME_COOLDOWN_MS        250

static const DasKeyboardX50Led x50_leds[] =
{
    {4,   0},
    {5,   0},
    {6,   0},
    {7,   0},
    {8,   0},
    {9,   0},
    {10,  0},
    {13,  0},
    {14,  0},
    {15,  0},
    {16,  0},
    {17,  0},
    {18,  0},
    {19,  0},
    {20,  0},
    {22,  0},
    {23,  0},
    {24,  0},
    {25,  0},
    {26,  0},
    {27,  0},
    {28,  0},
    {29,  0},
    {31,  0},
    {32,  0},
    {33,  0},
    {34,  0},
    {35,  0},
    {36,  0},
    {37,  0},
    {41,  0},
    {42,  0},
    {43,  0},
    {45,  0},
    {49,  0},
    {50,  0},
    {51,  0},
    {52,  0},
    {53,  0},
    {54,  0},
    {58,  0},
    {60, 79},
    {62,  0},
    {63,  0},
    {64,  0},
    {65,  0},
    {67,  0},
    {68,  0},
    {69,  0},
    {70,  0},
    {71,  0},
    {72,  0},
    {73,  0},
    {74,  0},
    {76,  0},
    {77,  0},
    {78,  0},
    {80,  0},
    {81,  0},
    {82,  0},
    {83,  0},
    {85,  0},
    {86,  0},
    {87,  0},
    {88,  0},
    {89,  0},
    {90,  0},
    {91,  0},
    {92,  0},
    {94,  0},
    {95,  0},
    {96,  0},
    {97,  0},
    {98,  0},
    {99,  0},
    {100, 0},
    {103, 0},
    {104, 0},
    {105, 0},
    {106, 0},
    {107, 0},
    {108, 0},
    {109, 0},
    {110, 0},
    {113, 0},
    {114, 0},
    {115, 0},
    {116, 0},
    {119, 0},
    {123, 0},
    {124, 0},
    {125, 0},
    {128, 0},
    {131, 0},
    {132, 0},
    {133, 0},
    {134, 0},
    {136, 0},
    {137, 0},
    {139, 0},
    {140, 0},
    {141, 0},
    {142, 0},
    {143, 0},
    {144, 0},
    {145, 0},
    {146, 0},
    {130, 0},
    {121, 0},
    {135, 0},
    {122, 0}
};

DasKeyboardX50Controller::DasKeyboardX50Controller(hidapi_wrapper hid_wrapper, hid_device* device_handle, const std::string& path, const std::string& serial_number)
    : wrapper(hid_wrapper), device(device_handle), location(path), serial(serial_number), update_pending(false), update_thread_running(true)
{
    preserve_lighting_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(X50_INITIAL_UPDATE_GUARD_MS);
    update_thread = std::thread(&DasKeyboardX50Controller::UpdateThreadFunction, this);
}

DasKeyboardX50Controller::~DasKeyboardX50Controller()
{
    {
        std::lock_guard<std::mutex> lock(update_mutex);
        update_thread_running = false;
    }
    update_condition.notify_one();
    update_thread.join();

    if(device != nullptr)
    {
        wrapper.hid_close(device);
        device = nullptr;
    }
}

std::string DasKeyboardX50Controller::GetLocationString() const
{
    return("USB: " + location);
}

std::string DasKeyboardX50Controller::GetSerialString() const
{
    return(serial);
}

unsigned int DasKeyboardX50Controller::LedCount()
{
    return(sizeof(x50_leds) / sizeof(x50_leds[0]));
}

bool DasKeyboardX50Controller::SendReport(unsigned char command, unsigned char page, const unsigned char* payload)
{
    unsigned char report[X50_REPORT_SIZE] = {0};
    report[0] = 0x07;
    report[1] = command;
    report[2] = 0x06;
    report[3] = page;

    if(payload != nullptr)
    {
        memcpy(&report[4], payload, X50_PAYLOAD_SIZE);
    }

    if(!TransferReport(report, command, page))
    {
        return(false);
    }

    /* The official software follows every page with this confirmation
       report.  Omitting it works briefly, but sustained updates eventually
       fill the firmware queue and time out. */
    unsigned char confirmation[X50_REPORT_SIZE] = {0};
    confirmation[0] = 0x08;
    confirmation[2] = 0x08;
    confirmation[3] = command;

    return(TransferReport(confirmation, command, page));
}

bool DasKeyboardX50Controller::TransferReport(const unsigned char* report, unsigned char command, unsigned char page)
{
    unsigned char hid_report[X50_REPORT_SIZE + 1] = {0};
    memcpy(&hid_report[1], report, X50_REPORT_SIZE);

    for(unsigned int attempt = 0; attempt <= X50_TRANSFER_RETRIES; attempt++)
    {
        int result = device != nullptr ? wrapper.hid_write(device, hid_report, sizeof(hid_report)) : -1;

        if(result == (int)sizeof(hid_report))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(X50_REPORT_COOLDOWN_MS));
            return(true);
        }

        if(attempt < X50_TRANSFER_RETRIES)
        {
            LOG_WARNING("[DasKeyboardX50] HID write retry %u/%u for command 0x%02X page %u",
                        attempt + 1, X50_TRANSFER_RETRIES, command, page);
            std::this_thread::sleep_for(std::chrono::milliseconds(X50_LAYER_COOLDOWN_MS));
            continue;
        }

        const wchar_t* error = device != nullptr ? wrapper.hid_error(device) : nullptr;
        LOG_ERROR("[DasKeyboardX50] HID write failed for command 0x%02X page %u: %s (%d/%u bytes)",
                  command, page, error != nullptr ? StringUtils::wchar_to_char(error) : "device unavailable", result, (unsigned int)sizeof(hid_report));
        return(false);
    }

    return(false);
}

void DasKeyboardX50Controller::RecoverDevice()
{
    if(device != nullptr)
    {
        wrapper.hid_close(device);
        device = nullptr;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(X50_RECOVERY_COOLDOWN_MS));
    device = wrapper.hid_open_path(location.c_str());

    if(device == nullptr)
    {
        LOG_ERROR("[DasKeyboardX50] Reopening the HID device after a failed frame failed");
    }
}

bool DasKeyboardX50Controller::SendColorPages(unsigned char command, const std::vector<RGBColor>& colors)
{
    unsigned char payload[8][X50_PAYLOAD_SIZE] = {{0}};
    const unsigned char fixed[][2] = {{2,31},{2,32},{2,41},{2,50},{2,58},{2,59},{3,4},{3,7},{3,8},{3,16},{3,17},{3,19},{3,21},{3,26},{3,35},{3,39},{3,44},{3,53},{4,1},{4,2},{4,11},{4,12},{4,17},{4,18},{4,20},{4,26},{4,29},{4,38},{4,47}};
    for(unsigned int item_idx = 0; item_idx < sizeof(fixed) / sizeof(fixed[0]); item_idx++)
    {
        payload[fixed[item_idx][0]][fixed[item_idx][1]] = 0xFF;
    }

    unsigned int count = colors.size() < LedCount() ? colors.size() : LedCount();
    for(unsigned int led = 0; led < count; led++)
    {
        unsigned short positions[2] = {x50_leds[led].position, x50_leds[led].extra_position};
        unsigned char values[3] =
        {
            static_cast<unsigned char>(RGBGetRValue(colors[led])),
            static_cast<unsigned char>(RGBGetGValue(colors[led])),
            static_cast<unsigned char>(RGBGetBValue(colors[led]))
        };
        for(unsigned int pos_idx = 0; pos_idx < 2 && positions[pos_idx] != 0; pos_idx++)
        {
            unsigned int base = positions[pos_idx] - 4;
            for(unsigned int channel = 0; channel < 3; channel++)
            {
                unsigned int index = base + channel * X50_CHANNEL_STRIDE;
                payload[index / X50_PAYLOAD_SIZE][index % X50_PAYLOAD_SIZE] = values[channel];
            }
        }
    }
    for(unsigned int page = 0; page < 8; page++)
    {
        if(!SendReport(command, page, payload[page]))
        {
            return(false);
        }
    }
    return(true);
}

void DasKeyboardX50Controller::SendColors(const std::vector<RGBColor>& colors)
{
    bool all_black = true;
    for(unsigned int color_idx = 0; color_idx < colors.size(); color_idx++)
    {
        RGBColor color = colors[color_idx];

        if(RGBGetRValue(color) != 0 || RGBGetGValue(color) != 0 || RGBGetBValue(color) != 0)
        {
            all_black = false;
            break;
        }
    }

    if(all_black && std::chrono::steady_clock::now() < preserve_lighting_until)
    {
        LOG_INFO("[DasKeyboardX50] Ignoring initial black update to preserve the keyboard's current lighting");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(update_mutex);
        pending_colors = colors;
        update_pending = true;
    }
    update_condition.notify_one();
}

void DasKeyboardX50Controller::UpdateThreadFunction()
{
    std::unique_lock<std::mutex> lock(update_mutex);

    while(update_thread_running)
    {
        update_condition.wait(lock, [this]
        {
            return(update_pending || !update_thread_running);
        });

        if(!update_thread_running)
        {
            break;
        }

        std::vector<RGBColor> colors = pending_colors;
        update_pending = false;
        lock.unlock();

        /* The protocol validated by x50_rgb_cli.py uses 0x0A for the
           key-press layer and 0x09 for the normal visible layer. */
        bool update_successful = SendColorPages(0x0A, colors);

        if(update_successful)
        {
            /* The keyboard needs a short quiet period when switching
               from the key-press layer to the normal visible layer. */
            std::this_thread::sleep_for(std::chrono::milliseconds(X50_LAYER_COOLDOWN_MS));
            update_successful = SendColorPages(0x09, colors);
        }

        if(update_successful)
        {
            /* Keep the firmware-safe update rate off OpenRGB's device
               lock.  Updates arriving here are coalesced to the newest
               complete frame. */
            std::this_thread::sleep_for(std::chrono::milliseconds(X50_FRAME_COOLDOWN_MS));
        }
        else
        {
            LOG_WARNING("[DasKeyboardX50] Reopening the HID device after a failed frame; subsequent updates will retry");
            RecoverDevice();
        }

        lock.lock();
    }
}

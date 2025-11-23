/*---------------------------------------------------------*\
| FnaticStreakController.cpp                                |
|                                                           |
|   Driver for Fnatic Streak keyboard                       |
|                                                           |
|   Based on leddy by Hanna Czenczek                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <algorithm>
#include <thread>
#include <chrono>
#include "FnaticStreakController.h"
#include "StringUtils.h"
#include "LogManager.h"

FnaticStreakController::FnaticStreakController(hid_device* dev_handle, const char* path, const unsigned short pid, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    usb_pid     = pid;
    mini        = (pid == FNATIC_MINISTREAK_PID);
    profile     = 1;

    /*-----------------------------------------------------*\
    | Get device name from HID if not provided             |
    \*-----------------------------------------------------*/
    if(dev_name.empty())
    {
        wchar_t product_string[128];
        int ret = hid_get_product_string(dev, product_string, 128);
        if(ret == 0)
        {
            name = StringUtils::wstring_to_string(product_string);
        }
        else
        {
            name = mini ? "Fnatic miniStreak" : "Fnatic Streak";
        }
    }
    else
    {
        name = dev_name;
    }

    /*-----------------------------------------------------*\
    | Set up device-specific parameters                     |
    \*-----------------------------------------------------*/
    if(mini)
    {
        width       = 18;
        led_count   = 106;
        ledmap = {
           1,    0,    7,   13,   19,   25,   31,   37,   43,   49,  103,   55,   67,   73,   79,   90,   93,   98,
           2,    8,   14,   20,   26,   32,   38,   44,   50,   56,   61,   62,   68,   74,   80,   89,   94,   99,
           3,    9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,   75, 0xff,   81,   88,   95,   96,
           4, 0xff,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,   76,   82, 0xff, 0xff, 0xff,
           5,   11,   17,   23,   29,   35,   41,   47,   53,   59,   65,   66,   71,   77, 0xff, 0xff,   87, 0xff,
           6,   12, 0xff,   18,   24,   30,   36,   42,   48,   54,   60,   72, 0xff,   78,   83,   84,   85,   86,
        };
    }
    else
    {
        width       = 22;
        led_count   = 124;
        ledmap = {
           1,    0,    7,   13,   19,   25,   31,   37,   43,   49,  120,   55,   67,   73,   79,   90,   93,   98,   91,   97,   92,  118,
           2,    8,   14,   20,   26,   32,   38,   44,   50,   56,   61,   62,   68,   74,   80,   89,   94,   99,  100,  108,  109,  116,
           3,    9,   15,   21,   27,   33,   39,   45,   51,   57,   63,   69,   75, 0xff,   81,   88,   95,   96,  101,  107,  110,  115,
           4, 0xff,   10,   16,   22,   28,   34,   40,   46,   52,   58,   64,   70,   76,   82, 0xff, 0xff, 0xff,  102,  106,  111, 0xff,
           5,   11,   17,   23,   29,   35,   41,   47,   53,   59,   65,   66,   71,   77, 0xff, 0xff,   87, 0xff,  103,  105,  112,  114,
           6,   12, 0xff,   18,   24,   30,   36,   42,   48,   54,   60,   72, 0xff,   78,   83,   84,   85,   86,  104, 0xff,  113, 0xff,
        };
    }

    color_cmd_prefix = {0x05, profile, 0x02};

    /*-----------------------------------------------------*\
    | Initialize the device                                 |
    \*-----------------------------------------------------*/
    Initialize();
}

FnaticStreakController::~FnaticStreakController()
{
    hid_close(dev);
}

std::string FnaticStreakController::GetLocationString()
{
    return("HID: " + location);
}

std::string FnaticStreakController::GetNameString()
{
    return(name);
}

std::string FnaticStreakController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

unsigned short FnaticStreakController::GetUSBPID()
{
    return(usb_pid);
}

bool FnaticStreakController::IsMini()
{
    return(mini);
}

unsigned int FnaticStreakController::GetLEDCount()
{
    return(led_count);
}

unsigned int FnaticStreakController::GetWidth()
{
    return(width);
}

const std::vector<unsigned char>& FnaticStreakController::GetLEDMap()
{
    return(ledmap);
}

void FnaticStreakController::SetProfile(unsigned char new_profile)
{
    profile = new_profile;
    color_cmd_prefix = {0x05, profile, 0x02};
    RefreshProfile();
}

void FnaticStreakController::SendRequest(const std::vector<unsigned char>& prefix, const unsigned char* raw_data, unsigned int data_len)
{
    unsigned int plen = prefix.size();
    unsigned int len = data_len + plen;
    unsigned int ofs = 0;

    unsigned char cmd = (plen > 0) ? prefix[0] : raw_data[0];

    LOG_DEBUG("[FnaticStreak] SendRequest: cmd=0x%02X, total_len=%u, data_len=%u", cmd, len, data_len);

    while(ofs < len)
    {
        unsigned char data[65];
        memset(data, 0x00, sizeof(data));

        data[0] = 0x00;
        data[1] = cmd;

        /*-----------------------------------------------------*\
        | Total length (24-bit little endian)                  |
        \*-----------------------------------------------------*/
        data[2] = len & 0xFF;
        data[3] = (len >> 8) & 0xFF;
        data[4] = (len >> 16) & 0xFF;

        /*-----------------------------------------------------*\
        | Current offset (24-bit little endian)                |
        \*-----------------------------------------------------*/
        data[5] = ofs & 0xFF;
        data[6] = (ofs >> 8) & 0xFF;
        data[7] = (ofs >> 16) & 0xFF;

        /*-----------------------------------------------------*\
        | Copy data payload (max 57 bytes per packet)          |
        \*-----------------------------------------------------*/
        for(unsigned int i = ofs; i < std::min(ofs + 57, len); i++)
        {
            if(i >= plen)
            {
                data[i - ofs + 8] = raw_data[i - plen];
            }
            else
            {
                data[i - ofs + 8] = prefix[i];
            }
        }

        int bytes_written = hid_write(dev, data, 65);
        if(bytes_written < 0)
        {
            LOG_ERROR("[FnaticStreak] HID write failed at offset %u", ofs);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        ofs += 57;
    }

    /*-----------------------------------------------------*\
    | If this is a profile command, save and show it        |
    \*-----------------------------------------------------*/
    if(cmd == 0x05)
    {
        /* Save changes */
        std::vector<unsigned char> save_cmd = {0x13};
        SendRequest(save_cmd, nullptr, 0);

        /* Show profile */
        std::vector<unsigned char> show_cmd = {0x04};
        unsigned char profile_data = profile;
        SendRequest(show_cmd, &profile_data, 1);
    }
}

void FnaticStreakController::RefreshProfile()
{
    std::vector<unsigned char> cmd = {0x04};
    unsigned char profile_data = profile;
    SendRequest(cmd, &profile_data, 1);
}

void FnaticStreakController::SendColors(unsigned char* color_data, unsigned int color_data_size)
{
    /*-----------------------------------------------------*\
    | Send all keys command with color data                 |
    \*-----------------------------------------------------*/
    if(color_cmd_prefix[0] == 0x05)
    {
        std::vector<unsigned char> cmd = {0x05, profile, 0x02, 0x03};
        SendRequest(cmd, color_data, color_data_size);
    }
    else
    {
        std::vector<unsigned char> cmd = {0x0f, 0x03};
        SendRequest(cmd, color_data, color_data_size);
    }

    /*-----------------------------------------------------*\
    | Small delay to let device process the command        |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void FnaticStreakController::Initialize()
{
    /*-----------------------------------------------------*\
    | Refresh the current profile to ensure device is ready|
    \*-----------------------------------------------------*/
    RefreshProfile();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

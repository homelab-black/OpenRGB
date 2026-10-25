/*---------------------------------------------------------*\
| TurtleBeachKB7Controller.cpp                              |
|                                                           |
|   Driver for Turtle Beach Command Series KB7 keyboard     |
|                                                           |
|   Joe Peterson (joevpeterson@gmail.com)       04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include "StringUtils.h"
#include "TurtleBeachKB7Controller.h"

using namespace std::chrono_literals;

TurtleBeachKB7Controller::TurtleBeachKB7Controller(hid_device* dev_handle, const char* path)
{
    dev                     = dev_handle;
    location                = path;
    last_led_data_size      = 0;

    memset(last_led_data, 0x00, sizeof(last_led_data));

    EnterDirectMode();

    keepalive_running       = true;
    keepalive_thread        = std::thread(&TurtleBeachKB7Controller::KeepAliveThreadFunction, this);
}

TurtleBeachKB7Controller::~TurtleBeachKB7Controller()
{
    /*-----------------------------------------------------*\
    | Stop and join the keep alive thread before taking     |
    | led_data_mutex, which the thread also locks           |
    \*-----------------------------------------------------*/
    keepalive_running = false;

    if(keepalive_thread.joinable())
    {
        keepalive_thread.join();
    }

    /*-----------------------------------------------------*\
    | Null out dev under led_data_mutex so a write in       |
    | flight on another thread finishes before hid_close    |
    \*-----------------------------------------------------*/
    std::lock_guard<std::mutex> lock(led_data_mutex);

    hid_device* dev_to_close = dev;
    dev = nullptr;

    if(dev_to_close != nullptr)
    {
        hid_close(dev_to_close);
    }
}

std::string TurtleBeachKB7Controller::GetLocation()
{
    return("HID: " + location);
}

std::string TurtleBeachKB7Controller::GetSerialString()
{
    std::lock_guard<std::mutex> lock(led_data_mutex);

    if(dev == nullptr)
    {
        return("");
    }

    wchar_t serial_string[128];
    int     ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void TurtleBeachKB7Controller::EnterDirectMode()
{
    /*-----------------------------------------------------*\
    | The enable report is sent on a different interface    |
    | than the color stream, so a second device handle is   |
    | opened for it and closed again once it has been sent. |
    |                                                       |
    | Interface 2 is itself a composite of six separate top |
    | level HID collections (keyboard, consumer control,    |
    | system control, and three vendor defined), each with  |
    | its own device path.  Only one of them actually owns  |
    | report ID 0x0E in its report descriptor, and Windows  |
    | correctly rejects a SET_FEATURE for a report ID that  |
    | is not present on the collection the handle was       |
    | opened against.  hidapi's enumeration order across    |
    | these sibling collections is not guaranteed stable -  |
    | it can and does change across reconnects - so every   |
    | interface 2 path is tried in turn and the loop stops  |
    | at the first one that actually succeeds, rather than  |
    | the first one enumerated.                             |
    \*-----------------------------------------------------*/
    hid_device_info* devs = hid_enumerate(TURTLE_BEACH_VID, TURTLE_BEACH_KB7_PID);
    hid_device_info* cur  = devs;

    while(cur != NULL)
    {
        if(cur->interface_number == TURTLE_BEACH_KB7_ENABLE_INTERFACE)
        {
            hid_device* enable_dev = hid_open_path(cur->path);

            if(enable_dev != NULL)
            {
                unsigned char buf[TURTLE_BEACH_KB7_ENABLE_REPORT_SIZE];

                /*-------------------------------------------*\
                | This report is numbered, so the report ID  |
                | goes in the first byte with no padding.    |
                \*-------------------------------------------*/
                memset(buf, 0x00, sizeof(buf));

                buf[0] = TURTLE_BEACH_KB7_ENABLE_REPORT_ID;
                buf[1] = 0x05;
                buf[2] = 0x01;

                int result = hid_send_feature_report(enable_dev, buf, sizeof(buf));
                hid_close(enable_dev);

                if(result >= 0)
                {
                    break;
                }
            }
        }

        cur = cur->next;
    }

    hid_free_enumeration(devs);
}

void TurtleBeachKB7Controller::SendDirect(unsigned char* led_data, unsigned int led_data_size)
{
    unsigned int copy_size = led_data_size;

    if(copy_size > sizeof(last_led_data))
    {
        copy_size = sizeof(last_led_data);
    }

    /*-----------------------------------------------------*\
    | Cache the frame for the keep alive thread.  The lock  |
    | is held across the write so only one thread writes    |
    | to the device handle at a time                        |
    \*-----------------------------------------------------*/
    std::lock_guard<std::mutex> lock(led_data_mutex);

    memcpy(last_led_data, led_data, copy_size);
    last_led_data_size = copy_size;

    SendFrame(led_data, led_data_size);
}

void TurtleBeachKB7Controller::SendFrame(unsigned char* led_data, unsigned int led_data_size)
{
    /*-----------------------------------------------------*\
    | Caller holds led_data_mutex.  dev is null once the    |
    | destructor has closed the handle                      |
    \*-----------------------------------------------------*/
    if(dev == nullptr)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | The output report is unnumbered, so the buffer needs  |
    | a leading zero byte for hidapi to consume as the      |
    | report ID and one extra byte of length.  See the      |
    | header for details.                                   |
    \*-----------------------------------------------------*/
    unsigned char buf[TURTLE_BEACH_KB7_WRITE_SIZE];

    /*-----------------------------------------------------*\
    | The LED buffer is split across six pages sent back to |
    | back.  The first page also carries the total payload  |
    | length as a little endian short after the page index. |
    \*-----------------------------------------------------*/
    for(unsigned int page = 0; page < TURTLE_BEACH_KB7_PAGE_COUNT; page++)
    {
        memset(buf, 0x00, sizeof(buf));

        buf[1] = TURTLE_BEACH_KB7_REPORT_ID;
        buf[2] = (unsigned char)(page + 1);

        if(page == 0)
        {
            buf[3] = (unsigned char)(TURTLE_BEACH_KB7_BUFFER_SIZE & 0xFF);
            buf[4] = (unsigned char)((TURTLE_BEACH_KB7_BUFFER_SIZE >> 8) & 0xFF);
        }

        unsigned int page_offset = page * TURTLE_BEACH_KB7_PAGE_DATA_BYTES;
        unsigned int copy_size   = 0;

        if(page_offset < led_data_size)
        {
            copy_size = led_data_size - page_offset;

            if(copy_size > TURTLE_BEACH_KB7_PAGE_DATA_BYTES)
            {
                copy_size = TURTLE_BEACH_KB7_PAGE_DATA_BYTES;
            }

            memcpy(&buf[5], &led_data[page_offset], copy_size);
        }

        hid_write(dev, buf, sizeof(buf));
    }
}

void TurtleBeachKB7Controller::KeepAliveThreadFunction()
{
    while(keepalive_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(TURTLE_BEACH_KB7_KEEPALIVE_DELAY_MS));

        if(!keepalive_running)
        {
            break;
        }

        /*-------------------------------------------------*\
        | Hold the lock across the write, as in SendDirect |
        \*-------------------------------------------------*/
        std::lock_guard<std::mutex> lock(led_data_mutex);

        if(last_led_data_size > 0)
        {
            SendFrame(last_led_data, last_led_data_size);
        }
    }
}

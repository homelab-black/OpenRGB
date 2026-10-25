/*---------------------------------------------------------*\
| TurtleBeachKB7Controller.h                                |
|                                                           |
|   Driver for Turtle Beach Command Series KB7 keyboard     |
|                                                           |
|   Joe Peterson (joevpeterson@gmail.com)       04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <hidapi.h>

/*---------------------------------------------------------*\
| Turtle Beach vendor ID (Voyetra Turtle Beach, Inc.)       |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_VID                            0x10F5

/*---------------------------------------------------------*\
| The KB7 is a four interface composite HID device.         |
| Interface 1 carries the per key RGB stream and is the     |
| interface this controller is registered against.          |
| Interface 0 is the Command Display's own channel,         |
| interface 2 is the keyboard and media key collection,     |
| and interface 3 is a boot protocol mouse used by the      |
| touchscreen.  Only the RGB stream is implemented here.    |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_PID                        0x5038
#define TURTLE_BEACH_KB7_INTERFACE                  1

/*---------------------------------------------------------*\
| The device ignores the RGB stream until it is switched    |
| into host controlled mode by a feature report.  That      |
| report is sent on interface 2, not on the RGB interface,  |
| so it needs a second device handle opened separately.     |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_ENABLE_INTERFACE           2
#define TURTLE_BEACH_KB7_ENABLE_REPORT_ID           0x0E
#define TURTLE_BEACH_KB7_ENABLE_REPORT_SIZE         5

/*---------------------------------------------------------*\
| RGB report layout (interface 1, EP 0x0D OUT, interrupt,   |
| 64 byte reports on the wire):                             |
|                                                           |
|       byte 0      tag byte (0xA1)                         |
|       byte 1      page index (0x01 - 0x06)                |
|       byte 2-3    page 1 only, total payload length as a  |
|                    little endian short (0x0168 = 360).    |
|                    Zero on pages 2 through 6.             |
|       byte 4+     60 bytes of LED buffer data per page    |
|                                                           |
| One full frame is 360 bytes and does not fit in a single  |
| report, so it is split across six pages sent back to      |
| back.  The device acknowledges each page on EP 0x85 IN    |
| but the acknowledgement carries no state, so it is not    |
| read here.                                                |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_REPORT_SIZE                64
#define TURTLE_BEACH_KB7_REPORT_ID                  0xA1
#define TURTLE_BEACH_KB7_PAGE_COUNT                 6
#define TURTLE_BEACH_KB7_PAGE_DATA_BYTES            60
#define TURTLE_BEACH_KB7_BUFFER_SIZE                (TURTLE_BEACH_KB7_PAGE_COUNT * TURTLE_BEACH_KB7_PAGE_DATA_BYTES)

/*---------------------------------------------------------*\
| This device's report descriptor declares no report ID for |
| its output report, so the report is unnumbered.  hidapi   |
| always consumes the first byte of the buffer passed to    |
| hid_write as a report ID and strips it before the         |
| transfer, so an unnumbered report has to be written with  |
| a leading zero byte and a buffer one byte longer than the |
| report itself.  Writing the tag byte at index zero        |
| instead silently drops it and shifts the whole frame.     |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_WRITE_SIZE                 (TURTLE_BEACH_KB7_REPORT_SIZE + 1)

/*---------------------------------------------------------*\
| The LED buffer is planar rather than interleaved.  Each   |
| of the seven rows is three consecutive 16 byte blocks     |
| holding the red, then green, then blue values for up to   |
| 16 key columns.  RGBController_TurtleBeachKB7.cpp holds   |
| the table mapping each LED index to a row and column.     |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_PLANE_WIDTH                16
#define TURTLE_BEACH_KB7_ROW_BYTES                  (TURTLE_BEACH_KB7_PLANE_WIDTH * 3)

/*---------------------------------------------------------*\
| The LEDs are exposed as three zones, in this order in the |
| key name / position tables in RGBController_TurtleBeachKB7 |
| .cpp: the six dedicated hardware keys above the function  |
| row, then the 78 standard keyboard keys, then the ten     |
| wrist rest glow LEDs.  The hardware keys and the wrist    |
| rest are physically separate lighting elements from the   |
| main key area, so each gets its own zone.                 |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_HWKEY_COUNT                6
#define TURTLE_BEACH_KB7_KEY_COUNT                  78
#define TURTLE_BEACH_KB7_WRIST_COUNT                10
#define TURTLE_BEACH_KB7_NONWRIST_COUNT             (TURTLE_BEACH_KB7_HWKEY_COUNT + TURTLE_BEACH_KB7_KEY_COUNT)
#define TURTLE_BEACH_KB7_LED_COUNT                  (TURTLE_BEACH_KB7_NONWRIST_COUNT + TURTLE_BEACH_KB7_WRIST_COUNT)

/*---------------------------------------------------------*\
| The vendor software re-sends the full frame continuously  |
| at roughly this interval rather than only on change, so   |
| the last frame is re-sent here on the same cadence to     |
| hold the device in host controlled mode.                  |
\*---------------------------------------------------------*/
#define TURTLE_BEACH_KB7_KEEPALIVE_DELAY_MS         50

class TurtleBeachKB7Controller
{
public:
    TurtleBeachKB7Controller(hid_device* dev_handle, const char* path);
    ~TurtleBeachKB7Controller();

    std::string         GetLocation();
    std::string         GetSerialString();

    void                SendDirect(unsigned char* led_data, unsigned int led_data_size);

private:
    void                EnterDirectMode();
    void                KeepAliveThreadFunction();
    void                SendFrame(unsigned char* led_data, unsigned int led_data_size);

    hid_device*         dev;
    std::string         location;

    std::thread         keepalive_thread;
    std::atomic<bool>   keepalive_running;
    std::mutex          led_data_mutex;
    unsigned char       last_led_data[TURTLE_BEACH_KB7_BUFFER_SIZE];
    unsigned int        last_led_data_size;
};

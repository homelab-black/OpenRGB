/*---------------------------------------------------------*\
| HyperXAlloyRiseController.cpp                             |
|                                                           |
|   Driver for HyperX Alloy Rise keyboard (full size)       |
|                                                           |
|   Protocol obtained by capturing USB traffic between      |
|   NGENUITY and the keyboard (USB VID 03F0, PID 0FA0).     |
|                                                           |
|   Commands are 64 byte reports sent to HID interface 2.   |
|   The interface uses report IDs, so the first byte of     |
|   every packet is also the report ID.                     |
|                                                           |
|   The keyboard answers every packet and drops packets     |
|   that arrive before the previous one was answered, so    |
|   each packet is sent only after the replies to the       |
|   previous one were read:                                 |
|                                                           |
|     10 01                 hello             (2 replies)   |
|     40 01 00 00 ff        start             (1 reply)     |
|     44 01 07              begin frame       (1 reply)     |
|     44 02 <n> 00 <60 B>   20 LEDs, R,G,B    (2 replies)   |
|     20 05 / 22 07         sent once after the first       |
|                           frames            (1 reply)     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <chrono>
#include <thread>
#include "HyperXAlloyRiseController.h"
#include "LogManager.h"
#include "StringUtils.h"

#define RISE_LOG_NAME           "HyperXAlloyRise"
#define RISE_DEBUG_PACKETS      160
#define RISE_REPLY_TIMEOUT_MS   20

#define PACKET_SIZE             64
#define SLOT_COUNT              140
#define SLOTS_PER_PACKET        20
#define PACKETS_PER_FRAME       7

/*---------------------------------------------------------*\
| Frame slot of each key LED, in the same order as the      |
| LED list of the RGBController.                            |
| Slots 0-115 are keys (row-major, left to right),          |
| 116-129 are the light bars (even = left, odd = right)     |
| and 130-139 are unused. Slots 32, 54, 91, 101, 105 and    |
| 106 have no LED.                                          |
\*---------------------------------------------------------*/
static const unsigned int key_slot[HYPERX_ALLOY_RISE_KEY_LED_COUNT] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 92, 93, 94, 95, 96, 97, 98, 99, 100, 102, 103, 104, 107, 108, 109, 110, 111, 112, 113, 114, 115
};

#define BAR_FIRST_SLOT          116

static unsigned int debug_out_count  = 0;
static unsigned int debug_in_count   = 0;
static unsigned int debug_timeouts   = 0;
static unsigned int debug_unknown    = 0;
static unsigned int debug_slow       = 0;

static std::string HexString(const unsigned char* data, unsigned int len)
{
    static const char digits[] = "0123456789abcdef";
    std::string       out;

    for(unsigned int i = 0; i < len; i++)
    {
        out += digits[data[i] >> 4];
        out += digits[data[i] & 0x0F];
        out += ' ';
    }

    return(out);
}

static std::string HidErrorString(hid_device* dev)
{
    const wchar_t* err = hid_error(dev);

    if(err)
    {
        return(StringUtils::wstring_to_string(err));
    }

    return("unknown");
}

HyperXAlloyRiseController::HyperXAlloyRiseController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev             = dev_handle;
    location        = path;
    name            = dev_name;
    initialized     = false;
    report_id_mode  = true;
    frames_sent     = 0;
}

HyperXAlloyRiseController::~HyperXAlloyRiseController()
{
    /*-----------------------------------------------------*\
    | No shutdown command was seen in the NGENUITY captures:|
    | it simply stops sending frames.                       |
    \*-----------------------------------------------------*/
    hid_close(dev);
}

std::string HyperXAlloyRiseController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string HyperXAlloyRiseController::GetNameString()
{
    return(name);
}

std::string HyperXAlloyRiseController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool HyperXAlloyRiseController::WritePacket(const unsigned char* data)
{
    unsigned char buf[PACKET_SIZE + 1];
    int           ret;

    if(debug_out_count < RISE_DEBUG_PACKETS)
    {
        LOG_DEBUG("[%s] OUT %s", RISE_LOG_NAME, HexString(data, 12).c_str());
        debug_out_count++;
    }

    if(report_id_mode)
    {
        /*-------------------------------------------------*\
        | The first byte of the packet is the report ID     |
        \*-------------------------------------------------*/
        ret = hid_write(dev, data, PACKET_SIZE);

        if(ret >= 0)
        {
            return(true);
        }

        LOG_WARNING("[%s] hid_write failed (%d): %s - trying without report ID", RISE_LOG_NAME, ret, HidErrorString(dev).c_str());
    }

    /*-----------------------------------------------------*\
    | Fallback for interfaces without report IDs            |
    \*-----------------------------------------------------*/
    buf[0] = 0x00;
    memcpy(&buf[1], data, PACKET_SIZE);

    ret = hid_write(dev, buf, PACKET_SIZE + 1);

    if(ret >= 0)
    {
        report_id_mode = false;
        return(true);
    }

    LOG_ERROR("[%s] hid_write failed in both modes (%d): %s", RISE_LOG_NAME, ret, HidErrorString(dev).c_str());

    return(false);
}

void HyperXAlloyRiseController::FlushInput()
{
    unsigned char rbuf[PACKET_SIZE + 1];

    while(hid_read_timeout(dev, rbuf, sizeof(rbuf), 0) > 0)
    {
    }
}

unsigned int HyperXAlloyRiseController::WaitForReplies(unsigned int expected)
{
    unsigned char rbuf[PACKET_SIZE + 1];
    unsigned int  got      = 0;
    unsigned int  ignored  = 0;
    auto          deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(RISE_REPLY_TIMEOUT_MS);

    while(got < expected)
    {
        auto now = std::chrono::steady_clock::now();

        if(now >= deadline)
        {
            break;
        }

        int wait_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count();

        if(wait_ms < 1)
        {
            wait_ms = 1;
        }

        int len = hid_read_timeout(dev, rbuf, sizeof(rbuf), wait_ms);

        if(len <= 0)
        {
            break;
        }

        /*-------------------------------------------------*\
        | Replies always start with 0xFF (echo), 0x45       |
        | (acknowledge) or 0x11 (device info). Anything else|
        | is an unsolicited report, for example the one the |
        | keyboard sends when a key is pressed, and must not|
        | be counted as a reply.                            |
        \*-------------------------------------------------*/
        if(rbuf[0] == 0xFF || rbuf[0] == 0x45 || rbuf[0] == 0x11)
        {
            if(debug_in_count < RISE_DEBUG_PACKETS)
            {
                LOG_DEBUG("[%s] IN  (%d bytes) %s", RISE_LOG_NAME, len, HexString(rbuf, 20).c_str());
                debug_in_count++;
            }

            got++;
        }
        else
        {
            ignored++;

            if(debug_unknown < 60)
            {
                LOG_DEBUG("[%s] Unsolicited IN (%d bytes) %s", RISE_LOG_NAME, len, HexString(rbuf, 24).c_str());
                debug_unknown++;
            }
        }
    }

    if(got < expected && debug_timeouts < 20)
    {
        LOG_WARNING("[%s] Expected %u replies but got %u (%u unsolicited reports ignored)", RISE_LOG_NAME, expected, got, ignored);
        debug_timeouts++;
    }

    return(got);
}

void HyperXAlloyRiseController::SendCommand(unsigned char cmd, unsigned char sub, unsigned char arg2, unsigned char arg3, unsigned char arg4, unsigned int expected_replies)
{
    unsigned char pkt[PACKET_SIZE];

    memset(pkt, 0x00, sizeof(pkt));

    pkt[0x00] = cmd;
    pkt[0x01] = sub;
    pkt[0x02] = arg2;
    pkt[0x03] = arg3;
    pkt[0x04] = arg4;

    if(WritePacket(pkt))
    {
        WaitForReplies(expected_replies);
    }
}

void HyperXAlloyRiseController::SendFrame(const unsigned char* slot_colors)
{
    unsigned char pkt[PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Announce a frame made of 7 packets                    |
    \*-----------------------------------------------------*/
    SendCommand(0x44, 0x01, PACKETS_PER_FRAME, 0x00, 0x00, 1);

    for(unsigned int pkt_idx = 0; pkt_idx < PACKETS_PER_FRAME; pkt_idx++)
    {
        memset(pkt, 0x00, sizeof(pkt));

        pkt[0x00] = 0x44;
        pkt[0x01] = 0x02;
        pkt[0x02] = (unsigned char)pkt_idx;
        pkt[0x03] = 0x00;

        memcpy(&pkt[0x04], &slot_colors[pkt_idx * SLOTS_PER_PACKET * 3], SLOTS_PER_PACKET * 3);

        /*-------------------------------------------------*\
        | Each color packet is answered by an echo and an   |
        | acknowledge (45 02 <index>)                       |
        \*-------------------------------------------------*/
        if(WritePacket(pkt))
        {
            WaitForReplies(2);
        }
    }
}

void HyperXAlloyRiseController::SetLEDsDirect(const std::vector<RGBColor>& colors)
{
    unsigned char slot_colors[SLOT_COUNT * 3];

    if(colors.size() < (HYPERX_ALLOY_RISE_KEY_LED_COUNT + (2 * HYPERX_ALLOY_RISE_BAR_LED_COUNT)))
    {
        return;
    }

    memset(slot_colors, 0x00, sizeof(slot_colors));

    /*-----------------------------------------------------*\
    | Keys                                                  |
    \*-----------------------------------------------------*/
    for(unsigned int key = 0; key < HYPERX_ALLOY_RISE_KEY_LED_COUNT; key++)
    {
        unsigned int slot = key_slot[key];

        slot_colors[(slot * 3) + 0] = RGBGetRValue(colors[key]);
        slot_colors[(slot * 3) + 1] = RGBGetGValue(colors[key]);
        slot_colors[(slot * 3) + 2] = RGBGetBValue(colors[key]);
    }

    /*-----------------------------------------------------*\
    | Light bars: left bar uses even slots, right bar odd   |
    \*-----------------------------------------------------*/
    for(unsigned int seg = 0; seg < HYPERX_ALLOY_RISE_BAR_LED_COUNT; seg++)
    {
        RGBColor      left       = colors[HYPERX_ALLOY_RISE_KEY_LED_COUNT + seg];
        RGBColor      right      = colors[HYPERX_ALLOY_RISE_KEY_LED_COUNT + HYPERX_ALLOY_RISE_BAR_LED_COUNT + seg];
        unsigned int  left_slot  = BAR_FIRST_SLOT + (seg * 2);
        unsigned int  right_slot = BAR_FIRST_SLOT + (seg * 2) + 1;

        slot_colors[(left_slot  * 3) + 0] = RGBGetRValue(left);
        slot_colors[(left_slot  * 3) + 1] = RGBGetGValue(left);
        slot_colors[(left_slot  * 3) + 2] = RGBGetBValue(left);
        slot_colors[(right_slot * 3) + 0] = RGBGetRValue(right);
        slot_colors[(right_slot * 3) + 1] = RGBGetGValue(right);
        slot_colors[(right_slot * 3) + 2] = RGBGetBValue(right);
    }

    std::lock_guard<std::mutex> guard(write_mutex);

    /*-----------------------------------------------------*\
    | Same start-up order NGENUITY uses                     |
    \*-----------------------------------------------------*/
    if(!initialized)
    {
        LOG_INFO("[%s] Sending start-up sequence", RISE_LOG_NAME);

        FlushInput();

        SendCommand(0x10, 0x01, 0x00, 0x00, 0x00, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        SendCommand(0x40, 0x01, 0x00, 0x00, 0xFF, 1);

        initialized = true;
    }

    auto frame_start = std::chrono::steady_clock::now();

    FlushInput();
    SendFrame(slot_colors);
    frames_sent++;

    long frame_ms = (long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - frame_start).count();

    if(frame_ms > 40 && debug_slow < 20)
    {
        LOG_WARNING("[%s] Slow frame: %ld ms", RISE_LOG_NAME, frame_ms);
        debug_slow++;
    }

    /*-----------------------------------------------------*\
    | NGENUITY sends these two commands once, after the     |
    | first four frames. Their exact purpose is unknown.    |
    \*-----------------------------------------------------*/
    if(frames_sent == 4)
    {
        SendCommand(0x20, 0x05, 0x00, 0x00, 0x00, 1);
        SendCommand(0x22, 0x07, 0x00, 0x00, 0x00, 1);
    }
}

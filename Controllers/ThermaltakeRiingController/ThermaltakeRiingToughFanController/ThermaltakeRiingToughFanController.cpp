/*---------------------------------------------------------*\
| ThermaltakeRiingToughFanController.cpp                    |
|                                                           |
|   Driver for Thermaltake ToughFan 14 RGB                  |
|                                                           |
|   Omar Jaber (Bewinxed)                       18 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "LogManager.h"
#include "StringUtils.h"
#include "ThermaltakeRiingToughFanController.h"

#define THERMALTAKE_TOUGHFAN_CONTROLLER_NAME    "ThermaltakeRiingToughFan"

ThermaltakeRiingToughFanController::ThermaltakeRiingToughFanController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;

    current_mode    = THERMALTAKE_TOUGHFAN_MODE_DIRECT;
    current_speed   = 0x00;

    /*---------------------------------------------------------*\
    | Get device name from HID manufacturer and product strings |
    \*---------------------------------------------------------*/
    wchar_t name_string[HID_MAX_STR];

    hid_get_manufacturer_string(dev, name_string, HID_MAX_STR);
    device_name = StringUtils::wstring_to_string(name_string);

    hid_get_product_string(dev, name_string, HID_MAX_STR);
    device_name.append(" ").append(StringUtils::wstring_to_string(name_string));

    /*---------------------------------------------------------*\
    | Initialize controller and detect active channels          |
    \*---------------------------------------------------------*/
    SendInit();
    DrainRead();
    DetectChannels();

    /*---------------------------------------------------------*\
    | Set up the per-channel send buffers with constant header  |
    | bytes: [0x00] [0x32] [0x52] [channel] [mode]             |
    \*---------------------------------------------------------*/
    memset(tt_buffer, 0x00, sizeof(tt_buffer));

    unsigned char temp_buffer[3] = { 0x00, 0x32, 0x52 };

    for(std::size_t zone_index = 0; zone_index < THERMALTAKE_TOUGHFAN_NUM_CHANNELS; zone_index++)
    {
        memcpy(&tt_buffer[zone_index][0], temp_buffer, 3);
    }

    /*---------------------------------------------------------*\
    | The ToughFan controller only runs in direct mode and      |
    | requires a packet within seconds to remain in the set     |
    | mode (similar to Corsair Node Pro). Start a thread to     |
    | send a packet every THERMALTAKE_TOUGHFAN_KEEPALIVE secs   |
    \*---------------------------------------------------------*/
    keepalive_thread_run = 1;
    keepalive_thread = new std::thread(&ThermaltakeRiingToughFanController::KeepaliveThread, this);
}

ThermaltakeRiingToughFanController::~ThermaltakeRiingToughFanController()
{
    keepalive_thread_run = 0;
    keepalive_thread->join();
    delete keepalive_thread;

    hid_close(dev);
}

void ThermaltakeRiingToughFanController::KeepaliveThread()
{
    while(keepalive_thread_run.load())
    {
        if((std::chrono::steady_clock::now() - last_commit_time) > std::chrono::seconds(THERMALTAKE_TOUGHFAN_KEEPALIVE))
        {
            SendBuffer();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::string ThermaltakeRiingToughFanController::GetDeviceName()
{
    return device_name;
}

std::string ThermaltakeRiingToughFanController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ThermaltakeRiingToughFanController::GetSerial()
{
    wchar_t serial_string[HID_MAX_STR];
    int ret = hid_get_serial_number_string(dev, serial_string, HID_MAX_STR);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string ThermaltakeRiingToughFanController::GetFirmwareVersion()
{
    return firmware_version;
}

unsigned int ThermaltakeRiingToughFanController::GetLEDsPerFan()
{
    return THERMALTAKE_TOUGHFAN_LEDS_PER_FAN;
}

bool ThermaltakeRiingToughFanController::IsChannelActive(unsigned int channel)
{
    if(channel < THERMALTAKE_TOUGHFAN_NUM_CHANNELS)
    {
        return channel_active[channel];
    }
    return false;
}

void ThermaltakeRiingToughFanController::SetChannelLEDs(unsigned char channel, RGBColor* colors, unsigned int num_colors)
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    /*---------------------------------------------------------*\
    | Convert RGBColor array to GRB byte data in the buffer     |
    \*---------------------------------------------------------*/
    for(unsigned int color = 0; color < num_colors; color++)
    {
        unsigned int color_idx = THERMALTAKE_TOUGHFAN_DATA_BYTE + (color * 3);

        tt_buffer[channel][color_idx + 0] = RGBGetGValue(colors[color]);
        tt_buffer[channel][color_idx + 1] = RGBGetRValue(colors[color]);
        tt_buffer[channel][color_idx + 2] = RGBGetBValue(colors[color]);
    }

    tt_buffer[channel][THERMALTAKE_TOUGHFAN_ZONE_BYTE] = channel + 1;
    tt_buffer[channel][THERMALTAKE_TOUGHFAN_MODE_BYTE] = current_mode + (current_speed & 0x03);

    hid_write(dev, tt_buffer[channel], THERMALTAKE_TOUGHFAN_PACKET_SIZE);

    /*---------------------------------------------------------*\
    | Read the response to prevent HID pipe stalling.           |
    | The ToughFan controller sends a response for every write  |
    | and will stall if the read buffer fills up.               |
    \*---------------------------------------------------------*/
    uint8_t response[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
    hid_read_timeout(dev, response, sizeof(response), THERMALTAKE_TOUGHFAN_INTERRUPT_TIMEOUT);

    last_commit_time = std::chrono::steady_clock::now();
}

void ThermaltakeRiingToughFanController::SetMode(unsigned char mode, unsigned char speed)
{
    current_mode    = mode;
    current_speed   = speed;
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void ThermaltakeRiingToughFanController::SendInit()
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    unsigned char usb_buf[THERMALTAKE_TOUGHFAN_PACKET_SIZE];

    /*---------------------------------------------------------*\
    | Zero out buffer                                           |
    \*---------------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*---------------------------------------------------------*\
    | Set up Init packet                                        |
    \*---------------------------------------------------------*/
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0xFE;
    usb_buf[0x02]   = 0x33;

    /*---------------------------------------------------------*\
    | Send packet                                               |
    \*---------------------------------------------------------*/
    hid_write(dev, usb_buf, THERMALTAKE_TOUGHFAN_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, THERMALTAKE_TOUGHFAN_PACKET_SIZE, THERMALTAKE_TOUGHFAN_INTERRUPT_TIMEOUT);

    /*---------------------------------------------------------*\
    | Query firmware version                                    |
    \*---------------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0x00]   = 0x00;
    usb_buf[0x01]   = 0x33;
    usb_buf[0x02]   = 0x50;

    hid_write(dev, usb_buf, THERMALTAKE_TOUGHFAN_PACKET_SIZE);

    unsigned char fw_resp[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
    int ret = hid_read_timeout(dev, fw_resp, sizeof(fw_resp), THERMALTAKE_TOUGHFAN_INTERRUPT_TIMEOUT);

    if(ret > 4 && fw_resp[0] == 0x33 && fw_resp[1] == 0x50)
    {
        firmware_version = std::to_string(fw_resp[2]) + "."
                         + std::to_string(fw_resp[3]) + "."
                         + std::to_string(fw_resp[4]);

        LOG_DEBUG("[%s] Firmware version: %s", THERMALTAKE_TOUGHFAN_CONTROLLER_NAME, firmware_version.c_str());
    }
    else
    {
        firmware_version = "";
    }
}

void ThermaltakeRiingToughFanController::DetectChannels()
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    /*---------------------------------------------------------*\
    | Query each channel's fan RPM to determine if a fan is     |
    | connected. The response format (after hid_read) is:       |
    |   [0]=0x33 [1]=0x51 [2]=channel [3]=0xFF [4]=0x3C        |
    |   [5]=RPM_lo [6]=RPM_hi                                  |
    | Non-zero RPM means a fan is physically connected.         |
    \*---------------------------------------------------------*/
    for(unsigned int ch = 0; ch < THERMALTAKE_TOUGHFAN_NUM_CHANNELS; ch++)
    {
        /*-----------------------------------------------------*\
        | Drain any stale reads before each query               |
        \*-----------------------------------------------------*/
        DrainRead();

        unsigned char usb_buf[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
        memset(usb_buf, 0x00, sizeof(usb_buf));

        usb_buf[0x00]   = 0x00;
        usb_buf[0x01]   = 0x33;
        usb_buf[0x02]   = 0x51;
        usb_buf[0x03]   = ch + 1;

        hid_write(dev, usb_buf, THERMALTAKE_TOUGHFAN_PACKET_SIZE);

        unsigned char resp[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
        int ret = hid_read_timeout(dev, resp, sizeof(resp), THERMALTAKE_TOUGHFAN_DETECT_TIMEOUT);

        if(ret > 6 && resp[0] == 0x33 && resp[1] == 0x51)
        {
            unsigned int rpm = (resp[6] << 8) | resp[5];
            channel_active[ch] = (rpm > 0);

            LOG_VERBOSE("[%s] Channel %d: RPM=%d, active=%s",
                THERMALTAKE_TOUGHFAN_CONTROLLER_NAME, ch + 1, rpm,
                channel_active[ch] ? "yes" : "no");
        }
        else
        {
            channel_active[ch] = false;

            LOG_VERBOSE("[%s] Channel %d: no valid response, marking inactive",
                THERMALTAKE_TOUGHFAN_CONTROLLER_NAME, ch + 1);
        }
    }
}

void ThermaltakeRiingToughFanController::SendBuffer()
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    for(std::size_t channel_index = 0; channel_index < THERMALTAKE_TOUGHFAN_NUM_CHANNELS; channel_index++)
    {
        if(channel_active[channel_index])
        {
            hid_write(dev, tt_buffer[channel_index], THERMALTAKE_TOUGHFAN_PACKET_SIZE);

            /*-------------------------------------------------*\
            | Drain the HID read to prevent pipe stalling       |
            \*-------------------------------------------------*/
            unsigned char response[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
            hid_read_timeout(dev, response, sizeof(response), 10);
        }
    }

    /*-------------------------------------*\
    | Update the last commit time           |
    \*-------------------------------------*/
    last_commit_time = std::chrono::steady_clock::now();
}

void ThermaltakeRiingToughFanController::DrainRead()
{
    /*---------------------------------------------------------*\
    | Flush any stale data from the HID read buffer             |
    \*---------------------------------------------------------*/
    unsigned char drain[THERMALTAKE_TOUGHFAN_PACKET_SIZE];
    while(hid_read_timeout(dev, drain, sizeof(drain), 1) > 0) {}
}

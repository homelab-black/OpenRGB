/*---------------------------------------------------------*\
| GMK67Controller.cpp                                       |
|                                                           |
|   Driver for the GMK67 (VID 05AC PID 024F) keyboard.      |
|   Protocol ported from the SignalRGB "GMK68" plugin.      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <chrono>
#include <thread>
#include "GMK67Controller.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

GMK67Controller::GMK67Controller(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

GMK67Controller::~GMK67Controller()
{
    hid_close(dev);
}

std::string GMK67Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string GMK67Controller::GetNameString()
{
    return(name);
}

std::string GMK67Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*---------------------------------------------------------------------*\
| Send a single 64-byte payload as a 65-byte feature report.            |
\*---------------------------------------------------------------------*/
void GMK67Controller::SendFeature(const unsigned char* data)
{
    unsigned char usb_buf[GMK67_PACKET_DATA_LENGTH + 1];

    usb_buf[0] = GMK67_REPORT_ID;
    memcpy(&usb_buf[1], data, GMK67_PACKET_DATA_LENGTH);

    hid_send_feature_report(dev, usb_buf, GMK67_PACKET_DATA_LENGTH + 1);

    std::this_thread::sleep_for(1ms);
}

/*---------------------------------------------------------------------*\
| The keyboard expects a feature-report read after each command block.  |
\*---------------------------------------------------------------------*/
void GMK67Controller::ReadFeature()
{
    unsigned char usb_buf[GMK67_PACKET_DATA_LENGTH + 1];

    memset(usb_buf, 0x00, sizeof(usb_buf));
    usb_buf[0] = GMK67_REPORT_ID;

    hid_get_feature_report(dev, usb_buf, GMK67_PACKET_DATA_LENGTH + 1);

    std::this_thread::sleep_for(1ms);
}

/*---------------------------------------------------------------------*\
| Tell the keyboard we are about to stream a direct colour frame.       |
| Mirrors start_refresh() from the SignalRGB plugin:                    |
|   payload[0] = 0x04 (header)                                          |
|   payload[1] = 0x20 (direct write command)                            |
|   payload[8] = 0x08                                                   |
\*---------------------------------------------------------------------*/
void GMK67Controller::StartRefresh()
{
    unsigned char data[GMK67_PACKET_DATA_LENGTH];

    memset(data, 0x00, sizeof(data));
    data[0] = GMK67_HEADER;
    data[1] = GMK67_CMD_DIRECT_START;
    data[8] = 0x08;

    SendFeature(data);
    ReadFeature();
}

/*---------------------------------------------------------------------*\
| Apply the streamed frame. Mirrors refresh() from the plugin:          |
|   1) an empty payload                                                 |
|   2) payload[0] = 0x04 (header), payload[1] = 0x02 (end)              |
|   3) a feature-report read                                            |
\*---------------------------------------------------------------------*/
void GMK67Controller::EndRefresh()
{
    unsigned char data[GMK67_PACKET_DATA_LENGTH];

    memset(data, 0x00, sizeof(data));
    SendFeature(data);

    memset(data, 0x00, sizeof(data));
    data[0] = GMK67_HEADER;
    data[1] = GMK67_CMD_END;
    SendFeature(data);

    ReadFeature();
}

/*---------------------------------------------------------------------*\
| Stream a full frame of per-key colours.                               |
|                                                                       |
| The 264-byte colour stream is padded up to 7 x 64 bytes and sent as   |
| seven feature reports, exactly as the SignalRGB plugin does.          |
\*---------------------------------------------------------------------*/
void GMK67Controller::SendColors(const unsigned char* color_stream, unsigned int size)
{
    unsigned char buffer[GMK67_COLOR_PACKETS * GMK67_PACKET_DATA_LENGTH];

    memset(buffer, 0x00, sizeof(buffer));

    if(size > sizeof(buffer))
    {
        size = sizeof(buffer);
    }

    memcpy(buffer, color_stream, size);

    StartRefresh();

    for(unsigned int packet = 0; packet < GMK67_COLOR_PACKETS; packet++)
    {
        SendFeature(&buffer[packet * GMK67_PACKET_DATA_LENGTH]);
    }

    EndRefresh();
}

/*---------------------------------------------------------*\
| MCHOSEV9DockController.cpp                                |
|                                                           |
|   Driver for MCHOSE V9 charging dock                      |
|                                                           |
|   Michael Mano <michael.mano26@gmail.com>     24 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "MCHOSEV9DockController.h"
#include "LogManager.h"
#include "StringUtils.h"

MCHOSEV9DockController::MCHOSEV9DockController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev                 = dev_handle;
    location            = path;
    name                = dev_name;

    ReadInfo();
}

MCHOSEV9DockController::~MCHOSEV9DockController()
{
    hid_close(dev);
}

std::string MCHOSEV9DockController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string MCHOSEV9DockController::GetNameString()
{
    return(name);
}

std::string MCHOSEV9DockController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string MCHOSEV9DockController::GetFirmwareVersion()
{
    return(firmware_version);
}

void MCHOSEV9DockController::SendCommand(unsigned char cmd, unsigned char frame_seq, unsigned char total_frames, unsigned char* params, unsigned char param_len)
{
    unsigned char buf[MCHOSE_V9_DOCK_PACKET_SIZE];

    /*---------------------------------------------------------*\
    | Packet layout (after report ID 0x00):                 |
    |   AA <cmd> <type> <frame seq> <total frames> <len> ...|
    \*---------------------------------------------------------*/
    memset(buf, 0x00, MCHOSE_V9_DOCK_PACKET_SIZE);

    buf[0]              = 0x00;
    buf[1]              = MCHOSE_V9_DOCK_START_BYTE;
    buf[2]              = cmd;
    buf[3]              = 0x00;
    buf[4]              = frame_seq;
    buf[5]              = total_frames;
    buf[6]              = param_len;

    if(param_len > 0)
    {
        memcpy(&buf[MCHOSE_V9_DOCK_HEADER_SIZE], params, param_len);
    }

    hid_write(dev, buf, MCHOSE_V9_DOCK_PACKET_SIZE);
}

void MCHOSEV9DockController::ReadInfo()
{
    unsigned char buf[MCHOSE_V9_DOCK_PACKET_SIZE];

    SendCommand(MCHOSE_V9_DOCK_CMD_GET_INFO, 0, 0, NULL, 0);

    /*---------------------------------------------------------*\
    | Wait for the reply to the info request, skipping any  |
    | unrelated input reports                               |
    \*---------------------------------------------------------*/
    for(int attempt = 0; attempt < 5; attempt++)
    {
        memset(buf, 0x00, MCHOSE_V9_DOCK_PACKET_SIZE);

        int bytes_read = hid_read_timeout(dev, buf, MCHOSE_V9_DOCK_PACKET_SIZE, 500);

        if(bytes_read <= 0)
        {
            break;
        }

        if(buf[0] == MCHOSE_V9_DOCK_START_BYTE && buf[1] == MCHOSE_V9_DOCK_CMD_GET_INFO)
        {
            /*---------------------------------------------*\
            | Reply data starts after the 6 byte header,    |
            | firmware version is data bytes 11-13 in       |
            | reverse order, followed by the product name   |
            \*---------------------------------------------*/
            unsigned char* data = &buf[6];
            char version[32];

            snprintf(version, sizeof(version), "%d.%d.%d", data[13], data[12], data[11]);
            firmware_version = version;

            LOG_DEBUG("[%s] On: %d Mode: %d Speed: %d Brightness: %d Sync: %d Color: %02X%02X%02X Firmware: %s",
                      name.c_str(), data[0], data[1], data[3], data[4], data[5], data[6], data[7], data[8], firmware_version.c_str());
            return;
        }
    }

    LOG_WARNING("[%s] No response to info request", name.c_str());
}

void MCHOSEV9DockController::SetEffect(bool enable, unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color)
{
    unsigned char params[10];

    /*---------------------------------------------------------*\
    | Parameters:                                           |
    |   on, mode, mode count, speed, brightness, sync,      |
    |   red, green, blue, direction                         |
    | Sync must be enabled for per-LED colors to be accepted|
    \*---------------------------------------------------------*/
    params[0]           = enable ? 0x01 : 0x00;
    params[1]           = mode;
    params[2]           = MCHOSE_V9_DOCK_MODE_COUNT;
    params[3]           = speed;
    params[4]           = brightness;
    params[5]           = (mode == MCHOSE_V9_DOCK_MODE_DIRECT) ? 0x01 : 0x00;
    params[6]           = RGBGetRValue(color);
    params[7]           = RGBGetGValue(color);
    params[8]           = RGBGetBValue(color);
    params[9]           = 0x00;

    SendCommand(MCHOSE_V9_DOCK_CMD_SET_EFFECT, 0, 0, params, sizeof(params));
}

void MCHOSEV9DockController::SetLEDs(std::vector<RGBColor> colors)
{
    unsigned char color_data[MCHOSE_V9_DOCK_LED_COUNT * 3];
    unsigned int  data_len      = MCHOSE_V9_DOCK_LED_COUNT * 3;

    for(unsigned int led_idx = 0; led_idx < MCHOSE_V9_DOCK_LED_COUNT; led_idx++)
    {
        color_data[(led_idx * 3) + 0] = RGBGetRValue(colors[led_idx]);
        color_data[(led_idx * 3) + 1] = RGBGetGValue(colors[led_idx]);
        color_data[(led_idx * 3) + 2] = RGBGetBValue(colors[led_idx]);
    }

    /*---------------------------------------------------------*\
    | Color data is split into frames of up to 57 bytes, the|
    | total frames field holds the index of the last frame  |
    \*---------------------------------------------------------*/
    unsigned int  frame_count   = (data_len + MCHOSE_V9_DOCK_MAX_PARAM_LEN - 1) / MCHOSE_V9_DOCK_MAX_PARAM_LEN;

    for(unsigned int frame = 0; frame < frame_count; frame++)
    {
        unsigned int offset     = frame * MCHOSE_V9_DOCK_MAX_PARAM_LEN;
        unsigned int len        = data_len - offset;

        if(len > MCHOSE_V9_DOCK_MAX_PARAM_LEN)
        {
            len = MCHOSE_V9_DOCK_MAX_PARAM_LEN;
        }

        SendCommand(MCHOSE_V9_DOCK_CMD_SET_LEDS, frame, frame_count - 1, &color_data[offset], len);
    }
}

/*---------------------------------------------------------*\
| CorsairPeripheralV2Controller.cpp                         |
|                                                           |
|   Driver for Corsair V2 peripherals                       |
|                                                           |
|   Chris M (Dr_No)                             07 Aug 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "CorsairPeripheralV2Controller.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

CorsairPeripheralV2Controller::CorsairPeripheralV2Controller(hid_device* dev_handle, const char* path, std::string name)
{
    dev                 = dev_handle;
    location            = path;
    device_name         = name;

    /*---------------------------------------------------------*\
    | Get PID                                                   |
    |   If the PID is in the know wireless receivers list       |
    |   switch the write_cmd to talk to the device and retry    |
    \*---------------------------------------------------------*/
    unsigned int pid    = GetAddress(0x12);

    switch(pid)
    {
        case CORSAIR_SLIPSTREAM_WIRELESS_PID1:
        case CORSAIR_SLIPSTREAM_WIRELESS_V2_PID1:
        case CORSAIR_SLIPSTREAM_WIRELESS_PID2:
        case CORSAIR_K70_CORE_RGB_TKL_WL_PID:
            write_cmd   = CORSAIR_V2_WRITE_WIRELESS_ID;
            pid         = GetAddress(0x12);
            break;

        case CORSAIR_K57_RGB_WIRED_PID:
            write_cmd   = 0x80;
            light_ctrl  = CORSAIR_V2_LIGHT_CTRL1;
            skip_reads  = true;
            break;
    }

    /*---------------------------------------------------------*\
    | If the hid_pid passed in from the detector does not match |
    |   the pid reported by the device then it is likey         |
    |   behind a wireless receiver.                             |
    \*---------------------------------------------------------*/
    LOG_DEBUG("[%s] Setting write CMD to %02X for %s mode for PID %04X", device_name.c_str(),
              write_cmd, (write_cmd == CORSAIR_V2_WRITE_WIRELESS_ID) ? "wireless" : "wired", pid);

    /*---------------------------------------------------------*\
    | Get VID                                                   |
    |   NB: this can be achieved with GetAddress(0x11) but we   |
    |   also need to set the packet length capabilities for     |
    |   the device being set up.                                |
    \*---------------------------------------------------------*/
    uint8_t buffer[CORSAIR_V2_PACKET_SIZE];
    buffer[1] = write_cmd;
    buffer[2] = CORSAIR_V2_CMD_GET;
    buffer[3] = 0x11;
    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);
    uint16_t result = hid_read_timeout(dev, buffer, CORSAIR_V2_PACKET_SIZE, CORSAIR_V2_TIMEOUT);
    result++;
    pkt_sze = std::max(result, (uint16_t)CORSAIR_V2_WRITE_SIZE);
    LOG_DEBUG("[%s] Packet length set to %d", device_name.c_str(), pkt_sze);

    /*---------------------------------------------------------*\
    | NB: If the device is not found in the device list         |
    |   then wireless mode may not work reliably                |
    \*---------------------------------------------------------*/
    bool not_found          = true;

    for(uint16_t i = 0; i < CORSAIR_V2_DEVICE_COUNT; i++)
    {
        LOG_DEBUG("[%s] Checking PID %04X against index %d with %04X - %smatch", device_name.c_str(),
                  pid, i, corsair_v2_device_list[i]->pid, corsair_v2_device_list[i]->pid == pid ? "" : "no ");
        if(corsair_v2_device_list[i]->pid == pid)
        {
            /*---------------------------------------------------------*\
            | Set device ID                                             |
            \*---------------------------------------------------------*/
            not_found       = false;
            device_index    = i;
            break;
        }
    }

    if(not_found)
    {
        LOG_ERROR("[%s] device capabilities not found. Please creata a new device request.",
                  device_name.c_str());
    }

    /*---------------------------------------------------------*\
    | Check lighting control endpoints                          |
    |   If lighting control endpoint 2 is unavailable           |
    |   then use endpoint 1.                                    |
    \*---------------------------------------------------------*/
    if(light_ctrl == CORSAIR_V2_LIGHT_CTRL2)
    {
        /*-----------------------------------------------------*\
        | Drain any stale replies first - over a wireless       |
        |   receiver, delayed responses from earlier commands   |
        |   can desync this probe and cause a false fallback    |
        \*-----------------------------------------------------*/
        ClearPacketBuffer();
        result = StartTransaction(0);
        if(result > 0)
        {
            LOG_DEBUG("[%s] Endpoint %02X probe failed (%02X), retrying", device_name.c_str(), light_ctrl, result);
            StopTransaction(0);
            ClearPacketBuffer();
            result = StartTransaction(0);
        }
        if(result > 0)
        {
            light_ctrl = CORSAIR_V2_LIGHT_CTRL1;
            StartTransaction(0);
        }
        StopTransaction(0);
        LOG_DEBUG("[%s] Lighting Endpoint set to %02X", device_name.c_str(), light_ctrl);
    }
}

CorsairPeripheralV2Controller::~CorsairPeripheralV2Controller()
{
    hid_close(dev);
}

const corsair_v2_device* CorsairPeripheralV2Controller::GetDeviceData()
{
    return corsair_v2_device_list[device_index];
}

std::string CorsairPeripheralV2Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string CorsairPeripheralV2Controller::GetErrorString(uint8_t err)
{
    switch(err)
    {
        case 1:
            return "Invalid Value";
        case 3:
            return "Failed";
        case 5:
            return "Unsupported";
        default:
            return "Protocol Error (Unknown)";
    }
}

std::string CorsairPeripheralV2Controller::GetFirmwareString()
{
    return "";
}

std::string CorsairPeripheralV2Controller::GetName()
{
    return device_name;
}

std::string CorsairPeripheralV2Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void CorsairPeripheralV2Controller::SetRenderMode(corsair_v2_device_mode mode)
{
    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];

    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);

    /*---------------------------------------------------------*\
    | Set Mode                                                  |
    \*---------------------------------------------------------*/
    buffer[1]   = write_cmd;
    buffer[2]   = CORSAIR_V2_CMD_SET;
    buffer[3]   = CORSAIR_V2_VALUE_MODE;
    buffer[5]   = mode;

    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);

    if(!skip_reads)
    {
        hid_read_timeout(dev, buffer, CORSAIR_V2_WRITE_SIZE, CORSAIR_V2_TIMEOUT);
    }
}

void CorsairPeripheralV2Controller::LightingControl(uint8_t opt1)
{
    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];

    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);

    /*---------------------------------------------------------*\
    | The Corsair command is the same for each initialisation   |
    |   packet and the registers and options differ for         |
    |   each peripheral supported by the protocol               |
    \*---------------------------------------------------------*/
    buffer[1]   = write_cmd;
    buffer[2]   = CORSAIR_V2_CMD_GET;
    buffer[3]   = opt1;
    buffer[5]   = 0x00;

    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);

    if(!skip_reads)
    {
        hid_read_timeout(dev, buffer, CORSAIR_V2_WRITE_SIZE, CORSAIR_V2_TIMEOUT);
    }
}

unsigned int CorsairPeripheralV2Controller::GetKeyboardLayout()
{
    return GetAddress(0x41);
}

unsigned int CorsairPeripheralV2Controller::GetAddress(uint8_t address)
{
    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];
    uint8_t read[CORSAIR_V2_WRITE_SIZE];

    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);
    memset(read, 0, CORSAIR_V2_WRITE_SIZE);

    buffer[1]   = write_cmd;
    buffer[2]   = CORSAIR_V2_CMD_GET;
    buffer[3]   = address;

    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);
    hid_read_timeout(dev, read, CORSAIR_V2_WRITE_SIZE, CORSAIR_V2_TIMEOUT);

    unsigned int temp = (unsigned int)(read[6] << 24 | read[5] << 16 | read[4] << 8 | read[3]);
    LOG_DEBUG("[%s] GetAddress %02X - %02X %02X - %02X %02X %02X %02X   %02X %02X %02X %02X", device_name.c_str(),
              address, read[0], read[1], read[2], read[3], read[4], read[5], read[6], read[7], read[8], read[9]);

    uint8_t result = read[2];
    if(result > 0)
    {
        LOG_DEBUG("[%s] An error occurred! Get Address %02X failed - %d %s", device_name.c_str(),
                  address, result, GetErrorString(result).c_str());
        return -1;
    }
    return temp;
}

unsigned char CorsairPeripheralV2Controller::StartTransaction(uint8_t opt1)
{
    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];

    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);

    buffer[1]   = write_cmd;
    buffer[2]   = CORSAIR_V2_CMD_START_TX;
    buffer[3]   = opt1;
    buffer[4]   = light_ctrl;

    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);

    if(!skip_reads)
    {
        hid_read_timeout(dev, buffer, CORSAIR_V2_WRITE_SIZE, CORSAIR_V2_TIMEOUT);
    }

    return buffer[2];
}

void CorsairPeripheralV2Controller::StopTransaction(uint8_t opt1)
{
    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];

    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);

    buffer[1]   = write_cmd;
    buffer[2]   = CORSAIR_V2_CMD_STOP_TX;
    buffer[3]   = 0x01;
    buffer[4]   = opt1;

    hid_write(dev, buffer, CORSAIR_V2_WRITE_SIZE);

    if(!skip_reads)
    {
        hid_read_timeout(dev, buffer, CORSAIR_V2_WRITE_SIZE, CORSAIR_V2_TIMEOUT);
    }
}

void CorsairPeripheralV2Controller::ClearPacketBuffer()
{
    if(skip_reads)
    {
        return;
    }

    uint8_t result          = 0;
    uint8_t buffer[CORSAIR_V2_PACKET_SIZE];

    do
    {
        result = hid_read_timeout(dev, buffer, pkt_sze, CORSAIR_V2_TIMEOUT_SHORT);
    }
    while(result > 0);
}

void CorsairPeripheralV2Controller::SetLEDs(uint8_t *data, uint16_t data_size)
{
    const uint8_t offset1   = 8;
    const uint8_t offset2   = 4;
    uint16_t remaining      = data_size;

    uint8_t buffer[CORSAIR_V2_PACKET_SIZE];
    uint8_t read_buf[CORSAIR_V2_PACKET_SIZE];
    memset(buffer, 0, CORSAIR_V2_PACKET_SIZE);

    ClearPacketBuffer();
    unsigned char tx_result = StartTransaction(0);
    if(tx_result != 0)
    {
        /*-----------------------------------------------------*\
        | Handle may be stuck open from a previous session -    |
        |   close it and retry once (mirrors SignalRGB logic)   |
        \*-----------------------------------------------------*/
        LOG_DEBUG("[%s] SetLEDs open handle failed (%02X), closing and retrying", device_name.c_str(), tx_result);
        StopTransaction(0);
        tx_result = StartTransaction(0);
    }
    if(tx_result != 0)
    {
        /*-----------------------------------------------------*\
        | Device may have dropped out of software mode (sleep,  |
        |   power cycle) - re-assert it and retry the open      |
        \*-----------------------------------------------------*/
        LOG_DEBUG("[%s] SetLEDs open still failing - re-asserting SW mode", device_name.c_str());
        SetRenderMode(CORSAIR_V2_MODE_SW);
        ClearPacketBuffer();
        tx_result = StartTransaction(0);
    }
    if(tx_result != 0)
    {
        /*-----------------------------------------------------*\
        | The chosen lighting endpoint may be wrong.  A sleeping|
        |   or re-linking wireless device fails the constructor |
        |   endpoint probe, so light_ctrl can be left on an     |
        |   endpoint the device does not actually support (e.g. |
        |   the K70 Core TKL Wireless only accepts 0x22).  This |
        |   also means the first frames after a wireless        |
        |   reconnect can fail until the link is up - the       |
        |   keepalive thread retries here every update period,  |
        |   so control recovers on its own once the device      |
        |   wakes.  Try the alternate endpoint; if it opens,    |
        |   keep it and skip this frame - the caller formats    |
        |   data for the endpoint, so the next frame will be    |
        |   built correctly for it.                             |
        \*-----------------------------------------------------*/
        uint8_t prev_ctrl   = light_ctrl;
        light_ctrl          = (light_ctrl == CORSAIR_V2_LIGHT_CTRL2) ? CORSAIR_V2_LIGHT_CTRL1 : CORSAIR_V2_LIGHT_CTRL2;
        ClearPacketBuffer();
        tx_result           = StartTransaction(0);
        if(tx_result == 0)
        {
            LOG_DEBUG("[%s] Lighting endpoint switched %02X -> %02X, skipping this frame", device_name.c_str(), prev_ctrl, light_ctrl);
            StopTransaction(0);
            return;
        }
        light_ctrl          = prev_ctrl;
    }

    /*---------------------------------------------------------*\
    | Lighting data packets (0x06/0x07) are addressed with the  |
    |   plain device ID: bit 3 (0x08) is only set on control    |
    |   commands.  Receivers drop data packets with 0x08 set.   |
    \*---------------------------------------------------------*/
    uint8_t data_cmd        = (write_cmd == CORSAIR_V2_WRITE_WIRELESS_ID) ? (CORSAIR_V2_WRITE_WIRELESS_ID & 0x07) : write_cmd;

    /*---------------------------------------------------------*\
    | Set the data header in packet 1 with the data length      |
    |   signaling how many packets to expect to the device      |
    \*---------------------------------------------------------*/
    buffer[1]               = data_cmd;
    buffer[2]               = CORSAIR_V2_CMD_BLK_W1;
    buffer[4]               = data_size & 0xFF;
    buffer[5]               = data_size >> 8;

    /*---------------------------------------------------------*\
    | Check if the data needs more than 1 packet                |
    \*---------------------------------------------------------*/
    uint16_t copy_bytes     = pkt_sze - offset1;
    if(remaining < copy_bytes)
    {
        copy_bytes          = remaining;
    }

    memcpy(&buffer[offset1], &data[0], copy_bytes);

    hid_write(dev, buffer, pkt_sze);

    if(!skip_reads)
    {
        /*-----------------------------------------------------*\
        | Read the ack into a separate buffer - reading into    |
        |   the send buffer corrupts the headers of the         |
        |   continuation packets when a reply arrives           |
        \*-----------------------------------------------------*/
        hid_read_timeout(dev, read_buf, pkt_sze, CORSAIR_V2_TIMEOUT_SHORT);
    }

    remaining              -= copy_bytes;
    buffer[2]               = CORSAIR_V2_CMD_BLK_WN;
    copy_bytes              = pkt_sze - offset2;

    /*---------------------------------------------------------*\
    | Send the remaining packets                                |
    \*---------------------------------------------------------*/
    while(remaining)
    {
        uint16_t index      = data_size - remaining;
        if(remaining < copy_bytes)
        {
            memset(&buffer[offset2], 0, copy_bytes);
            copy_bytes      = remaining;
        }

        memcpy(&buffer[offset2], &data[index], copy_bytes);

        hid_write(dev, buffer, pkt_sze);

        if(!skip_reads)
        {
            hid_read_timeout(dev, read_buf, pkt_sze, CORSAIR_V2_TIMEOUT_SHORT);
        }

        remaining          -= copy_bytes;
    }

    StopTransaction(0);
}

void CorsairPeripheralV2Controller::UpdateHWMode(uint16_t mode, corsair_v2_color /*color_mode*/, uint8_t /*speed*/,
                                                 uint8_t /*direction*/, uint8_t /*brightness*/, std::vector<RGBColor> /*colors*/)
{
    /*---------------------------------------------------------*\
    | If we are switching to `Direct` mode                      |
    |   set device in software mode                             |
    \*---------------------------------------------------------*/
    if(mode == CORSAIR_V2_MODE_DIRECT)
    {
        SetRenderMode(CORSAIR_V2_MODE_SW);
        return;
    }

    /*
    SetRenderMode(CORSAIR_V2_MODE_HW);

    uint8_t buffer[CORSAIR_V2_WRITE_SIZE];
    memset(buffer, 0, CORSAIR_V2_WRITE_SIZE);
    */

    /*---------------------------------------------------------*\
    | Set the data header in packet 1 with the data length      |
    |   signaling how many packets to expect to the device      |
    \*---------------------------------------------------------*/

    /*
    buffer[1]               = write_cmd;
    buffer[2]               = CORSAIR_V2_CMD_BLK_W1;
    buffer[3]               = CORSAIR_V2_MODE_HW;
    buffer[4]               = 0x30;
    buffer[8]               = mode & 0xFF;
    buffer[9]               = mode >> 8;

    buffer[10]              = CORSAIR_V2_COLOR_SPECIFIC;
    buffer[11]              = speed;

    buffer[14]              = colors.size();

    for(size_t i = 0; i < colors.size(); ++i)
    {
        uint8_t offset      = 15 + (i * 4);

        buffer[offset]      = brightness;
        buffer[offset + 1]  = RGBGetBValue(colors[i]);
        buffer[offset + 2]  = RGBGetGValue(colors[i]);
        buffer[offset + 3]  = RGBGetRValue(colors[i]);
    }
    */

}

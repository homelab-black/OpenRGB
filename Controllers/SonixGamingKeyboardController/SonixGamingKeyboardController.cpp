/*---------------------------------------------------------*\
| SonixGamingKeyboardController.cpp                         |
|                                                           |
|   Driver for Sonix based keyboards using the "Gaming      |
|   Force Z" configuration software (CZC.Gaming Revenant)   |
|                                                           |
|   Roman Ondráček                              05 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "LogManager.h"
#include "SonixGamingKeyboardController.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| Delay between packets, taken from the vendor software     |
| configuration (CommandDelayTime)                          |
\*---------------------------------------------------------*/
#define SONIX_GK_COMMAND_DELAY      std::chrono::milliseconds(15)
#define SONIX_GK_RETRIES            5

/*---------------------------------------------------------*\
| The keyboard may still be writing the block to flash when |
| the checksum is first requested, so allow it more time    |
\*---------------------------------------------------------*/
#define SONIX_GK_CHECKSUM_RETRIES   20
#define SONIX_GK_CHECKSUM_DELAY     std::chrono::milliseconds(50)

SonixGamingKeyboardController::SonixGamingKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

SonixGamingKeyboardController::~SonixGamingKeyboardController()
{
    hid_close(dev);
}

std::string SonixGamingKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string SonixGamingKeyboardController::GetNameString()
{
    return(name);
}

std::string SonixGamingKeyboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool SonixGamingKeyboardController::SetLighting
    (
    const sonix_gk_mode_record* records,
    unsigned int                record_count,
    unsigned int                active,
    const RGBColor*             key_colors
    )
{
    unsigned char data[SONIX_GK_LIGHTING_DATA_SIZE];

    memset(data, 0x00, sizeof(data));

    if(record_count > SONIX_GK_MODE_RECORD_COUNT || active >= record_count)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Mode records                                          |
    \*-----------------------------------------------------*/
    for(unsigned int record_idx = 0; record_idx < record_count; record_idx++)
    {
        BuildModeRecord(&data[record_idx * SONIX_GK_MODE_RECORD_SIZE], records[record_idx]);
    }

    /*-----------------------------------------------------*\
    | Per key colors                                        |
    \*-----------------------------------------------------*/
    for(unsigned int key_idx = 0; key_idx < (SONIX_GK_MATRIX_ROWS * SONIX_GK_MATRIX_COLUMNS); key_idx++)
    {
        unsigned char* key = &data[SONIX_GK_KEY_DATA_OFFSET + (key_idx * 4)];

        key[0]  = 0x80;
        key[1]  = RGBGetRValue(key_colors[key_idx]);
        key[2]  = RGBGetGValue(key_colors[key_idx]);
        key[3]  = RGBGetBValue(key_colors[key_idx]);
    }

    /*-----------------------------------------------------*\
    | Active mode record                                    |
    \*-----------------------------------------------------*/
    BuildModeRecord(&data[SONIX_GK_ACTIVE_RECORD_OFFSET], records[active]);

    /*-----------------------------------------------------*\
    | The new settings take effect as soon as the block is  |
    | written.  The vendor software sometimes sends 04 F0   |
    | afterwards, but that makes the keyboard reject the    |
    | next block, so it is not sent here.                   |
    \*-----------------------------------------------------*/
    return(WriteBlock(SONIX_GK_COMMAND_WRITE_LIGHTING, data, sizeof(data)));
}

void SonixGamingKeyboardController::BuildModeRecord(unsigned char* buf, const sonix_gk_mode_record& record)
{
    memset(buf, 0x00, SONIX_GK_MODE_RECORD_SIZE);

    buf[0x00]   = record.mode;
    buf[0x09]   = record.brightness;
    buf[0x0E]   = 0xAA;
    buf[0x0F]   = 0x55;

    if(record.mode != SONIX_GK_MODE_CUSTOM)
    {
        buf[0x01]   = record.red;
        buf[0x02]   = record.green;
        buf[0x03]   = record.blue;
        buf[0x08]   = record.random ? 0x01 : 0x00;
        buf[0x0A]   = record.speed;
        buf[0x0B]   = record.direction;
    }
}

/*---------------------------------------------------------*\
| Private packet functions                                  |
\*---------------------------------------------------------*/
bool SonixGamingKeyboardController::SendPacket(const unsigned char* data)
{
    unsigned char usb_buf[SONIX_GK_PACKET_SIZE + 1];

    usb_buf[0] = 0x00;
    memcpy(&usb_buf[1], data, SONIX_GK_PACKET_SIZE);

    int ret = hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));

    std::this_thread::sleep_for(SONIX_GK_COMMAND_DELAY);

    return(ret > 0);
}

bool SonixGamingKeyboardController::ReadPacket(unsigned char* data)
{
    unsigned char usb_buf[SONIX_GK_PACKET_SIZE + 1];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    int ret = hid_get_feature_report(dev, usb_buf, sizeof(usb_buf));

    if(ret <= 1)
    {
        return(false);
    }

    memcpy(data, &usb_buf[1], SONIX_GK_PACKET_SIZE);

    return(true);
}

bool SonixGamingKeyboardController::SendCommand(unsigned char command)
{
    unsigned char buf[SONIX_GK_PACKET_SIZE];

    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = 0x04;
    buf[0x01]   = command;

    return(SendPacket(buf));
}

bool SonixGamingKeyboardController::WaitForAck(unsigned char* response)
{
    /*-----------------------------------------------------*\
    | The keyboard sets byte 3 of the response to 0x01 when |
    | the previous command was accepted                     |
    \*-----------------------------------------------------*/
    return(ReadPacket(response) && response[0x03] == 0x01);
}

bool SonixGamingKeyboardController::WriteBlock(unsigned char command, const unsigned char* data, unsigned int size)
{
    unsigned char   buf[SONIX_GK_PACKET_SIZE];
    unsigned char   response[SONIX_GK_PACKET_SIZE];
    unsigned int    chunk_count = (size + SONIX_GK_PACKET_SIZE - 1) / SONIX_GK_PACKET_SIZE;
    unsigned int    checksum    = 0;
    bool            acked       = false;

    /*-----------------------------------------------------*\
    | Header: 04 <command> 00 00 00 00 00 00 <chunk count>  |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = 0x04;
    buf[0x01]   = command;
    buf[0x08]   = chunk_count & 0xFF;
    buf[0x09]   = chunk_count >> 8;

    for(unsigned int retry = 0; retry < SONIX_GK_RETRIES && !acked; retry++)
    {
        SendPacket(buf);
        acked = WaitForAck(response);
    }

    if(!acked)
    {
        LOG_WARNING("[SonixGamingKeyboardController] %s: Block 0x%02X header was not acknowledged", name.c_str(), command);
        return(false);
    }

    /*-----------------------------------------------------*\
    | Raw data chunks                                       |
    \*-----------------------------------------------------*/
    for(unsigned int chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++)
    {
        unsigned int offset     = chunk_idx * SONIX_GK_PACKET_SIZE;
        unsigned int chunk_size = size - offset;

        if(chunk_size > SONIX_GK_PACKET_SIZE)
        {
            chunk_size          = SONIX_GK_PACKET_SIZE;
        }

        memset(buf, 0x00, sizeof(buf));
        memcpy(buf, &data[offset], chunk_size);

        for(unsigned int byte_idx = 0; byte_idx < SONIX_GK_PACKET_SIZE; byte_idx++)
        {
            checksum += buf[byte_idx];
        }

        bool sent = false;

        for(unsigned int retry = 0; retry < SONIX_GK_RETRIES && !sent; retry++)
        {
            sent = SendPacket(buf);
        }

        if(!sent)
        {
            LOG_WARNING("[SonixGamingKeyboardController] %s: Failed to send block 0x%02X chunk %u", name.c_str(), command, chunk_idx);
            return(false);
        }
    }

    /*-----------------------------------------------------*\
    | Verify checksum reported by the keyboard              |
    \*-----------------------------------------------------*/
    acked = false;

    for(unsigned int retry = 0; retry < SONIX_GK_CHECKSUM_RETRIES && !acked; retry++)
    {
        if(retry > 0)
        {
            std::this_thread::sleep_for(SONIX_GK_CHECKSUM_DELAY);
        }

        SendCommand(SONIX_GK_COMMAND_GET_CHECKSUM);
        acked = WaitForAck(response);
    }

    if(!acked)
    {
        LOG_WARNING("[SonixGamingKeyboardController] %s: Block 0x%02X checksum was not reported", name.c_str(), command);
        return(false);
    }

    unsigned int device_checksum = response[0x04]
                                 | (response[0x05] << 8)
                                 | (response[0x06] << 16)
                                 | ((unsigned int)response[0x07] << 24);

    if(device_checksum != checksum)
    {
        LOG_WARNING("[SonixGamingKeyboardController] %s: Block 0x%02X checksum mismatch (expected 0x%08X, got 0x%08X)", name.c_str(), command, checksum, device_checksum);
        return(false);
    }

    return(true);
}

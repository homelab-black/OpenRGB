/*---------------------------------------------------------*\
| EVisionKeyboardController.cpp                             |
|                                                           |
|   Driver for EVision keyboard (Redragon, Glorious, Ajazz, |
|   Tecware, and many other brands)                         |
|                                                           |
|   Adam Honse (CalcProgrammer1)                15 Mar 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "EVisionKeyboardController.h"
#include "LogManager.h"
#include "StringUtils.h"

EVisionKeyboardController::EVisionKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

EVisionKeyboardController::~EVisionKeyboardController()
{
    hid_close(dev);
}

std::string EVisionKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string EVisionKeyboardController::GetNameString()
{
    return(name);
}

std::string EVisionKeyboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void EVisionKeyboardController::SetKeyboardColors
    (
    unsigned char *     color_data,
    unsigned int        size
    )
{
    unsigned int    packet_size = 0;
    unsigned int    packet_offset = 0;

    while(size > 0)
    {
        if(size >= EVISION_KB_MAX_PACKET_SIZE)
        {
            packet_size     = EVISION_KB_MAX_PACKET_SIZE;
        }
        else
        {
            packet_size     = size;
        }

        SendKeyboardData
            (
            &color_data[packet_offset],
            packet_size,
            (profile * EVISION_KB_PROFILE_COLOR_SIZE) + packet_offset
            );

        size           -= packet_size;
        packet_offset  += packet_size;
    }
}

void EVisionKeyboardController::SendKeyboardMode
    (
    unsigned char       mode
    )
{
    SendKeyboardParameter((profile * EVISION_KB_PROFILE_PARAMETER_SIZE) + EVISION_KB_PARAMETER_MODE, 1, &mode);
}

int EVisionKeyboardController::ReadActiveProfile()
{
    unsigned char usb_buf[64];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Read Config (0x03) packet, 0x2C bytes from offset 0   |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = EVISION_KB_COMMAND_READ_CONFIG;
    usb_buf[0x04]           = 0x2C;

    ComputeChecksum((char *)usb_buf);

    hid_write(dev, usb_buf, 64);

    /*-----------------------------------------------------*\
    | Reply echoes the command at byte 3; byte 7 is the     |
    | status (0x01 = OK) and data starts at byte 8          |
    \*-----------------------------------------------------*/
    if(hid_read_timeout(dev, usb_buf, 64, 1000) != 64
    || usb_buf[0x03] != EVISION_KB_COMMAND_READ_CONFIG
    || usb_buf[0x07] != 0x01)
    {
        return(-1);
    }

    return(usb_buf[0x08 + 10]);
}

void EVisionKeyboardController::SetProfile(unsigned char profile_index)
{
    profile = profile_index;
}

void EVisionKeyboardController::SetVerifyReplies(bool verify)
{
    verify_replies = verify;
}

void EVisionKeyboardController::Transfer(unsigned char* usb_buf)
{
    if(!verify_replies)
    {
        hid_write(dev, usb_buf, 64);
        hid_read(dev, usb_buf, 64);
        return;
    }

    /*-----------------------------------------------------*\
    | Like the vendor software: the reply must echo the     |
    | command (byte 3) and must not report busy/error in    |
    | byte 7 (0xFF/0xFE), otherwise retry up to 3 times     |
    \*-----------------------------------------------------*/
    unsigned char reply[64];

    if(usb_buf[0x03] == EVISION_KB_COMMAND_END)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for(unsigned int attempt = 0; attempt < 3; attempt++)
    {
        /*-------------------------------------------------*\
        | Drop stale input reports first: replies to other  |
        | applications that have the device open are also   |
        | delivered to this handle                          |
        \*-------------------------------------------------*/
        while(hid_read_timeout(dev, reply, 64, 0) > 0)
        {
        }

        hid_write(dev, usb_buf, 64);

        int bytes_read = hid_read_timeout(dev, reply, 64, 1000);

        if(bytes_read == 64 && reply[0x03] == usb_buf[0x03] && reply[0x07] != 0xFF && reply[0x07] != 0xFE)
        {
            return;
        }

        LOG_DEBUG("[%s] Command 0x%02X at offset 0x%02X%02X not acknowledged (read %d, status 0x%02X), retrying", name.c_str(), usb_buf[0x03], usb_buf[0x06], usb_buf[0x05], bytes_read, reply[0x07]);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void EVisionKeyboardController::SendKeyboardModeEx
    (
    unsigned char       mode,
    unsigned char       brightness,
    unsigned char       speed,
    unsigned char       direction,
    unsigned char       random_flag,
    unsigned char       red,
    unsigned char       green,
    unsigned char       blue
    )
{
    unsigned char parameter_data[8];

    parameter_data[0]   = mode;
    parameter_data[1]   = brightness;
    parameter_data[2]   = speed;
    parameter_data[3]   = direction;
    parameter_data[4]   = random_flag;
    parameter_data[5]   = red;
    parameter_data[6]   = green;
    parameter_data[7]   = blue;

    SendKeyboardParameter(profile * EVISION_KB_PROFILE_PARAMETER_SIZE, 8, parameter_data);
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void EVisionKeyboardController::ComputeChecksum
    (
    char                usb_buf[64]
    )
{
    unsigned short checksum = 0;

    for(unsigned int byte_idx = 0x03; byte_idx < 64; byte_idx++)
    {
        checksum += usb_buf[byte_idx];
    }

    usb_buf[0x01] = checksum & 0xFF;
    usb_buf[0x02] = checksum >> 8;
}

void EVisionKeyboardController::SendKeyboardBegin()
{
    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Keyboard Begin (0x01) packet                   |
    |   Note: Not computing checksum as packet contents are |
    |         fixed                                         |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x01]           = EVISION_KB_COMMAND_BEGIN;
    usb_buf[0x02]           = 0x00;
    usb_buf[0x03]           = EVISION_KB_COMMAND_BEGIN;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    Transfer((unsigned char *)usb_buf);
}

void EVisionKeyboardController::SendKeyboardEnd()
{
    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Keyboard End (0x02) packet                     |
    |   Note: Not computing checksum as packet contents are |
    |         fixed                                         |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x01]           = EVISION_KB_COMMAND_END;
    usb_buf[0x02]           = 0x00;
    usb_buf[0x03]           = EVISION_KB_COMMAND_END;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    Transfer((unsigned char *)usb_buf);
}

void EVisionKeyboardController::SendKeyboardData
    (
    unsigned char *     data,
    unsigned char       data_size,
    unsigned short      data_offset
    )
{
    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Keyboard Color Data (0x11) packet              |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = 0x11;

    usb_buf[0x04]           = data_size;
    usb_buf[0x05]           = data_offset & 0x00FF;
    usb_buf[0x06]           = data_offset >> 8;

    /*-----------------------------------------------------*\
    | Copy in data bytes                                    |
    \*-----------------------------------------------------*/
    memcpy(&usb_buf[0x08], data, data_size);

    /*-----------------------------------------------------*\
    | Compute Checksum                                      |
    \*-----------------------------------------------------*/
    ComputeChecksum(usb_buf);

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    Transfer((unsigned char *)usb_buf);
}

void EVisionKeyboardController::SendKeyboardParameter
    (
    unsigned char       parameter,
    unsigned char       parameter_size,
    unsigned char*      parameter_data
    )
{
    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up Keyboard Parameter (0x06) packet               |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = EVISION_KB_COMMAND_SET_PARAMETER;
    usb_buf[0x04]           = parameter_size;
    usb_buf[0x05]           = parameter;

    /*-----------------------------------------------------*\
    | Copy in data bytes                                    |
    \*-----------------------------------------------------*/
    memcpy(&usb_buf[0x08], parameter_data, parameter_size);

    /*-----------------------------------------------------*\
    | Compute Checksum                                      |
    \*-----------------------------------------------------*/
    ComputeChecksum(usb_buf);

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    Transfer((unsigned char *)usb_buf);
}

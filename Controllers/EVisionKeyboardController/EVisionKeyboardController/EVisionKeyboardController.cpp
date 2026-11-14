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

#include <cstring>
#include "EVisionKeyboardController.h"
#include "StringUtils.h"

EVisionKeyboardController::EVisionKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name, bool direct_supported)
{
    dev                     = dev_handle;
    location                = path;
    name                    = dev_name;
    this->direct_supported  = direct_supported;
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

bool EVisionKeyboardController::SupportsDirect()
{
    return(direct_supported);
}

void EVisionKeyboardController::SetKeyboardColors
    (
    unsigned char *     color_data,
    unsigned int        size
    )
{
    /*-----------------------------------------------------*\
    | Direct-capable 320F:5000 firmware uses a different    |
    | base offset for the persistent 0x11 color region.     |
    \*-----------------------------------------------------*/
    unsigned short data_offset = direct_supported ? 0x0200 : 0x0000;

    SetKeyboardColorsEx
        (
        EVISION_KB_COMMAND_WRITE_CUSTOM_COLOR_DATA,
        data_offset,
        color_data,
        size
        );
}

void EVisionKeyboardController::SetKeyboardColorsDirect
    (
    unsigned char *     color_data,
    unsigned int        size
    )
{
    if(!direct_supported)
    {
        return;
    }

    SetKeyboardColorsEx
        (
        EVISION_KB_COMMAND_SEND_DYNAMIC_COLOR_DATA,
        0x0000,
        color_data,
        size
        );
}

void EVisionKeyboardController::SetKeyboardColorsEx
    (
    unsigned char       command,
    unsigned short      data_offset,
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
            command,
            &color_data[packet_offset],
            packet_size,
            data_offset + packet_offset
            );

        size           -= packet_size;
        packet_offset  += packet_size;
    }
}

void EVisionKeyboardController::SendKeyboardDirectEnd()
{
    if(!direct_supported)
    {
        return;
    }

    char usb_buf[64];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up End Dynamic Color (0x13) packet                |
    \*-----------------------------------------------------*/
    usb_buf[0x00] = 0x04;
    usb_buf[0x03] = EVISION_KB_COMMAND_END_DYNAMIC_COLOR_DATA;

    ComputeChecksum(usb_buf);

    /*-----------------------------------------------------*\
    | Send packet.  0104 does not ACK 0x13; use a short     |
    | timed read so ACK-capable revisions still drain.      |
    \*-----------------------------------------------------*/
    hid_write(dev, (unsigned char *)usb_buf, 64);
    hid_read_timeout(dev, (unsigned char *)usb_buf, 64, 10);
}

void EVisionKeyboardController::SendKeyboardMode
    (
    unsigned char       mode
    )
{
    SendKeyboardParameter(EVISION_KB_PARAMETER_MODE, 1, &mode);
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

    SendKeyboardParameter(0, 8, parameter_data);
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
    hid_write(dev, (unsigned char *)usb_buf, 64);
    hid_read(dev, (unsigned char *)usb_buf, 64);
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
    hid_write(dev, (unsigned char *)usb_buf, 64);
    hid_read(dev, (unsigned char *)usb_buf, 64);
}

void EVisionKeyboardController::SendKeyboardData
    (
    unsigned char       command,
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
    | Set up Keyboard Color Data packet                     |
    \*-----------------------------------------------------*/
    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = command;

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
    hid_write(dev, (unsigned char *)usb_buf, 64);

    /*-----------------------------------------------------*\
    | Command 0x12 (and 0x13) may not ACK on bcdDevice      |
    | 0x0104.  A blocking hid_read stalls after the first   |
    | 54-byte chunk.  Use a short timed read so ACK-capable |
    | revisions (e.g. 0x0103) still drain their reply.      |
    \*-----------------------------------------------------*/
    if(command == EVISION_KB_COMMAND_SEND_DYNAMIC_COLOR_DATA)
    {
        hid_read_timeout(dev, (unsigned char *)usb_buf, 64, 10);
    }
    else
    {
        hid_read(dev, (unsigned char *)usb_buf, 64);
    }
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
    hid_write(dev, (unsigned char *)usb_buf, 64);
    hid_read(dev, (unsigned char *)usb_buf, 64);
}

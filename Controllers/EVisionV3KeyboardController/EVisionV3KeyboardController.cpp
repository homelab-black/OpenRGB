/*---------------------------------------------------------*\
| EVisionV3KeyboardController.cpp                           |
|                                                           |
|   Driver for EVision "RDR" tri-mode keyboards (VIA)       |
|                                                           |
|   Jan Imrich (imrija)                         05 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string.h>
#include "EVisionV3KeyboardController.h"
#include "QMKViaCommands.h"
#include "StringUtils.h"

EVisionV3KeyboardController::EVisionV3KeyboardController(hid_device* dev_handle, const char* path)
{
    dev                     = dev_handle;
    location                = path;
    via_protocol_version    = 0;

    /*-----------------------------------------------------*\
    | Read product string                                   |
    \*-----------------------------------------------------*/
    wchar_t product_string[256];

    if(hid_get_product_string(dev, product_string, 256) == 0)
    {
        name = StringUtils::wstring_to_string(product_string);
    }

    /*-----------------------------------------------------*\
    | Read serial string                                    |
    \*-----------------------------------------------------*/
    wchar_t serial_string[256];

    if(hid_get_serial_number_string(dev, serial_string, 256) == 0)
    {
        serial = StringUtils::wstring_to_string(serial_string);
    }

    /*-----------------------------------------------------*\
    | Get VIA protocol version.  The RDR firmware answers   |
    | this on the 0xFF60 raw HID interface; devices which   |
    | do not answer are not supported.                      |
    \*-----------------------------------------------------*/
    CmdGetViaProtocolVersion(&via_protocol_version);
}

EVisionV3KeyboardController::~EVisionV3KeyboardController()
{
    hid_close(dev);
}

std::string EVisionV3KeyboardController::GetLocation()
{
    return("HID: " + location);
}

std::string EVisionV3KeyboardController::GetName()
{
    return(name);
}

std::string EVisionV3KeyboardController::GetSerial()
{
    return(serial);
}

std::string EVisionV3KeyboardController::GetVersion()
{
    return("VIA: " + std::to_string(via_protocol_version));
}

bool EVisionV3KeyboardController::GetSupported()
{
    return(via_protocol_version != 0);
}

unsigned char EVisionV3KeyboardController::GetBrightness()
{
    return(CmdGetRGBMatrixValue(QMK_VIA_RGB_MATRIX_BRIGHTNESS));
}

unsigned char EVisionV3KeyboardController::GetEffect()
{
    return(CmdGetRGBMatrixValue(QMK_VIA_RGB_MATRIX_EFFECT));
}

unsigned char EVisionV3KeyboardController::GetSpeed()
{
    return(CmdGetRGBMatrixValue(QMK_VIA_RGB_MATRIX_EFFECT_SPEED));
}

void EVisionV3KeyboardController::GetColor(unsigned char* hue, unsigned char* sat)
{
    *hue = CmdGetRGBMatrixValue(QMK_VIA_RGB_MATRIX_COLOR, sat);
}

void EVisionV3KeyboardController::SetMode
    (
    unsigned char       mode,
    unsigned char       speed,
    unsigned char       hue,
    unsigned char       sat,
    unsigned char       brightness,
    bool                set_color
    )
{
    unsigned char effect[1]     = { mode };
    unsigned char color[2]      = { hue, sat };
    unsigned char speed_val[1]  = { speed };
    unsigned char bright[1]     = { brightness };

    CmdSetRGBMatrixValue(QMK_VIA_RGB_MATRIX_EFFECT, effect, sizeof(effect));

    if(set_color)
    {
        CmdSetRGBMatrixValue(QMK_VIA_RGB_MATRIX_COLOR, color, sizeof(color));
    }

    CmdSetRGBMatrixValue(QMK_VIA_RGB_MATRIX_EFFECT_SPEED, speed_val, sizeof(speed_val));
    CmdSetRGBMatrixValue(QMK_VIA_RGB_MATRIX_BRIGHTNESS, bright, sizeof(bright));
}

void EVisionV3KeyboardController::SaveMode()
{
    unsigned char args[1] = { QMK_VIA_RGB_MATRIX_CHANNEL };

    ViaSendCommand(QMK_VIA_CMD_CUSTOM_SAVE, args, sizeof(args), NULL, 0);
}

unsigned char EVisionV3KeyboardController::CmdGetRGBMatrixValue
    (
    unsigned char       property,
    unsigned char*      extra
    )
{
    unsigned char args[2];
    unsigned char response[5];

    args[0] = QMK_VIA_RGB_MATRIX_CHANNEL;
    args[1] = property;

    memset(response, 0, sizeof(response));

    /*-----------------------------------------------------*\
    | Response layout: cmd, channel, property, value(s)     |
    \*-----------------------------------------------------*/
    ViaSendCommand(QMK_VIA_CMD_CUSTOM_GET_VALUE, args, sizeof(args), response, sizeof(response));

    if(extra != NULL)
    {
        *extra = response[4];
    }

    return(response[3]);
}

void EVisionV3KeyboardController::CmdSetRGBMatrixValue
    (
    unsigned char       property,
    unsigned char*      values,
    unsigned char       values_size
    )
{
    unsigned char args[EVISION_V3_PACKET_SIZE - 1];

    memset(args, 0, sizeof(args));

    args[0] = QMK_VIA_RGB_MATRIX_CHANNEL;
    args[1] = property;
    memcpy(&args[2], values, values_size);

    ViaSendCommand(QMK_VIA_CMD_CUSTOM_SET_VALUE, args, 2 + values_size, NULL, 0);
}

void EVisionV3KeyboardController::CmdGetViaProtocolVersion
    (
    unsigned short*     version
    )
{
    unsigned char response[3];

    memset(response, 0, sizeof(response));

    if(ViaSendCommand(QMK_VIA_CMD_GET_PROTOCOL_VERSION, NULL, 0, response, sizeof(response)) < 0)
    {
        *version = 0;
        return;
    }

    *version = (response[1] << 8) | response[2];
}

int EVisionV3KeyboardController::ViaSendCommand
    (
    unsigned char       cmd,
    unsigned char*      data_in,
    unsigned char       data_in_size,
    unsigned char*      data_out,
    unsigned char       data_out_size
    )
{
    /*-----------------------------------------------------*\
    | Standard VIA command                                  |
    |                                                       |
    | Byte 0: Command                                       |
    | Byte 1+: Data                                         |
    |                                                       |
    | The device echoes the request header in its response. |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[EVISION_V3_PACKET_SIZE + 1];

    memset(usb_buf, 0, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Write command, offsetting by 1 for HID report ID      |
    \*-----------------------------------------------------*/
    usb_buf[1] = cmd;

    if(data_in != NULL)
    {
        memcpy(&usb_buf[2], data_in, data_in_size);
    }

    hid_write(dev, usb_buf, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Read response                                         |
    \*-----------------------------------------------------*/
    int bytes_received = hid_read_timeout(dev, usb_buf, sizeof(usb_buf) - 1, 1000);

    if(bytes_received <= 0 || usb_buf[0] != cmd)
    {
        return(-1);
    }

    if(data_out != NULL)
    {
        memcpy(data_out, &usb_buf[0], data_out_size);
    }

    return(bytes_received);
}

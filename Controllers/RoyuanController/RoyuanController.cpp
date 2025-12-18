/*---------------------------------------------------------*\
| RoyuanController.cpp                                      |
|                                                           |
|   Driver for Royuan keyboard                              |
|                                                           |
|   Alvaro Munoz (alvaromunoz)                  05 Jun 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <math.h>
#include "RoyuanController.h"
#include "LogManager.h"
#include "StringUtils.h"

RoyuanController::RoyuanController(hid_device* dev_handle, hid_device_info* dev_info, std::string dev_name) :
    current_mode(ROYUAN_MODE_ALWAYS_ON),
    current_speed(ROYUAN_SPEED_DEFAULT),
    current_brightness(ROYUAN_BRIGHTNESS_DEFAULT),
    current_dazzle(ROYUAN_OPTION_DAZZLE_OFF),
    current_option(ROYUAN_OPTION_DEFAULT)
{
    dev             = dev_handle;
    location        = dev_info->path;
    name            = dev_name;
    version         = dev_info->release_number;
    serial          = StringUtils::wstring_to_string(dev_info->serial_number);

    wchar_t vendor_string[128];
    hid_get_indexed_string(dev, 1, vendor_string, 128);
    vendor = StringUtils::wstring_to_string(vendor_string);

    wchar_t product_string[128];
    hid_get_indexed_string(dev, 2, product_string, 128);
    product = StringUtils::wstring_to_string(product_string);
}

RoyuanController::~RoyuanController()
{
    hid_close(dev);
}

std::string RoyuanController::GetLocation()
{
    return("HID: " + location);
}

std::string RoyuanController::GetName()
{
    /*---------------------------------------------------------*\
    | Get device name from HID product string                  |
    \*---------------------------------------------------------*/
    return(vendor + " " + product);
}

std::string RoyuanController::GetVendor()
{
    return(vendor);
}

std::string RoyuanController::GetSerialNumber()
{
    return(serial);
}

std::string RoyuanController::GetFirmwareVersion()
{
    return std::to_string(version);
}

std::string RoyuanController::GetDescription()
{
    return "";
}

void RoyuanController::SetMode(unsigned char mode, unsigned char speed, unsigned char brightness)
{
    current_mode  = mode;
    current_speed = speed;
    current_brightness = brightness;

    SendUpdate();
}

void RoyuanController::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    current_red   = red;
    current_green = green;
    current_blue  = blue;

    SendUpdate();
}

void RoyuanController::SetDazzle(bool is_dazzle)
{
    if(is_dazzle)
    {
        current_dazzle = ROYUAN_OPTION_DAZZLE_ON;
    }
    else
    {
        current_dazzle = ROYUAN_OPTION_DAZZLE_OFF;
    }
}

void RoyuanController::SetOption(unsigned char option)
{
    current_option = option;
}

unsigned char RoyuanController::Crc(const unsigned char data[], unsigned int start, unsigned int end)
{
    int sum = 0;
    for(unsigned int i = start; i < end; i++)
    {
        sum += data[i];
    }

    // Find the smallest power of two that is greater than or equal to the sum
    int next_pow2 = (int)(pow(2, ceil(log2((double)(sum)))));
    int crc = next_pow2 - sum - 1;

    return crc;
}

void RoyuanController::SendUpdate()
{
    unsigned char buffer[ROYUAN_PACKET_LENGTH + 1] = { 0x00 };

    buffer[ROYUAN_BYTE_COMMAND]    = ROYUAN_COMMAND_RGB;
    buffer[ROYUAN_BYTE_MODE]       = current_mode;
    buffer[ROYUAN_BYTE_SPEED]      = current_speed;
    buffer[ROYUAN_BYTE_BRIGHTNESS] = current_brightness;
    buffer[ROYUAN_BYTE_FLAGS]      = (current_option << 4) | (current_dazzle & 0xF);
    buffer[ROYUAN_BYTE_RED]        = current_red;
    buffer[ROYUAN_BYTE_GREEN]      = current_green;
    buffer[ROYUAN_BYTE_BLUE]       = current_blue;

    buffer[ROYUAN_BYTE_CRC]        = Crc(buffer, 0, ROYUAN_BYTE_CRC);

    int send_buffer_result = hid_send_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(send_buffer_result<0)
    {
        LOG_ERROR("[%s]: Send Buffer Error. HIDAPI Error: %ls", name, hid_error(dev));
    }
}

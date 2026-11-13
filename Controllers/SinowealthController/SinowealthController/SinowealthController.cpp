/*---------------------------------------------------------*\
| SinowealthController.cpp                                  |
|                                                           |
|   Driver for Sinowealth mice, including Glorious          |
|                                                           |
|   Niels Westphal (crashniels)                 20 May 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "LogManager.h"
#include "SinowealthController.h"
#include "StringUtils.h"
#include "RGBControllerInterface.h" // for RGBColor

SinowealthController::SinowealthController(hid_device* dev_data_handle, hid_device* dev_cmd_handle, char *_path, const std::string& dev_name)
{
    dev_data            = dev_data_handle;
    dev_cmd             = dev_cmd_handle;
    location            = _path;
    name                = dev_name;

    breathing_colors_limit = 7; // Empirical value!
}

SinowealthController::~SinowealthController()
{
    hid_close(dev_data);

    /*---------------------------------------------------------------------*\
    | If the dev_cmd handle was passed in as the same device as dev_data    |
    |   then attempting to close it a second time will segfault             |
    \*---------------------------------------------------------------------*/
    if(dev_cmd && dev_cmd != dev_data)
    {
        hid_close(dev_cmd);
    }
}

unsigned int SinowealthController::GetPresetCount()
{
    return 6;
}

std::string SinowealthController::GetLocation()
{
    return("HID: " + location);
}

std::string SinowealthController::GetName()
{
    return(name);
}

unsigned int SinowealthController::getBreathingLimit()
{
    return breathing_colors_limit; // Expected 7
}

std::string SinowealthController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev_cmd, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string SinowealthController::GetFirmwareVersion()
{
    unsigned char usb_buf[SINOWEALTH_COMMAND_REPORT_SIZE + 1]; // Additional byte for null-terminator
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0] = 5;
    usb_buf[1] = 1;

    int ret = hid_send_feature_report(dev_cmd, usb_buf, SINOWEALTH_COMMAND_REPORT_SIZE);
    if(ret < 0) return("");

    usb_buf[1] = 0;
    ret = hid_get_feature_report(dev_cmd, usb_buf, SINOWEALTH_COMMAND_REPORT_SIZE);
    if(ret < 0) return("");

    return std::string(reinterpret_cast<char*>(usb_buf) + 2); // Skip report and command byte
}

void SinowealthController::setMode
(
        unsigned char   mode,
        unsigned char   speed,
        unsigned char   brightness,
        unsigned char   direction,
        RGBColor*       color_buf
        )
{
    if(!ensureProfileValid())
    {
        return;
    }

    unsigned int color_index = 0;
    uint32_t out_color = 0;

    /*------------------------------------------*\
    | Mode index                                 |
    \*------------------------------------------*/
    device_configuration[0x35] = mode;

    RGBColor* mode_colors = color_buf;
    int color_limit = 7;

    switch (mode)
    {
    case GLORIOUS_MODE_RAINBOW:
        device_configuration[0x36] = ((brightness & 0xF) << 4) | (speed & 0xF);
        device_configuration[0x37] = direction;
        break;
    case GLORIOUS_MODE_STATIC_SINGLE:
    {
        device_configuration[0x38] = ((brightness & 0xF) << 4);
        if(color_limit > 0)
        {
            //out_color = reorderColor(mode_colors[0]);
            out_color = mode_colors[0];
        }
        device_configuration[0x39] = RGBGetRValue(out_color);
        device_configuration[0x3A] = RGBGetGValue(out_color);
        device_configuration[0x3B] = RGBGetBValue(out_color);
        break;
    }
    case GLORIOUS_MODE_SPECTRUM_BREATHING:
    {
        device_configuration[0x3C] = ((brightness & 0xF) << 4) | (speed & 0xF);
        device_configuration[0x3D] = 0x07; //color_limit;  //maybe some kind of bank change?+ Could be number of colors in mode
        for(color_index = 0; color_index < 7; ++color_index)
        {
            int offset = 0x3E + color_index * 3;
            out_color = 0;
            if(int(color_index) < color_limit)
            {
                //out_color = reorderColor(mode_colors[color_index]);
                out_color = mode_colors[color_index];
            }
            device_configuration[offset    ] = RGBGetRValue(out_color);
            device_configuration[offset + 1] = RGBGetGValue(out_color);
            device_configuration[offset + 2] = RGBGetBValue(out_color);
        }
        break;
    }
    case GLORIOUS_MODE_TAIL:
        device_configuration[0x53] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_SPECTRUM_CYCLE:
        device_configuration[0x54] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_STATIC:
    {
        device_configuration[0x55] = ((brightness & 0xF) << 4);
        // NOTE: Technically allows up to 10 colors, but the tested device only had 4
        for(int color_index = 0; color_index < 4; ++color_index)
        {
            int offset = 0x56 + color_index * 3;
            out_color = 0;
            if(color_index < color_limit)
            {
                out_color = mode_colors[color_index];
                //out_color = reorderColor(mode_colors[color_index]);
            }
            device_configuration[offset    ] = RGBGetRValue(out_color);
            device_configuration[offset + 1] = RGBGetGValue(out_color);
            device_configuration[offset + 2] = RGBGetBValue(out_color);
        }
        break;
    }
    case GLORIOUS_MODE_RAVE:
    {
        device_configuration[0x74] = ((brightness & 0xF) << 4) | (speed & 0xF);
        for(int color_index = 0; color_index < 2; ++color_index)
        {
            int offset = 0x75 + color_index * 3;
            out_color = 0;
            if(color_index < color_limit)
            {
                out_color = mode_colors[color_index];
            }
            device_configuration[offset    ] = RGBGetRValue(out_color);
            device_configuration[offset + 1] = RGBGetGValue(out_color);
            device_configuration[offset + 2] = RGBGetBValue(out_color);
        }
        break;
    }
    case GLORIOUS_MODE_WAVE:
        device_configuration[0x7C] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_BREATHING:
    {
        device_configuration[0x7D] = ((brightness & 0xF) << 4) | (speed & 0xF);
        out_color = 0;
        if(color_limit > 0)
        {
            out_color = mode_colors[0];
        }
        device_configuration[0x7E] = RGBGetRValue(out_color);
        device_configuration[0x7F] = RGBGetGValue(out_color);
        device_configuration[0x80] = RGBGetBValue(out_color);
    }
    case GLORIOUS_MODE_OFF:
        device_configuration[0x81] = 0x00; //mode 0 either 0x00 or 0x03 - meaning unknown
        break;
    default:
        break;
    }
}

void SinowealthController::setDpiData(unsigned char activeDpiPreset, unsigned char dpiMask, unsigned char* dpiValues, RGBColor* dpiColors)
{
    if(!ensureProfileValid())
    {
        return;
    }

    if((dpiMask | 0xC0) == 0xFF)
    {
        // mandatory check - if you send 0xFF, 0x7F or similar - you'll brick your mouse!
        dpiMask = 0xC0;
    }

    device_configuration[0x0C] = dpiMask; // DPI mask (0xC0 = enable all)

    //memcpy(&device_configuration[0x0D], dpiValues, 6);

    unsigned char t = device_configuration[0x0B] & 0x0F; // expected to be 0x06
    device_configuration[0x0B] = (activeDpiPreset & 0x0F) << 4 | t; // 0x16, 0x26, 0x36... etc

    for(int i = 0; i < 6; ++i)
    {
        device_configuration[0x0D   + 1] = dpiValues[i];

        int offset = 0x1C + i * 3;
        device_configuration[offset    ] = RGBGetRValue( dpiColors[i]);
        device_configuration[offset + 1] = RGBGetGValue( dpiColors[i]);
        device_configuration[offset + 2] = RGBGetBValue( dpiColors[i]);
    }


    //device_configuration[0x17] = 0x6D;
    //device_configuration[0x19] = 0x9F;
}

int SinowealthController::requestProfile()
{
    int actual;
    unsigned char usb_buf[SINOWEALTH_COMMAND_REPORT_SIZE];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0] = 0x05;
    usb_buf[1] = 0x11;

    actual = hid_send_feature_report(dev_cmd, usb_buf, sizeof(usb_buf));

    if (actual != SINOWEALTH_COMMAND_REPORT_SIZE)
    {
        LOG_ERROR("[Sinowealth Mouse] Error sending read request!");
        return -1;
    }
    else
    {
        // Make sure buffer is correct size and zeroed out
        device_configuration.clear();
        device_configuration.resize(SINOWEALTH_CONFIG_REPORT_SIZE, 0);
        device_configuration[0] = 0x04;

        actual = hid_get_feature_report(dev_data, &device_configuration[0], SINOWEALTH_CONFIG_REPORT_SIZE);

        if (actual < 0)
        {
            LOG_ERROR("[Sinowealth Mouse] Error reading device configuration!");
        }
    }

    unsigned int sensor_id = device_configuration[0x09] & 0x3F;
    if(sensor_id != 0x0E)
    {
        LOG_WARNING("[Sinowealth Mouse] Mouse reported corrupted sensor ID! Received 0x%02x, overriding", sensor_id);
        device_configuration[0x09] = 0x0E;
    }
    return actual;
}

void SinowealthController::sendProfile()
{
    unsigned char usb_buf[SINOWEALTH_CONFIG_REPORT_SIZE];
    memcpy(usb_buf, &device_configuration[0], SINOWEALTH_CONFIG_SIZE);

    /*--------------------------------------------*\
    | Set the command bytes to write to the device |
    \*--------------------------------------------*/
    usb_buf[0x00] = 0x04; // Report ID

    //usb_buf[0x01] = 0x11; // 0x11 for one internal preset, 0x21 for the other
    usb_buf[0x02] = 0x00;
    usb_buf[0x03] = 0x7B; // Write command

    usb_buf[0x06] = 0x00; // Has to be 0x00 or the mouse bricks

    // usb_buf[0x07] = 0x00;
    //usb_buf[0x09] = 0x0e; // Sensor ID!!! HAS to stay the same or the mouse WILL brick
    //usb_buf[0x0A] = 0x03; // Poll rate; 01 = 125, 02 = 250, 03 = 500 Hz (preferred), 04 = 1000, DO NOT SET ANYTHING ELSE
    //usb_buf[0x0B] = 0x11; // or 16 - unknown

    hid_send_feature_report(dev_data, usb_buf, SINOWEALTH_CONFIG_REPORT_SIZE);
}

bool SinowealthController::isProfileValid() const
{
    if(device_configuration.size() < SINOWEALTH_CONFIG_SIZE_MIN)
    {
        return false;
    }
    if(device_configuration[0x01] == 0x00)
    {
        return false;
    }
    return true;
}

bool SinowealthController::ensureProfileValid()
{
    if(!isProfileValid())
    {
        if(requestProfile() < SINOWEALTH_CONFIG_SIZE_MIN)
        {
            return false;
        }
    }
    // Internal profile ID, could be FLASH bank change
    if(device_configuration[0x01] == 0x00)
    {
        //device_configuration[0x01] = 0x11;
    }
    // Sensor ID
    if(device_configuration[0x09] != 0x0E)
    {
        device_configuration[0x09] = 0x0E;
    }
    // DPI mask
    // Only checks that not all 6 are disabled
    if((device_configuration[0x0C] | 0xC0) == 0xFF)
    {
        device_configuration[0x0C] = 0xC0;
    }
    // NOTE: we do not replace DPI here

    return true;
}

/*
static int default_key_preset[] =
{

    GLORIOUS_KEY_ASSIGNMENT_LMB,
    GLORIOUS_KEY_ASSIGNMENT_RMB,
    GLORIOUS_KEY_ASSIGNMENT_WHEEL,
    GLORIOUS_KEY_ASSIGNMENT_FORWARD,
    GLORIOUS_KEY_ASSIGNMENT_BACK,
    GLORIOUS_KEY_ASSIGNMENT_DPI,
    GLORIOUS_KEY_ASSIGNMENT_RGB,

    GLORIOUS_KEY_ASSIGNMENT_DPI_LOCK + 0x04, // 500 DPI
    GLORIOUS_KEY_ASSIGNMENT_SCROLL_UP,
    GLORIOUS_KEY_ASSIGNMENT_SCROLL_DOWN,
    GLORIOUS_KEY_ASSIGNMENT_MODE
}

void SinowealthController::SendKeyPreset()
{
    unsigned char usb_buf[SINOWEALTH_CONFIG_REPORT_SIZE];
    memset(usb_buf, 0, SINOWEALTH_CONFIG_SIZE); // Yes, we only copy 167 bytes back, for now - if anything weird starts happening use SINOWEALTH_CONFIG_REPORT_SIZE

    usb_buf[0x00] = 04;
    usb_buf[0x01] = 12; // Can be 11 and 12
    usb_buf[0x02] = 00;
    usb_buf[0x03] = 0x50; // "Set keys" command

    for(int i = 0; i < 20; ++i)
    {
        int offset = 0x08 + i * 4;
        int key = GLORIOUS_KEY_ASSIGNMENT_NONE;
        if(i < keys.size())
        {
            key = keys[i];
        }
        usb_buf[offset    ] = (key >> 8) & 0xFF;
        usb_buf[offset + 1] = (key     ) & 0xFF;
    }

    hid_send_feature_report(dev_data, usb_buf, SINOWEALTH_CONFIG_REPORT_SIZE);
}
*/

std::vector<unsigned char> SinowealthController::GetRawConfig()
{
    // Makes a copy
    return device_configuration;
}

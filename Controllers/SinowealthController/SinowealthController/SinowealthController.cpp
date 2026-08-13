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

#include <cstring>
#include "LogManager.h"
#include "SinowealthController.h"
#include "StringUtils.h"

SinowealthController::SinowealthController(hid_device* dev_data_handle, hid_device* dev_cmd_handle, char *_path, std::string dev_name, sinowealth_variant dev_variant)
{
    dev_data            = dev_data_handle;
    dev_cmd             = dev_cmd_handle;
    location            = _path;
    name                = dev_name;
    variant             = dev_variant;

    /*---------------------------------------------------------*\
    | The GXT 970 drives its logo and DPI indicator from the DPI  |
    | stage colors, independently of the bottom strip             |
    \*---------------------------------------------------------*/
    led_count           = (variant == SINOWEALTH_VARIANT_TRUST_GXT970) ? 2 : 1;
}

SinowealthController::~SinowealthController()
{
    hid_close(dev_data);

    /*---------------------------------------------------------------------*\
    | If the dev_cmd handle was passed in as the same device as dev_data    |
    |   then attempting to close it a second time will segfault             |
    \*---------------------------------------------------------------------*/
    if(dev_cmd)
    {
        hid_close(dev_cmd);
    }
}

std::string SinowealthController::GetLocation()
{
    return("HID: " + location);
}

std::string SinowealthController::GetName()
{
    return(name);
}

unsigned int SinowealthController::GetLEDCount()
{
    return(led_count);
}

std::string SinowealthController::GetLEDName(unsigned int led_idx)
{
    if(variant != SINOWEALTH_VARIANT_TRUST_GXT970)
    {
        return("Mouse LED");
    }

    return(led_idx == SINOWEALTH_LED_DPI ? "Logo / DPI Indicator" : "Bottom");
}

/*---------------------------------------------------------*\
| Most variants expect red, blue, green in the packet, the   |
| GXT 970 expects plain red, green, blue                     |
\*---------------------------------------------------------*/
void SinowealthController::SetPacketColor(unsigned char* usb_buf, unsigned int offset, RGBColor color)
{
    bool swapped        = (variant != SINOWEALTH_VARIANT_TRUST_GXT970);

    usb_buf[offset]     = RGBGetRValue(color);
    usb_buf[offset + 1] = swapped ? RGBGetBValue(color) : RGBGetGValue(color);
    usb_buf[offset + 2] = swapped ? RGBGetGValue(color) : RGBGetBValue(color);
}

bool SinowealthController::RequiresStartupRefresh()
{
    return(variant == SINOWEALTH_VARIANT_TRUST_GXT970);
}

unsigned char SinowealthController::GetConfigByte(unsigned int offset)
{
    if(offset >= SINOWEALTH_CONFIG_REPORT_SIZE)
    {
        return(0x00);
    }

    return(device_configuration[offset]);
}

RGBColor SinowealthController::GetConfigColor(unsigned int offset)
{
    if(offset + 2 >= SINOWEALTH_CONFIG_REPORT_SIZE)
    {
        return(0);
    }

    /*-----------------------------------------------------*\
    | ToRGBColor() does not parenthesize its arguments, so   |
    | they have to be plain values rather than expressions   |
    \*-----------------------------------------------------*/
    bool          swapped = (variant != SINOWEALTH_VARIANT_TRUST_GXT970);
    unsigned char red     = device_configuration[offset];
    unsigned char green   = swapped ? device_configuration[offset + 2] : device_configuration[offset + 1];
    unsigned char blue    = swapped ? device_configuration[offset + 1] : device_configuration[offset + 2];

    return(ToRGBColor(red, green, blue));
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

void SinowealthController::SetMode
    (
    unsigned char   mode,
    unsigned char   speed,
    unsigned char   brightness,
    unsigned char   direction,
    RGBColor*       color_buf
    )
{
    if (GetProfile() < SINOWEALTH_CONFIG_SIZE_MIN) return;

    unsigned char usb_buf[SINOWEALTH_CONFIG_REPORT_SIZE];
    memcpy(usb_buf, device_configuration, SINOWEALTH_CONFIG_SIZE); // Yes, we only copy 167 bytes back, for now - if anything weird starts happening use SINOWEALTH_CONFIG_REPORT_SIZE

    usb_buf[0x03] = 0x7B;   //write to device
    usb_buf[0x06] = 0x00;

    usb_buf[0x35] = mode;

    switch (mode)
    {
    case GLORIOUS_MODE_RAINBOW:
        usb_buf[0x36] = ((brightness & 0xF) << 4) | (speed & 0xF);
        usb_buf[0x37] = direction;
        break;
    case GLORIOUS_MODE_STATIC:
        usb_buf[0x38] = ((brightness & 0xF) << 4);
        SetPacketColor(usb_buf, 0x39, color_buf[SINOWEALTH_LED_EFFECT]);

        /*-------------------------------------------------------*\
        | Static is the only per LED mode, so it is the only one   |
        | where color_buf holds an entry for the logo.             |
        \*-------------------------------------------------------*/
        if(led_count > SINOWEALTH_LED_DPI)
        {
            for(unsigned int stage = 0; stage < SINOWEALTH_DPI_STAGES; stage++)
            {
                SetPacketColor(usb_buf, SINOWEALTH_DPI_COLOR_OFFSET + (stage * 3), color_buf[SINOWEALTH_LED_DPI]);
            }
        }
        break;
    case GLORIOUS_MODE_SPECTRUM_BREATING:
        usb_buf[0x3C] = ((brightness & 0xF) << 4) | (speed & 0xF);
        usb_buf[0x3D] = 0x07;  //maybe some kind of bank change?+
        //usb_buf[0x3D] = 0x06;
        for(unsigned int color_idx = 0; color_idx < 7; color_idx++)
        {
            SetPacketColor(usb_buf, 0x3E + (color_idx * 3), color_buf[color_idx]);
        }
        break;
    case GLORIOUS_MODE_TAIL:
        usb_buf[0x53] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_SPECTRUM_CYCLE:
        usb_buf[0x54] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_RAVE:
        usb_buf[0x74] = ((brightness & 0xF) << 4) | (speed & 0xF);
        SetPacketColor(usb_buf, 0x75, color_buf[0]);
        SetPacketColor(usb_buf, 0x78, color_buf[1]);
        break;
    case GLORIOUS_MODE_WAVE:
        usb_buf[0x7C] = ((brightness & 0xF) << 4) | (speed & 0xF);
        break;
    case GLORIOUS_MODE_BREATHING:
        usb_buf[0x7D] = ((brightness & 0xF) << 4) | (speed & 0xF);
        SetPacketColor(usb_buf, 0x7E, color_buf[0]);
    case GLORIOUS_MODE_OFF:
        usb_buf[0x81] = 0x00; //mode 0 either 0x00 or 0x03
        break;
    default:
        break;
    }

    hid_send_feature_report(dev_data, usb_buf, SINOWEALTH_CONFIG_REPORT_SIZE);
}

int SinowealthController::GetProfile()
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
        memset(device_configuration, 0x00, SINOWEALTH_CONFIG_REPORT_SIZE);
        device_configuration[0] = 0x04;

        actual = hid_get_feature_report(dev_data, device_configuration, SINOWEALTH_CONFIG_REPORT_SIZE);

        if (actual < 0)
        {
            LOG_ERROR("[Sinowealth Mouse] Error reading device configuration!");
        }
    }

    return actual;
}

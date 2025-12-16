/*---------------------------------------------------------*\
| YunziiYZ75KeyboardController.cpp                          |
|                                                           |
|   Driver for Yunzii YZ75 keyboard                         |
|                                                           |
|   Kosta A. (kostaarvanitis)                   23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <cmath>
#include "LogManager.h"
#include "YunziiYZ75KeyboardController.h"
#include "StringUtils.h"


YunziiYZ75KeyboardController::YunziiYZ75KeyboardController(hid_device* dev_handle, hid_device_info* dev_info, std::string dev_name)
{
    dev         = dev_handle;
    location    = dev_info->path;
    name        = dev_name;
    version     = dev_info->release_number;
    vendor      = StringUtils::wstring_to_string(dev_info->manufacturer_string);
    serial      = StringUtils::wstring_to_string(dev_info->serial_number);
}

YunziiYZ75KeyboardController::~YunziiYZ75KeyboardController()
{
    hid_close(dev);
}

std::string YunziiYZ75KeyboardController::GetLocation()
{
    return("HID: " + location);
}

std::string YunziiYZ75KeyboardController::GetName()
{
    return(name);
}

std::string YunziiYZ75KeyboardController::GetVendor()
{
    return(vendor);
}

std::string YunziiYZ75KeyboardController::GetSerialNumber()
{
    return(serial);
}

std::string YunziiYZ75KeyboardController::GetFirmwareVersion()
{
    return std::to_string(version);
}

unsigned char YunziiYZ75KeyboardController::Crc(const unsigned char data[], unsigned int start, unsigned int end)
{
    unsigned int crc = 0;

    for(unsigned int i = start; i < end; ++i)
    {
        crc += data[i];
    }

    crc &= 0xFF;
    crc = (255 - crc);

    return (crc & 0xFF);
}

void YunziiYZ75KeyboardController::WriteMode(unsigned char zone, unsigned char mode, unsigned char red, unsigned char grn, unsigned char blu, unsigned char color_mode, unsigned char speed, unsigned char brightness, unsigned char direction)
{
    unsigned char buf[64+1];
    memset(buf, 0, sizeof(buf));

    buf[0] = 0x00;                      // Write
    buf[1] = zone;                      // Zone [0x07: Light, 0x08: Strip]
    buf[2] = mode;                      // Light mode
    
    buf[3] = speed;                     // Speed
    buf[4] = brightness;                // Brightness

    buf[5] = (color_mode & 0xF) |       // Direction | ColorMode
             (direction << 4);          //   ColorMode [0x7: Color, 0x8: Random]
                                        //   Direction [0x0: Right, 0x1: Left, 0x2: Down, 0x3: Up]

    buf[6] = red;                       // Red
    buf[7] = grn;                       // Green
    buf[8] = blu;                       // Blue

    buf[9] = Crc(buf, 0, 9);            // Crc

    int ret = hid_send_feature_report(dev, buf, sizeof(buf));
    if(ret < 0)
    {
        LOG_ERROR("[%s] Failure to send report (%ls)!", name.c_str(), hid_error(dev));
    }
}

void YunziiYZ75KeyboardController::SetLightMode(unsigned int mode, RGBColor* color, bool use_random, int speed, int brightness, int direction)
{
    unsigned char zone = 0x07;  // Light zone

    switch(mode)
    {
        case YUNZII_MODE_OFF:
            mode = 0x00;
        case YUNZII_MODE_ALWAYS_ON:
            mode = 0x1;
            break;
        case YUNZII_MODE_DYNAMIC_BREATHING:
            mode = 0x2;
            break;
        case YUNZII_MODE_SPECTRUM_CYCLE:
            mode = 0x3;
            break;
        case YUNZII_MODE_DRIFT:
            mode = 0x4;
            break;
        case YUNZII_MODE_RIPPLE:
            mode = 0x5;
            break;
        case YUNZII_MODE_STARS_TWINKLY:
            mode = 0x6;
            break;
        case YUNZII_MODE_STEADY_STREAM:
            mode = 0x7;
            break;
        case YUNZII_MODE_SHADOWING:
            mode = 0x8;
            break;
        case YUNZII_MODE_PEAKS_RISING:
            mode = 0x9;
            break;
        case YUNZII_MODE_SINE_WAVE:
            mode = 0x0A;
            break;
        case YUNZII_MODE_CAISPRING_SURGING:
            mode = 0x0B;
            break;
        case YUNZII_MODE_FLOWERS_BLOOMING:
            mode = 0x0C;
            break;
        case YUNZII_MODE_LASER:
            mode = 0x0E;
            break;
        case YUNZII_MODE_PEAK_TURN:
            mode = 0x0F;
            break;
        case YUNZII_MODE_WAVE:
            mode = 0x10;
            break;
        case YUNZII_MODE_NEON:
            mode = 0x11;
            break;
        case YUNZII_MODE_SNAKE:
            mode = 0x12;
            break;
        case YUNZII_MODE_THROUGH_THE_SNOW:
            mode = 0x13;
            break;
        default:
            return;
    }

    int red = 0, grn = 0, blu = 0;
    if (color)
    {
        red = RGBGetRValue(*color); // RGB
        grn = RGBGetGValue(*color);
        blu = RGBGetBValue(*color);
    }
    
    int color_mode = use_random ? 0x08 : 0x07;          // Color mode
    speed = std::max(0, std::min(speed, 4));            // Speed
    brightness = std::max(0, std::min(brightness, 4));  // Brightness
    direction = std::max(0, std::min(direction, 3));    // Direction

    this->WriteMode(zone, mode, red, grn, blu, color_mode, speed, brightness, direction);
}


void YunziiYZ75KeyboardController::SetLightStripMode(unsigned int mode, RGBColor* color, bool use_random, int speed, int brightness, int direction)
{
    unsigned char zone = 0x08;  // Light zone

    switch(mode)
    {
        case YUNZII_MODE_OFF:
            break;
        case YUNZII_MODE_ALWAYS_ON:
            mode = 0x1;
            break;
        case YUNZII_MODE_DYNAMIC_BREATHING:
            mode = 0x02;
            break;
        case YUNZII_MODE_NEON:
            mode = 0x03;
            break;
        case YUNZII_MODE_SNAKE:
            mode = 0x05;
            break;
        case YUNZII_MODE_WAVE:
            mode = 0x04;
            break;
        default:
            return;
    }

    int red = 0, grn = 0, blu = 0;
    if (color)
    {
        red = RGBGetRValue(*color); // RGB
        grn = RGBGetGValue(*color);
        blu = RGBGetBValue(*color);
    }

    int color_mode = use_random ? 0x08 : 0x07;          // Color mode
    speed = std::max(0, std::min(speed, 3));            // Speed
    brightness = std::max(0, std::min(brightness, 4));  // Brightness
    direction = std::max(0, std::min(direction, 3));    // Direction
    
    this->WriteMode(zone, mode, red, grn, blu, color_mode, speed, brightness, direction);
}
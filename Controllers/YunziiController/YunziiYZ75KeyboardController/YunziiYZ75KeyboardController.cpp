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

void YunziiYZ75KeyboardController::WriteMode(unsigned char zone, unsigned char mode, unsigned char r, unsigned char g, unsigned char b, unsigned char flags, unsigned char speed, unsigned char brightness, unsigned char direction)
{
    unsigned char buf[64+1];
    memset(buf, 0, sizeof(buf));

    buf[0] = 0x00;              // Write
    buf[1] = zone;              // Zone [0x07: Light, 0x08: Strip]
    buf[2] = mode;              // Light mode
    
    buf[3] = speed;             // Speed
    buf[4] = brightness;        // Brightness
    buf[5] = flags;             // Flags: Direction | ColorMode
                                //    ColorMode [0x7: Color, 0x8: Random]
                                //    Direction [0x0: Right, 0x1: Left, 0x2: Down, 0x3: Up]

    buf[6] = r;                 // Red
    buf[7] = g;                 // Green
    buf[8] = b;                 // Blue

    buf[9] = Crc(buf, 0, 9);    // Crc

    int ret = hid_send_feature_report(dev, buf, sizeof(buf));
    if(ret < 0)
    {
        LOG_ERROR("[%s] Failure to send report (%ls)!", name.c_str(), hid_error(dev));
    }
}

void YunziiYZ75KeyboardController::SetLightMode(unsigned int mode, RGBColor* colors, unsigned int num_colors, bool use_random, int speed, int brightness, int direction)
{
    unsigned char zone = 0x07;  // Light zone

    switch(mode)
    {
        case YUNZII_KEYBOARD_MODE_ALWAYS_ON:
            mode = 0x1;
            break;
        case YUNZII_KEYBOARD_MODE_DYNAMIC_BREATHING:
            mode = 0x2;
            break;
        case YUNZII_KEYBOARD_MODE_SPECTRUM_CYCLE:
            mode = 0x3;
            break;
        case YUNZII_KEYBOARD_MODE_DRIFT:
            mode = 0x4;
            break;
        case YUNZII_KEYBOARD_MODE_WAVES_RIPPLE:
            mode = 0x5;
            break;
        case YUNZII_KEYBOARD_MODE_STARS_TWINKLY:
            mode = 0x6;
            break;
        case YUNZII_KEYBOARD_MODE_STEADY_STREAM:
            mode = 0x7;
            break;
        case YUNZII_KEYBOARD_MODE_SHADOWING:
            mode = 0x8;
            break;
        case YUNZII_KEYBOARD_MODE_PEAKS_RISING:
            mode = 0x9;
            break;
        case YUNZII_KEYBOARD_MODE_SINE_WAVE:
            mode = 0x0A;
            break;
        case YUNZII_KEYBOARD_MODE_CAISPRING_SURGING:
            mode = 0x0B;
            break;
        case YUNZII_KEYBOARD_MODE_FLOWERS_BLOOMING:
            mode = 0x0C;
            break;
        case YUNZII_KEYBOARD_MODE_LASER:
            mode = 0x0E;
            break;
        case YUNZII_KEYBOARD_MODE_PEAK_TURN:
            mode = 0x0F;
            break;
        case YUNZII_KEYBOARD_MODE_COLORFUL:
            mode = 0x10;
            break;
        case YUNZII_KEYBOARD_MODE_SNOW:
            mode = 0x11;
            break;
        case YUNZII_KEYBOARD_MODE_MATEOR:
            mode = 0x12;
            break;
        case YUNZII_KEYBOARD_MODE_THROUGH_THE_SNOW:
            mode = 0x13;
            break;
        case YUNZII_KEYBOARD_MODE_NONE:
        default:
            mode = 0x00;
            break;
    }

    int r = 0, g = 0, b = 0;
    if (num_colors > 0)
    {
        r = RGBGetRValue(colors[0]); // RGB
        g = RGBGetGValue(colors[0]);
        b = RGBGetBValue(colors[0]);
    }
    
    int flags = use_random ? 0x08 : 0x07;               // Color mode
    flags = (flags & 0xF) | (direction << 4);           // Direction
    speed = std::max(0, std::min(speed, 4));            // Speed
    brightness = std::max(0, std::min(brightness, 4));  // Brightness

    this->WriteMode(zone, mode, r, g, b, flags, speed, brightness, direction);
}


void YunziiYZ75KeyboardController::SetLightStripMode(unsigned int mode, RGBColor* colors, unsigned int num_colors, bool use_random, int speed, int brightness, int direction)
{
    unsigned char zone = 0x08;  // Light zone

    switch(mode)
    {
        case YUNZII_KEYBOARD_MODE_STRIP_ALWAYS_ON:
            mode = 0x1;
            break;
        case YUNZII_KEYBOARD_MODE_STRIP_BREATH:
            mode = 0x02;
            break;
        case YUNZII_KEYBOARD_MODE_STRIP_NEON:
            mode = 0x03;
            break;
        case YUNZII_KEYBOARD_MODE_STRIP_SNAKE:
            mode = 0x05;
            break;
        case YUNZII_KEYBOARD_MODE_STRIP_WAVE:
            mode = 0x04;
            break;
        case YUNZII_KEYBOARD_MODE_STRIP_NONE:
        default:
            mode = 0x00;
            break;
    }

    int r = 0, g = 0, b = 0;
    if (num_colors > 0)
    {
        r = RGBGetRValue(colors[0]); // RGB
        g = RGBGetGValue(colors[0]);
        b = RGBGetBValue(colors[0]);
    }

    int flags = use_random ? 0x08 : 0x07;               // Color mode
    flags = (flags & 0xF) | (direction << 4);           // Direction
    speed = std::max(0, std::min(speed, 3));            // Speed
    brightness = std::max(0, std::min(brightness, 4));  // Brightness
    
    this->WriteMode(zone, mode, r, g, b, flags, speed, brightness, direction);
}
/*---------------------------------------------------------*\
| MaonoController.cpp                                       |
|                                                           |
|   Driver for Maono microphone lighting                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cmath>
#include "MaonoController.h"
#include "LogManager.h"
#include "StringUtils.h"

MaonoController::MaonoController(hid_device* dev_handle, const char* path, std::string dev_name, const MaonoDeviceInfo* device_info)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
    info        = device_info;

    for(unsigned int channel = 0; channel < MAONO_MAX_CHANNELS; channel++)
    {
        custom_slot_ready[channel] = false;
    }
}

MaonoController::~MaonoController()
{
    hid_close(dev);
}

const MaonoDeviceInfo* MaonoController::GetInfo()
{
    return(info);
}

std::string MaonoController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string MaonoController::GetNameString()
{
    return(name);
}

std::string MaonoController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void MaonoController::WriteRegister(unsigned int channel, unsigned short reg, unsigned short value)
{
    if(reg == MAONO_REG_NONE || channel >= info->channels)
    {
        return;
    }

    reg = (unsigned short)(reg + channel * info->channel_stride);

    std::lock_guard<std::mutex> guard(lock);

    std::map<unsigned short, unsigned short>::iterator it = cache.find(reg);

    if(it != cache.end() && it->second == value)
    {
        return;
    }

    unsigned char usb_buf[MAONO_MAX_REPORT_SIZE];
    int           len = MaonoBuildFrame(usb_buf, info->report_id, MAONO_FRAME_TYPE_WRITE, reg, value);

    if(hid_write(dev, usb_buf, len) < 0)
    {
        LOG_WARNING("[%s] hid_write failed for register 0x%04X", name.c_str(), reg);
        cache.erase(reg);
        return;
    }

    cache[reg] = value;
}

void MaonoController::SetPower(unsigned int channel, bool on)
{
    WriteRegister(channel, info->reg_power, on ? 1 : 0);
}

void MaonoController::SetBrightness(unsigned int channel, unsigned int brightness)
{
    brightness = std::max<unsigned int>(info->brightness_min, std::min<unsigned int>(info->brightness_max, brightness));

    WriteRegister(channel, info->reg_brightness, (unsigned short)brightness);
}

void MaonoController::SetEffect(unsigned int channel, unsigned short effect)
{
    if(effect == MAONO_VALUE_NONE)
    {
        return;
    }

    WriteRegister(channel, info->reg_effect, effect);
}

void MaonoController::SetColorValue(unsigned int channel, unsigned short value)
{
    if(value == MAONO_VALUE_NONE)
    {
        return;
    }

    WriteRegister(channel, info->reg_color, value);
}

/*---------------------------------------------------------*\
| Free colours go through custom slot 1 (REGISTER_BLOCK     |
| devices only). Maono Link converts the colour to HSV and  |
| writes hue x 100 (0-35900), saturation x 100 and value x  |
| 100 (DM40.dll 0x180011EF0, PD200XW.dll 0x18006D860), then |
| selects the slot through the colour register.             |
\*---------------------------------------------------------*/
void MaonoController::SetCustomColor(unsigned int channel, unsigned char red, unsigned char grn, unsigned char blu)
{
    if(info->custom_slots == 0 || info->reg_custom_hsv == MAONO_REG_NONE)
    {
        return;
    }

    int max_c = std::max<int>(red, std::max<int>(grn, blu));
    int min_c = std::min<int>(red, std::min<int>(grn, blu));
    int delta = max_c - min_c;

    float hue = 0.0f;

    if(delta > 0)
    {
        if(max_c == red)
        {
            hue = 60.0f * ((float)(grn - blu) / delta);
        }
        else if(max_c == grn)
        {
            hue = 60.0f * (((float)(blu - red) / delta) + 2.0f);
        }
        else
        {
            hue = 60.0f * (((float)(red - grn) / delta) + 4.0f);
        }

        if(hue < 0.0f)
        {
            hue += 360.0f;
        }
    }

    unsigned short hue_reg = (unsigned short)(((int)hue % 360) * 100);
    unsigned short sat_reg = (unsigned short)((max_c == 0) ? 0 : (delta * 100) / max_c);
    unsigned short val_reg = (unsigned short)((max_c * 100) / 255);

    WriteRegister(channel, info->reg_custom_hsv + 0, hue_reg);
    WriteRegister(channel, info->reg_custom_hsv + 1, sat_reg);
    WriteRegister(channel, info->reg_custom_hsv + 2, val_reg);

    if(channel < MAONO_MAX_CHANNELS && !custom_slot_ready[channel])
    {
        WriteRegister(channel, info->reg_custom_count, 1);
        custom_slot_ready[channel] = true;
    }

    WriteRegister(channel, info->reg_color, info->custom_first_index);
}

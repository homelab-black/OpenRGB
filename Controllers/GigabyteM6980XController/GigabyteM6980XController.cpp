/*---------------------------------------------------------*\
| GigabyteM6980XController.cpp                              |
|                                                           |
|   Driver for the Gigabyte M6980X mouse                    |
|                                                           |
|   Tom Rehm (Tom-8192)                         09 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <string.h>
#include <thread>
#include "GigabyteM6980XController.h"
#include "LogManager.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| Color byte (0-255) to channel level (0-2) and back        |
\*---------------------------------------------------------*/
static unsigned char LevelFromByte(unsigned char value)
{
    if(value < 64)
    {
        return(0);
    }

    if(value < 192)
    {
        return(1);
    }

    return(2);
}

static unsigned char ByteFromLevel(unsigned char level)
{
    const unsigned char bytes[GIGABYTE_M6980X_LEVELS] = { 0x00, 0x80, 0xFF };

    /*-----------------------------------------------------*\
    | Other software may have written 0-255 values, which   |
    | the mouse keeps and reports back.  Show them as is.   |
    \*-----------------------------------------------------*/
    if(level >= GIGABYTE_M6980X_LEVELS)
    {
        return(level);
    }

    return(bytes[level]);
}

GigabyteM6980XController::GigabyteM6980XController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name)
{
    dev         = dev_handle;
    location    = info.path;
    name        = dev_name;
}

GigabyteM6980XController::~GigabyteM6980XController()
{
    hid_close(dev);
}

std::string GigabyteM6980XController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string GigabyteM6980XController::GetNameString()
{
    return(name);
}

std::string GigabyteM6980XController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool GigabyteM6980XController::IsM6980X()
{
    std::lock_guard<std::mutex> guard(mutex_lock);

    unsigned char reply[GIGABYTE_M6980X_REPORT_SIZE];

    if(!Transfer(GIGABYTE_M6980X_GROUP_SYSTEM, GIGABYTE_M6980X_CMD_IDENTIFY, NULL, 0, 0, reply))
    {
        return(false);
    }

    LOG_DEBUG("[%s] Identify: %02X %02X %02X %02X %02X %02X %02X", name.c_str(), reply[1], reply[2], reply[3], reply[4], reply[5], reply[6], reply[7]);

    /*-----------------------------------------------------*\
    | Other mice (M8000X, M8600, 6980) use the same USB     |
    | ID but a different memory layout.  The M6980X         |
    | replies 0C 12 09 .. .. .. 01.                         |
    \*-----------------------------------------------------*/
    return(reply[1] == 0x0C && reply[2] == 0x12 && reply[3] == 0x09 && reply[7] == 0x01);
}

bool GigabyteM6980XController::GetColor(unsigned char profile, RGBColor* color)
{
    std::lock_guard<std::mutex> guard(mutex_lock);

    unsigned char levels[3];

    if(!GetLevels(profile, levels))
    {
        return(false);
    }

    *color = ToRGBColor(ByteFromLevel(levels[0]), ByteFromLevel(levels[1]), ByteFromLevel(levels[2]));

    return(true);
}

void GigabyteM6980XController::SetColor(unsigned char profile, RGBColor color)
{
    std::lock_guard<std::mutex> guard(mutex_lock);

    unsigned char levels[3];
    unsigned char current[3];

    levels[0] = LevelFromByte(RGBGetRValue(color));
    levels[1] = LevelFromByte(RGBGetGValue(color));
    levels[2] = LevelFromByte(RGBGetBValue(color));

    /*-----------------------------------------------------*\
    | The mouse keeps its colors without a save command, so |
    | every write presumably goes to flash.  Only write     |
    | when the color differs from what the mouse has.       |
    \*-----------------------------------------------------*/
    if(!GetLevels(profile, current))
    {
        return;
    }

    if(memcmp(levels, current, sizeof(levels)) == 0)
    {
        return;
    }

    unsigned char args[4];
    unsigned char reply[GIGABYTE_M6980X_REPORT_SIZE];

    args[0] = profile;
    args[1] = levels[0];
    args[2] = levels[1];
    args[3] = levels[2];

    LOG_DEBUG("[%s] Writing profile %d: %d %d %d", name.c_str(), profile + 1, levels[0], levels[1], levels[2]);

    if(!Transfer(GIGABYTE_M6980X_GROUP_REGISTER, GIGABYTE_M6980X_REG_LED | GIGABYTE_M6980X_WRITE_BIT, args, 4, GIGABYTE_M6980X_WRITE_DELAY_MS, reply))
    {
        return;
    }

    if(GetLevels(profile, current) && memcmp(levels, current, sizeof(levels)) != 0)
    {
        LOG_WARNING("[%s] Profile %d did not take the new color", name.c_str(), profile + 1);
    }
}

bool GigabyteM6980XController::GetLevels(unsigned char profile, unsigned char* levels)
{
    unsigned char reply[GIGABYTE_M6980X_REPORT_SIZE];

    if(profile >= GIGABYTE_M6980X_PROFILES)
    {
        return(false);
    }

    if(!Transfer(GIGABYTE_M6980X_GROUP_REGISTER, GIGABYTE_M6980X_REG_LED, &profile, 1, 0, reply))
    {
        return(false);
    }

    levels[0] = reply[1];
    levels[1] = reply[2];
    levels[2] = reply[3];

    return(true);
}

bool GigabyteM6980XController::Transfer(unsigned char group, unsigned char command, unsigned char* args, int arg_count, int delay_ms, unsigned char* reply)
{
    /*-----------------------------------------------------*\
    | Byte 0 is the report ID (0, the mouse has none)       |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[GIGABYTE_M6980X_REPORT_SIZE + 1];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[1] = group;
    usb_buf[2] = command;

    for(int arg_idx = 0; arg_idx < arg_count; arg_idx++)
    {
        usb_buf[3 + arg_idx] = args[arg_idx];
    }

    /*-----------------------------------------------------*\
    | Wait until the mouse is not busy                      |
    \*-----------------------------------------------------*/
    bool ready = false;

    for(int attempt = 0; attempt < GIGABYTE_M6980X_BUSY_TRIES; attempt++)
    {
        if(GetReport(reply) && (reply[0] & GIGABYTE_M6980X_STATUS_BUSY) == 0)
        {
            ready = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(GIGABYTE_M6980X_POLL_MS));
    }

    if(!ready)
    {
        LOG_WARNING("[%s] Mouse stays busy, command %02X %02X not sent", name.c_str(), group, command);
        return(false);
    }

    if(hid_send_feature_report(dev, usb_buf, sizeof(usb_buf)) < 0)
    {
        LOG_WARNING("[%s] Sending command %02X %02X failed", name.c_str(), group, command);
        return(false);
    }

    if(delay_ms > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    /*-----------------------------------------------------*\
    | Poll until the mouse reports the command as done      |
    \*-----------------------------------------------------*/
    for(int attempt = 0; attempt < GIGABYTE_M6980X_DONE_TRIES; attempt++)
    {
        if(GetReport(reply) && (reply[0] & GIGABYTE_M6980X_STATUS_DONE) != 0)
        {
            return(true);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(GIGABYTE_M6980X_POLL_MS));
    }

    LOG_WARNING("[%s] No reply to command %02X %02X", name.c_str(), group, command);
    return(false);
}

bool GigabyteM6980XController::GetReport(unsigned char* data)
{
    unsigned char usb_buf[GIGABYTE_M6980X_REPORT_SIZE + 1];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    if(hid_get_feature_report(dev, usb_buf, sizeof(usb_buf)) != (int)sizeof(usb_buf))
    {
        return(false);
    }

    memcpy(data, usb_buf + 1, GIGABYTE_M6980X_REPORT_SIZE);

    return(true);
}

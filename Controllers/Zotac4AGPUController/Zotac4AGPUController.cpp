/*---------------------------------------------------------*\
| Zotac4AGPUController.cpp                                  |
|                                                           |
|   Driver for Zotac 4A GPU                                 |
|                                                           |
|   Peter Berendi                               29 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "Zotac4AGPUController.h"
#include "LogManager.h"
#include <thread>

Zotac4AGPUController::Zotac4AGPUController(i2c_smbus_interface* bus, u8 dev, std::string detected_version)
{
    this->bus = bus;
    this->dev = dev;
    version = detected_version;
    last_request = std::chrono::steady_clock::now();
    LOG_INFO("[Zotac4A] Controller firmware: %s", version.c_str());
    if(
        (version != "N702E-1002") &&
        (version != "N702E-2001") &&
        (version != "N764E-1001") &&  // 5070 Solid
        (version != "N765E-1001")     // 5080 Solid Core
      )
    {
        LOG_WARNING("[Zotac4A] Controller ID: %s", version.c_str());
    }
}

Zotac4AGPUController::~Zotac4AGPUController()
{
}

std::string Zotac4AGPUController::GetDeviceLocation()
{
    std::string return_string(bus->info.device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return ("I2C: " + return_string);
}

std::string Zotac4AGPUController::GetVersion()
{
    return version;
}

bool Zotac4AGPUController::SetMode(Zotac4AGPUZone zone_config)
{
    return SendCommand(true, false, zone_config);
}

bool Zotac4AGPUController::GetMode(Zotac4AGPUZone& zone_config, bool& on)
{
    return ParseCommand(on, zone_config);
}

bool Zotac4AGPUController::SendCommand(bool on, bool reset, Zotac4AGPUZone zone_config)
{
    std::lock_guard<std::mutex> lock(request_mutex);
    std::this_thread::sleep_until(last_request + ZOTAC_4A_GPU_WAIT_BETWEEN_REQUESTS);
    if(zone_config.mode != ZOTAC_4A_GPU_MODE_STATIC && zone_config.mode != ZOTAC_4A_GPU_MODE_BREATHE)
    {
        LOG_ERROR("[Zotac4A] Untested mode rejected: %d", zone_config.mode);
        return false;
    }
    u8 on_time = 45;
    u8 off_time = 10;
    switch(zone_config.mode)
    {
    case ZOTAC_4A_GPU_MODE_WINK:
    case ZOTAC_4A_GPU_MODE_RANDOM:
        on_time = zone_config.on_time;
        off_time = ZOTAC_4A_RGB_MAXSPEED - zone_config.speed;
        break;
    case ZOTAC_4A_GPU_MODE_BREATHE:
    case ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE:
    case ZOTAC_4A_GPU_MODE_FADE:
        on_time = ZOTAC_4A_RGB_MAXSPEED - zone_config.speed;
        break;
    }

    u8 data_pkt[ZOTAC_4A_GPU_RGB_CONTROL_LENGTH] =
        {
            reset ? (u8)0x01 : (u8)0x00,
            on ? (u8)0x01 : (u8)0x00,
            (u8)zone_config.mode,
            (u8)RGBGetRValue(zone_config.color),
            (u8)RGBGetGValue(zone_config.color),
            (u8)RGBGetBValue(zone_config.color),
            (u8)on_time,
            (u8)off_time,
            (u8)0x08, // preset index, 8 is custom
            (u8)0x00  // the rest is apparently always 0
        };

    s32 res = bus->i2c_smbus_write_i2c_block_data(dev, ZOTAC_4A_GPU_REG_RGB, sizeof(data_pkt), data_pkt);
    last_request = std::chrono::steady_clock::now();
    LOG_DEBUG("[Zotac4A] Write mode %d color %02X%02X%02X, 32 bytes: status %d", zone_config.mode, RGBGetRValue(zone_config.color), RGBGetGValue(zone_config.color), RGBGetBValue(zone_config.color), res);
    return res == 0;
}

bool Zotac4AGPUController::ParseCommand(bool& on, Zotac4AGPUZone& zone_config)
{
    std::lock_guard<std::mutex> lock(request_mutex);
    std::this_thread::sleep_until(last_request + ZOTAC_4A_GPU_WAIT_BETWEEN_REQUESTS);
    u8 data_pkt[I2C_SMBUS_BLOCK_MAX] = { 0 };
    s32 res = bus->i2c_smbus_read_i2c_block_data(dev, ZOTAC_4A_GPU_REG_RGB, sizeof(data_pkt), data_pkt);
    last_request = std::chrono::steady_clock::now();
    if(res != I2C_SMBUS_BLOCK_MAX)
    {
        LOG_TRACE("Zotac4AGPUController::ParseCommand() i2c_smbus_read_i2c_block_data(0x%02X) returned %d", dev, res);
        return false;
    }
    u8 *status = data_pkt + ZOTAC_4A_RGB_STATUS_OFFSET;
    on = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_ONOFF] != 0x00;
    zone_config.mode = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_MODE];
    zone_config.color = ToRGBColor(
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_RED],
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_GREEN],
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_BLUE]
    );
    switch(zone_config.mode)
    {
    case ZOTAC_4A_GPU_MODE_WINK:
    case ZOTAC_4A_GPU_MODE_RANDOM:
        zone_config.on_time = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_ON];
        zone_config.speed = ZOTAC_4A_RGB_MAXSPEED - status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_OFF];
        break;
    case ZOTAC_4A_GPU_MODE_BREATHE:
    case ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE:
    case ZOTAC_4A_GPU_MODE_FADE:
        zone_config.speed = ZOTAC_4A_RGB_MAXSPEED - status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_ON];
        break;
    }
    return true;
}

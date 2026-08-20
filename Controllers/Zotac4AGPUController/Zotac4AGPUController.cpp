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

Zotac4AGPUController::Zotac4AGPUController(i2c_smbus_interface* bus, u8 dev, std::string detectedversion)
{
    this->bus = bus;
    this->dev = dev;
    version = detectedversion;
    if(
        (version != "N702E-1002") &&
        (version != "N702E-2001") &&
        (version != "N764E-1001")     // 5070 Solid
      )
    {
        LOG_WARNING("Unknown controller ID \"%s\", attemping to use it anyway");
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

bool Zotac4AGPUController::SetMode(Zotac4AGPUZone zoneConfig)
{
    return SendCommand(true, false, zoneConfig);
}

bool Zotac4AGPUController::GetMode(Zotac4AGPUZone& zoneConfig, bool& on)
{
    return ParseCommand(on, zoneConfig);
}

bool Zotac4AGPUController::SendCommand(bool on, bool reset, Zotac4AGPUZone zoneConfig)
{
    u8 on_time = 45;
    u8 off_time = 10;
    switch(zoneConfig.mode)
    {
    case ZOTAC_4A_GPU_MODE_WINK:
    case ZOTAC_4A_GPU_MODE_RANDOM:
        on_time = zoneConfig.on_time;
        off_time = ZOTAC_4A_RGB_MAXSPEED - zoneConfig.speed;
        break;
    case ZOTAC_4A_GPU_MODE_BREATHE:
    case ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE:
    case ZOTAC_4A_GPU_MODE_FADE:
        on_time = ZOTAC_4A_RGB_MAXSPEED - zoneConfig.speed;
        break;
    }

    u8 data_pkt[ZOTAC_4A_GPU_RGB_CONTROL_LENGTH] =
        {
            reset ? (u8)0x01 : (u8)0x00,
            on ? (u8)0x01 : (u8)0x00,
            (u8)zoneConfig.mode,
            (u8)RGBGetRValue(zoneConfig.color),
            (u8)RGBGetGValue(zoneConfig.color),
            (u8)RGBGetBValue(zoneConfig.color),
            (u8)on_time,
            (u8)off_time,
            (u8)0x08, // preset index, 8 is custom
            (u8)0x00  // the rest is apparently always 0
        };

    s32 res = bus->i2c_smbus_write_i2c_block_data(dev, ZOTAC_4A_GPU_REG_RGB, sizeof(data_pkt), data_pkt);
    if(res < 0)
    {
        return false;
    }
    return true;
}

bool Zotac4AGPUController::ParseCommand(bool& on, Zotac4AGPUZone& zoneConfig)
{
    u8 data_pkt[I2C_SMBUS_BLOCK_MAX] = { 0 };
    s32 res = bus->i2c_smbus_read_i2c_block_data(dev, ZOTAC_4A_GPU_REG_RGB, sizeof(data_pkt), data_pkt);
    if(res < 0)
    {
        LOG_TRACE("Zotac4AGPUController::ParseCommand() i2c_smbus_read_i2c_block_data(0x%02X) returned %d", dev, res);
        return false;
    }
    u8 *status = data_pkt + ZOTAC_4A_RGB_STATUS_OFFSET;
    on = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_ONOFF] != 0x00;
    zoneConfig.mode = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_MODE];
    zoneConfig.color = ToRGBColor(
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_RED],
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_GREEN],
        status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_BLUE]
    );
    switch(zoneConfig.mode) {
    case ZOTAC_4A_GPU_MODE_WINK:
    case ZOTAC_4A_GPU_MODE_RANDOM:
        zoneConfig.on_time = status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_ON];
        zoneConfig.speed = ZOTAC_4A_RGB_MAXSPEED - status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_OFF];
        break;
    case ZOTAC_4A_GPU_MODE_BREATHE:
    case ZOTAC_4A_GPU_MODE_SPECTRUMBREATHE:
    case ZOTAC_4A_GPU_MODE_FADE:
        zoneConfig.speed = ZOTAC_4A_RGB_MAXSPEED - status[ZOTAC_4A_GPU_RGB_CONTROL_OFFSET_TIME_ON];
        break;
    }
    return true;
}

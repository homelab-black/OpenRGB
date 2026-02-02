/*---------------------------------------------------------*\
| ZotacV3GPUController.cpp                                  |
|                                                           |
|   Driver for Zotac V3 GPU                                 |
|                                                           |
|   Based on ZotacV2GPUController                           |
|   Adapted for Zotac RTX 5070 SOLIC OC                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ZotacV3GPUController.h"
#include "LogManager.h"

ZotacV3GPUController::ZotacV3GPUController(i2c_smbus_interface* bus, u8 dev, std::string dev_name)
{
    this->bus   = bus;
    this->dev   = dev;
    this->name  = dev_name;

    if(dev)
    {
        ReadVersion();
    }
}

ZotacV3GPUController::~ZotacV3GPUController()
{
}

std::string ZotacV3GPUController::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return ("I2C: " + return_string);
}

std::string ZotacV3GPUController::GetName()
{
    return(name);
}

std::string ZotacV3GPUController::GetVersion()
{
    return(version);
}

bool ZotacV3GPUController::ReadVersion()
{
    u8 rdata_pkt[I2C_SMBUS_BLOCK_MAX] = { 0x00 };

    if(bus->i2c_smbus_read_i2c_block_data(dev, ZOTAC_V3_GPU_REG_RGB, sizeof(rdata_pkt), rdata_pkt) < 0)
    {
        return false;
    }

    version = std::string((char*)rdata_pkt, 10);

    return true;
}

bool ZotacV3GPUController::ResetToDefaults()
{
    return SendCommand(true, true, ZotacV3GPUZone());
}

bool ZotacV3GPUController::SetMode(ZotacV3GPUZone zoneConfig)
{
    return SendCommand(true, false, zoneConfig);
}

bool ZotacV3GPUController::GetMode(ZotacV3GPUZone& zoneConfig, bool& on)
{
    bool readReset;
    if(!ParseCommand(on, readReset, zoneConfig))
    {
        return false;
    }

    if(readReset != 0)
    {
        LOG_WARNING("Reset byte was not 0?!");
    }

    return true;
}

bool ZotacV3GPUController::SendCommand(bool on, bool reset, ZotacV3GPUZone zoneConfig)
{
    u8 data_pkt[] =
    {
        reset ? (u8)0x01 : (u8)0x00,
        on ? (u8)0x01 : (u8)0x00,
        (u8)zoneConfig.mode,
        (u8)RGBGetRValue(zoneConfig.color1),
        (u8)RGBGetGValue(zoneConfig.color1),
        (u8)RGBGetBValue(zoneConfig.color1),
        0x2D,
        0x0A,
        (u8)zoneConfig.colorPreset,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if(bus->i2c_smbus_write_i2c_block_data(dev, ZOTAC_V3_GPU_REG_RGB, sizeof(data_pkt), data_pkt) < 0)
    {
        return false;
    }
    return true;
}

bool ZotacV3GPUController::ParseCommand(bool& on, bool& reset, ZotacV3GPUZone& zoneConfig)
{
    u8 rdata_pkt[I2C_SMBUS_BLOCK_MAX] = { 0x00 };

    if(bus->i2c_smbus_read_i2c_block_data(dev, ZOTAC_V3_GPU_REG_RGB, sizeof(rdata_pkt), rdata_pkt) < 0)
    {
        return false;
    }

    on                     = rdata_pkt[10] != 0x00;
    reset                  = false;
    zoneConfig.mode        = rdata_pkt[11];
    zoneConfig.color1      = ToRGBColor(rdata_pkt[12], rdata_pkt[13], rdata_pkt[14]);
    zoneConfig.colorPreset = rdata_pkt[17];
    return true;
}

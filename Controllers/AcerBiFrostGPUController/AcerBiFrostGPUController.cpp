/*---------------------------------------------------------*\
| AcerBiFrostGPUController.cpp                              |
|                                                           |
|   Driver for Acer Predator BiFrost GPU                    |
|                                                           |
|   Baobaosheng                                 18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "AcerBiFrostGPUController.h"

AcerBiFrostGPUController::AcerBiFrostGPUController(i2c_smbus_interface* bus, acer_bifrost_dev_id dev, std::string dev_name)
{
    this->bus           = bus;
    this->dev           = dev;
    this->name          = dev_name;
    this->lamp_count    = ACER_BIFROST_NUM_LEDS;
    this->host_control  = false;
}

AcerBiFrostGPUController::~AcerBiFrostGPUController()
{

}

std::string AcerBiFrostGPUController::GetDeviceLocation()
{
    std::string return_string(bus->info.device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return("I2C:" + return_string);
}

std::string AcerBiFrostGPUController::GetDeviceName()
{
    return(name);
}

int AcerBiFrostGPUController::GetLampCount()
{
    int count = bus->i2c_smbus_read_byte_data(dev, ACER_BIFROST_REG_LAMP_COUNT);

    if(count > 0 && count <= ACER_BIFROST_NUM_LEDS)
    {
        lamp_count = (unsigned int)count;
    }

    return(count);
}

/*---------------------------------------------------------*\
| Take the lamps away from the MCU's built-in effect. The   |
| MCU keeps this until power is removed, so it is only sent |
| once per controller lifetime.                             |
\*---------------------------------------------------------*/
void AcerBiFrostGPUController::SetHostControl()
{
    if(!host_control)
    {
        if(bus->i2c_smbus_write_byte_data(dev, ACER_BIFROST_REG_AUTONOMOUS_MODE, ACER_BIFROST_AUTONOMOUS_OFF) >= 0)
        {
            host_control = true;
        }
    }
}

/*---------------------------------------------------------*\
| LampRangeUpdate: flags, first lamp (LE16), last lamp      |
| (LE16), red, green, blue, intensity                       |
\*---------------------------------------------------------*/
void AcerBiFrostGPUController::SetLedColorAll(RGBColor color)
{
    SetHostControl();

    unsigned char data[9] =
    {
        ACER_BIFROST_LAMP_UPDATE_COMPLETE,
        0x00,
        0x00,
        (unsigned char)(lamp_count - 1),
        0x00,
        (unsigned char)RGBGetRValue(color),
        (unsigned char)RGBGetGValue(color),
        (unsigned char)RGBGetBValue(color),
        ACER_BIFROST_INTENSITY_MAX
    };

    bus->i2c_smbus_write_i2c_block_data(dev, ACER_BIFROST_REG_LAMP_RANGE_UPDATE, sizeof(data), data);
}

/*---------------------------------------------------------*\
| LampMultiUpdate: a flag byte, then (lamp, red, green,     |
| blue, intensity) for up to 2 lamps per write. The flag is |
| set on the write that carries the last lamps.             |
\*---------------------------------------------------------*/
void AcerBiFrostGPUController::SetLedColors(const RGBColor* colors, unsigned int count)
{
    SetHostControl();

    if(count > lamp_count)
    {
        count = lamp_count;
    }

    for(unsigned int first = 0; first < count; first += ACER_BIFROST_LAMPS_PER_MULTI_UPDATE)
    {
        unsigned int  remaining = count - first;
        unsigned int  lamps     = remaining < ACER_BIFROST_LAMPS_PER_MULTI_UPDATE ? remaining : ACER_BIFROST_LAMPS_PER_MULTI_UPDATE;
        unsigned char data[1 + (5 * ACER_BIFROST_LAMPS_PER_MULTI_UPDATE)];

        data[0] = (remaining <= ACER_BIFROST_LAMPS_PER_MULTI_UPDATE) ? 0x01 : 0x00;

        for(unsigned int i = 0; i < lamps; i++)
        {
            RGBColor color   = colors[first + i];
            data[1 + (5 * i)] = (unsigned char)(first + i);
            data[2 + (5 * i)] = (unsigned char)RGBGetRValue(color);
            data[3 + (5 * i)] = (unsigned char)RGBGetGValue(color);
            data[4 + (5 * i)] = (unsigned char)RGBGetBValue(color);
            data[5 + (5 * i)] = ACER_BIFROST_INTENSITY_MAX;
        }

        bus->i2c_smbus_write_i2c_block_data(dev, ACER_BIFROST_REG_LAMP_MULTI_UPDATE, (u8)(1 + (5 * lamps)), data);
    }
}

/*---------------------------------------------------------*\
| GalaxGPUv2Controller.cpp                                  |
|                                                           |
|   Driver for GALAX/KFA2 RTX 30 Series GPU                 |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstdio>
#include "LogManager.h"
#include "GalaxGPUv2Controller.h"

GalaxGPUv2Controller::GalaxGPUv2Controller(i2c_smbus_interface* bus, galax_gpu_dev_id dev)
{
    this->bus = bus;
    this->dev = dev;

    // Would be nice to get the actual GPU name. Using this as a placeholder.
    device_name = "GALAX/KFA2 RTX 30 Series GPU";
}

GalaxGPUv2Controller::~GalaxGPUv2Controller()
{

}

std::string GalaxGPUv2Controller::GetDeviceName()
{
    return(device_name);
}

std::string GalaxGPUv2Controller::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return("I2C: " + return_string);
}

unsigned char GalaxGPUv2Controller::GetLEDRed()
{
    return(RegisterRead(GALAX_V2_RED_REGISTER));
}

unsigned char GalaxGPUv2Controller::GetLEDGreen()
{
    return(RegisterRead(GALAX_V2_GREEN_REGISTER));
}

unsigned char GalaxGPUv2Controller::GetLEDBlue()
{
    return(RegisterRead(GALAX_V2_BLUE_REGISTER));
}

void GalaxGPUv2Controller::SetLEDColors(unsigned char red, unsigned char green, unsigned char blue)
{
    RegisterWrite(GALAX_V2_RED_REGISTER, red);
    RegisterWrite(GALAX_V2_GREEN_REGISTER, green);
    RegisterWrite(GALAX_V2_BLUE_REGISTER, blue);
}

unsigned char GalaxGPUv2Controller::GetMode()
{
    unsigned char mode = GALAX_V2_MODE_BREATHING_CYCLE;
    unsigned char data1 = RegisterRead(GALAX_V2_MODE_REGISTER_1);
    unsigned char data2 = RegisterRead(GALAX_V2_MODE_REGISTER_2);

    if(data1 == GALAX_V2_MODE_STATIC_DATA_1 && data2 == GALAX_V2_MODE_STATIC_DATA_2)
    {
        mode = GALAX_V2_MODE_STATIC;
    }
    else if(data1 == GALAX_V2_MODE_BREATHING_DATA_1 && data2 == GALAX_V2_MODE_BREATHING_DATA_2)
    {
        mode = GALAX_V2_MODE_BREATHING;
    }
    else if(data1 == GALAX_V2_MODE_BREATHING_CYCLE_DATA_1 && data2 == GALAX_V2_MODE_BREATHING_CYCLE_DATA_2)
    {
        mode = GALAX_V2_MODE_BREATHING_CYCLE;
    }
    else if(data1 == GALAX_V2_MODE_SPECTRUM_CYCLE_DATA_1 && data2 == GALAX_V2_MODE_SPECTRUM_CYCLE_DATA_2)
    {
        mode = GALAX_V2_MODE_SPECTRUM_CYCLE;
    }
    else if(data1 == GALAX_V2_MODE_OFF_DATA_1 && data2 == GALAX_V2_MODE_OFF_DATA_2)
    {
        mode = GALAX_V2_MODE_OFF;
    }
    else
    {
        LOG_DEBUG("[GALAX GPU v2] GetMode: Couldn't determine mode from data1 & data2: 0x%x, 0x%x", data1, data2);
    }

    return(mode);
}

void GalaxGPUv2Controller::SetMode(unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch(mode)
    {
    case GALAX_V2_MODE_STATIC:
        reg1 = GALAX_V2_MODE_STATIC_DATA_1;
        reg2 = GALAX_V2_MODE_STATIC_DATA_2;
        break;

    case GALAX_V2_MODE_BREATHING:
        reg1 = GALAX_V2_MODE_BREATHING_DATA_1;
        reg2 = GALAX_V2_MODE_BREATHING_DATA_2;
        break;

    case GALAX_V2_MODE_BREATHING_CYCLE:
        reg1 = GALAX_V2_MODE_BREATHING_CYCLE_DATA_1;
        reg2 = GALAX_V2_MODE_BREATHING_CYCLE_DATA_2;
        break;

    case GALAX_V2_MODE_SPECTRUM_CYCLE:
        reg1 = GALAX_V2_MODE_SPECTRUM_CYCLE_DATA_1;
        reg2 = GALAX_V2_MODE_SPECTRUM_CYCLE_DATA_2;
        break;

    case GALAX_V2_MODE_OFF:
        reg1 = GALAX_V2_MODE_OFF_DATA_1;
        reg2 = GALAX_V2_MODE_OFF_DATA_2;
        SetLEDColors(0, 0, 0);
        break;

    default:
        LOG_WARNING("[GALAX GPU v2] SetMode: Unknown mode value %d", mode);
        return;
    }

    RegisterWrite(GALAX_V2_MODE_REGISTER_1, reg1);
    RegisterWrite(GALAX_V2_MODE_REGISTER_2, reg2);
}

void GalaxGPUv2Controller::SaveMode()
{
    RegisterWrite(GALAX_V2_SAVE_REGISTER, GALAX_V2_SAVE_DATA);
}

unsigned char GalaxGPUv2Controller::RegisterRead(unsigned char reg)
{
    return(bus->i2c_smbus_read_byte_data(dev, reg));
}

void GalaxGPUv2Controller::RegisterWrite(unsigned char reg, unsigned char val)
{
    LOG_DEBUG("[GALAX GPU v2] RegisterWrite: 0x%x -> 0x%x", reg, val);
    bus->i2c_smbus_write_byte_data(dev, reg, val);
}

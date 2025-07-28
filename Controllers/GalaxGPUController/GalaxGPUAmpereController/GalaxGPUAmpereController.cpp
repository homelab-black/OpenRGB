/*---------------------------------------------------------*\
| GalaxGPUAmpereController.cpp                              |
|                                                           |
|   Driver for Galax/KFA2 GPU Ampere Series                 |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "GalaxGPUAmpereController.h"

GalaxGPUAmpereController::GalaxGPUAmpereController(i2c_smbus_interface* bus, galax_gpu_dev_id dev)
{
    this->bus = bus;
    this->dev = dev;

    // Would be nice to get the actual GPU name. Using this as a placeholder.
    device_name = "Galax RTX Ampere GPU";
}

GalaxGPUAmpereController::~GalaxGPUAmpereController()
{

}

std::string GalaxGPUAmpereController::GetDeviceName()
{
    return(device_name);
}

std::string GalaxGPUAmpereController::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return("I2C: " + return_string);
}

unsigned char GalaxGPUAmpereController::GetLEDRed()
{
    return(RegisterRead(GALAX_AMPERE_RED_REGISTER));
}

unsigned char GalaxGPUAmpereController::GetLEDGreen()
{
    return(RegisterRead(GALAX_AMPERE_GREEN_REGISTER));
}

unsigned char GalaxGPUAmpereController::GetLEDBlue()
{
    return(RegisterRead(GALAX_AMPERE_BLUE_REGISTER));
}

void GalaxGPUAmpereController::SetLEDColors(unsigned char red, unsigned char green, unsigned char blue)
{
    RegisterWrite(GALAX_AMPERE_RED_REGISTER, red);
    RegisterWrite(GALAX_AMPERE_GREEN_REGISTER, green);
    RegisterWrite(GALAX_AMPERE_BLUE_REGISTER, blue);
}

unsigned char GalaxGPUAmpereController::GetMode()
{
    unsigned char mode = GALAX_AMPERE_MODE_BREATHING_CYCLE;
    unsigned char data1 = RegisterRead(GALAX_AMPERE_MODE_REGISTER_1);
    unsigned char data2 = RegisterRead(GALAX_AMPERE_MODE_REGISTER_2);

    if(data1 == GALAX_AMPERE_MODE_STATIC_DATA_1 && data2 == GALAX_AMPERE_MODE_STATIC_DATA_2)
    {
        mode = GALAX_AMPERE_MODE_STATIC;
    }
    else if(data1 == GALAX_AMPERE_MODE_BREATHING_DATA_1 && data2 == GALAX_AMPERE_MODE_BREATHING_DATA_2)
    {
        mode = GALAX_AMPERE_MODE_BREATHING;
    }
    else if(data1 == GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_1 && data2 == GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_2)
    {
        mode = GALAX_AMPERE_MODE_BREATHING_CYCLE;
    }
    else if(data1 == GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_1 && data2 == GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_2)
    {
        mode = GALAX_AMPERE_MODE_SPECTRUM_CYCLE;
    }
    else if(data1 == GALAX_AMPERE_MODE_OFF_DATA_1 && data2 == GALAX_AMPERE_MODE_OFF_DATA_2)
    {
        mode = GALAX_AMPERE_MODE_OFF;
    }
    else
    {
        LOG_DEBUG("[GALAX GPU Ampere] GetMode: Couldn't determine mode from data1 & data2: 0x%x, 0x%x", data1, data2);
    }

    return(mode);
}

void GalaxGPUAmpereController::SetMode(unsigned char mode)
{
    unsigned char reg1;
    unsigned char reg2;

    switch(mode)
    {
    case GALAX_AMPERE_MODE_STATIC:
        reg1 = GALAX_AMPERE_MODE_STATIC_DATA_1;
        reg2 = GALAX_AMPERE_MODE_STATIC_DATA_2;
        break;

    case GALAX_AMPERE_MODE_BREATHING:
        reg1 = GALAX_AMPERE_MODE_BREATHING_DATA_1;
        reg2 = GALAX_AMPERE_MODE_BREATHING_DATA_2;
        break;

    case GALAX_AMPERE_MODE_BREATHING_CYCLE:
        reg1 = GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_1;
        reg2 = GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_2;
        break;

    case GALAX_AMPERE_MODE_SPECTRUM_CYCLE:
        reg1 = GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_1;
        reg2 = GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_2;
        break;

    case GALAX_AMPERE_MODE_OFF:
        reg1 = GALAX_AMPERE_MODE_OFF_DATA_1;
        reg2 = GALAX_AMPERE_MODE_OFF_DATA_2;
        break;

    default:
        LOG_WARNING("[GALAX GPU Ampere] SetMode: Unknown mode value %d", mode);
        return;
    }

    RegisterWrite(GALAX_AMPERE_MODE_REGISTER_1, reg1);
    RegisterWrite(GALAX_AMPERE_MODE_REGISTER_2, reg2);
}

void GalaxGPUAmpereController::SaveMode()
{
    RegisterWrite(GALAX_AMPERE_SAVE_REGISTER, GALAX_AMPERE_SAVE_DATA);
}

unsigned char GalaxGPUAmpereController::RegisterRead(unsigned char reg)
{
    return(bus->i2c_smbus_read_byte_data(dev, reg));
}

void GalaxGPUAmpereController::RegisterWrite(unsigned char reg, unsigned char val)
{
    bus->i2c_smbus_write_byte_data(dev, reg, val);
}

/*---------------------------------------------------------*\
| GalaxGPUv3Controller.cpp                                  |
|                                                           |
|   RGBController for Galax GPUs (Xtreme Tuner)             |
|                                                           |
|   Daniel Stuart  (daniel.stuart14)            26 may 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstring>
#include "GalaxGPUv3Controller.h"

GalaxGPUv3Controller::GalaxGPUv3Controller(i2c_smbus_interface* bus, galax_gpu_dev_id dev)
{
    this->bus = bus;
    this->dev = dev;

    strcpy(device_name, "Galax RTX GPU"); // Would be nice to get the actual GPU name. Using this as a placeholder.
}

GalaxGPUv3Controller::~GalaxGPUv3Controller()
{

}

std::string GalaxGPUv3Controller::GetDeviceName()
{
    return(device_name);
}

std::string GalaxGPUv3Controller::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    char addr[5];
    snprintf(addr, 5, "0x%02X", dev);
    return_string.append(", address ");
    return_string.append(addr);
    return("I2C: " + return_string);
}

unsigned char GalaxGPUv3Controller::GetLEDRed()
{
    return(GalaxGPURegisterRead(GALAX_V3_RED_REGISTER));
}

unsigned char GalaxGPUv3Controller::GetLEDGreen()
{
    return(GalaxGPURegisterRead(GALAX_V3_GREEN_REGISTER));
}

unsigned char GalaxGPUv3Controller::GetLEDBlue()
{
    return(GalaxGPURegisterRead(GALAX_V3_BLUE_REGISTER));
}

unsigned char GalaxGPUv3Controller::GetMode()
{
    return GalaxGPURegisterRead(GALAX_V3_MODE_REGISTER);
}

unsigned char GalaxGPUv3Controller::GetSync()
{
    return GalaxGPURegisterRead(GALAX_V3_SYNC_REGISTER);
}

unsigned char GalaxGPUv3Controller::GetSpeed()
{
    return GalaxGPURegisterRead(GALAX_V3_SPEED_REGISTER_A);
}

unsigned char GalaxGPUv3Controller::GetBrightness()
{
    return GalaxGPURegisterRead(GALAX_V3_BRIGHTNESS_REGISTER);
}

void GalaxGPUv3Controller::SetLEDColors(unsigned char red, unsigned char green, unsigned char blue)
{
    GalaxGPURegisterWrite(GALAX_V3_RED_REGISTER, red);
    GalaxGPURegisterWrite(GALAX_V3_GREEN_REGISTER, green);
    GalaxGPURegisterWrite(GALAX_V3_BLUE_REGISTER, blue);
}

void GalaxGPUv3Controller::SetMode(unsigned char value)
{
    GalaxGPURegisterWrite(GALAX_V3_MODE_REGISTER, value);
}

void GalaxGPUv3Controller::SetSync(unsigned char value)
{
    GalaxGPURegisterWrite(GALAX_V3_SYNC_REGISTER, value);
}

void GalaxGPUv3Controller::SetSpeed(unsigned char value)
{
    // We just duplicate the value to both speed registers
    GalaxGPURegisterWrite(GALAX_V3_SPEED_REGISTER_A, value);
    GalaxGPURegisterWrite(GALAX_V3_SPEED_REGISTER_B, value);
}

void GalaxGPUv3Controller::SetBrightness(unsigned char value)
{
    GalaxGPURegisterWrite(GALAX_V3_BRIGHTNESS_REGISTER, value);
}

void GalaxGPUv3Controller::SaveMode()
{
    GalaxGPURegisterWrite(GALAX_V3_SAVE_REGISTER, GALAX_V3_SAVE_VALUE);
}

unsigned char GalaxGPUv3Controller::GalaxGPURegisterRead(unsigned char reg)
{
    return(bus->i2c_smbus_read_byte_data(dev, reg));
}

void GalaxGPUv3Controller::GalaxGPURegisterWrite(unsigned char reg, unsigned char val)
{
    bus->i2c_smbus_write_byte_data(dev, reg, val);
}

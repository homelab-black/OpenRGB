/*---------------------------------------------------------*\
| OnixLumiGPUController.cpp                                 |
|                                                           |
|   Driver for ONIX LUMI Intel Arc B580                     |
|                                                           |
|   Jan Dvořák                                  11 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdio>

#include "OnixLumiGPUController.h"

OnixLumiGPUController::OnixLumiGPUController(i2c_smbus_interface* bus, onix_lumi_dev_id dev, std::string name)
{
    this->bus  = bus;
    this->dev  = dev;
    this->name = name;
}

OnixLumiGPUController::~OnixLumiGPUController()
{
}

std::string OnixLumiGPUController::GetDeviceName()
{
    return(name);
}

std::string OnixLumiGPUController::GetDeviceLocation()
{
    std::string return_string(bus->info.device_name);

    char addr[5];
    snprintf(addr, sizeof(addr), "0x%02X", dev);

    return_string.append(", address ");
    return_string.append(addr);

    return("I2C: " + return_string);
}

void OnixLumiGPUController::WriteRegister(u8 reg, u8 value)
{
    /*-----------------------------------------------------*\
    | ONIX LUMI uses raw two-byte I2C writes:               |
    |                                                       |
    |   [ register, value ]                                 |
    |                                                       |
    | Use i2c_write_block() to match verified i2ctransfer   |
    | behaviour rather than SMBus byte-data transactions.   |
    \*-----------------------------------------------------*/
    u8 data[2] = { reg, value };

    bus->i2c_write_block(dev, sizeof(data), data);
}

void OnixLumiGPUController::SetOff()
{
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, 0x00);
}

void OnixLumiGPUController::SetStatic(unsigned char red, unsigned char green, unsigned char blue, unsigned char brightness)
{
    WriteRegister(ONIX_LUMI_REG_MODE,       ONIX_LUMI_HW_MODE_STATIC);
    WriteRegister(ONIX_LUMI_REG_RED,        red);
    WriteRegister(ONIX_LUMI_REG_GREEN,      green);
    WriteRegister(ONIX_LUMI_REG_BLUE,       blue);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
}

void OnixLumiGPUController::SetRainbow(unsigned char brightness, unsigned char speed, unsigned char direction)
{
    if(speed < 2)
    {
        speed = 2;
    }

    if(speed > 13)
    {
        speed = 13;
    }

    if(direction != 1)
    {
        direction = 0;
    }

    WriteRegister(ONIX_LUMI_REG_RAINBOW_RESPONSE, speed);
    WriteRegister(ONIX_LUMI_REG_RAINBOW_PATTERN,
                  ONIX_LUMI_RAINBOW_PATTERN_DEFAULT);
    WriteRegister(ONIX_LUMI_REG_DIRECTION, direction);
    WriteRegister(ONIX_LUMI_REG_MODE, ONIX_LUMI_HW_MODE_RAINBOW);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
}

void OnixLumiGPUController::SetBreathing(unsigned char red, unsigned char green, unsigned char blue, unsigned char brightness, unsigned char speed)
{
    if(speed > ONIX_LUMI_BREATHING_SPEED_MAX)
    {
        speed = ONIX_LUMI_BREATHING_SPEED_MAX;
    }

    unsigned char tempo =
        ONIX_LUMI_BREATHING_SPEED_MAX - speed;

    WriteRegister(ONIX_LUMI_REG_BREATHING_RED,   red);
    WriteRegister(ONIX_LUMI_REG_BREATHING_GREEN, green);
    WriteRegister(ONIX_LUMI_REG_BREATHING_BLUE,  blue);
    WriteRegister(ONIX_LUMI_REG_BREATHING_TEMPO, tempo);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS,      brightness);
    WriteRegister(ONIX_LUMI_REG_MODE,            ONIX_LUMI_HW_MODE_BREATHING);
}


void OnixLumiGPUController::SetRunway(unsigned char brightness, unsigned char speed, unsigned char direction, bool bounce)
{
    if(speed < 2)
    {
        speed = 2;
    }

    if(speed > 13)
    {
        speed = 13;
    }

    if(direction != 1)
    {
        direction = 0;
    }

    unsigned char response = 15 - speed;

    WriteRegister(ONIX_LUMI_REG_RUNWAY_RESPONSE, response);
    WriteRegister(ONIX_LUMI_REG_RUNWAY_INTERVAL, 1);
    WriteRegister(ONIX_LUMI_REG_RUNWAY_BOUNCE, bounce ? 1 : 0);
    WriteRegister(ONIX_LUMI_REG_DIRECTION, direction);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
    WriteRegister(ONIX_LUMI_REG_MODE, ONIX_LUMI_HW_MODE_RUNWAY);
}


void OnixLumiGPUController::SetColorCycle(unsigned char brightness, unsigned char speed)
{
    if(speed < 2)
    {
        speed = 2;
    }

    if(speed > 13)
    {
        speed = 13;
    }

    unsigned char response = 15 - speed;

    WriteRegister(ONIX_LUMI_REG_ONE_COLOR_RESPONSE, response);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
    WriteRegister(ONIX_LUMI_REG_MODE, ONIX_LUMI_HW_MODE_COLOR_CYCLE);
}


void OnixLumiGPUController::SetSerial(unsigned char brightness, unsigned char speed, unsigned char direction)
{
    if(speed > 5)
    {
        speed = 5;
    }

    if(direction != 1)
    {
        direction = 0;
    }

    unsigned char response = 5 - speed;

    WriteRegister(ONIX_LUMI_REG_SERIAL_RESPONSE, response);

    /*-----------------------------------------------------*\
    | One complete colour sweep before starting the next     |
    \*-----------------------------------------------------*/
    WriteRegister(ONIX_LUMI_REG_SERIAL_INTERVAL, 0x10);

    WriteRegister(ONIX_LUMI_REG_DIRECTION, direction);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
    WriteRegister(ONIX_LUMI_REG_MODE, ONIX_LUMI_HW_MODE_SERIAL);
}


void OnixLumiGPUController::SetBlockStacking(unsigned char brightness, unsigned char speed, unsigned char direction)
{
    if(speed < 2)
    {
        speed = 2;
    }

    if(speed > 13)
    {
        speed = 13;
    }

    unsigned char hardware_speed = 15 - speed;

    /*-----------------------------------------------------*\
    | Raw hardware direction:                                |
    | 0 = left-to-right                                      |
    | 1 = right-to-left                                      |
    \*-----------------------------------------------------*/
    if(direction != 1)
    {
        direction = 0;
    }

    WriteRegister(ONIX_LUMI_REG_BLOCK_STACKING_SPEED, hardware_speed);
    WriteRegister(ONIX_LUMI_REG_DIRECTION, direction);
    WriteRegister(ONIX_LUMI_REG_BRIGHTNESS, brightness);
    WriteRegister(ONIX_LUMI_REG_MODE, ONIX_LUMI_HW_MODE_BLOCK_STACKING);
}

/*---------------------------------------------------------*\
| AcerBiFrostGPUController.h                                |
|                                                           |
|   Driver for Acer Predator BiFrost GPU                    |
|                                                           |
|   Baobaosheng                                 18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "i2c_smbus.h"
#include "RGBController.h"

typedef unsigned char   acer_bifrost_dev_id;

/*---------------------------------------------------------*\
| The logo is driven by an ITE Dynamic Lighting MCU that    |
| bridges HID LampArray reports to I2C registers. Register  |
| 0x04/0x05/0x06 take the payload of LampArray report 4/5/6 |
\*---------------------------------------------------------*/
enum
{
    ACER_BIFROST_REG_LAMP_MULTI_UPDATE  = 0x04,
    ACER_BIFROST_REG_LAMP_RANGE_UPDATE  = 0x05,
    ACER_BIFROST_REG_AUTONOMOUS_MODE    = 0x06,
    ACER_BIFROST_REG_LAMP_COUNT         = 0x10
};

enum
{
    ACER_BIFROST_AUTONOMOUS_OFF         = 0x00,
    ACER_BIFROST_AUTONOMOUS_ON          = 0x01
};

#define ACER_BIFROST_NUM_LEDS               6
#define ACER_BIFROST_LAMPS_PER_MULTI_UPDATE 2
#define ACER_BIFROST_LAMP_UPDATE_COMPLETE   0x01
#define ACER_BIFROST_INTENSITY_MAX          0xFF

class AcerBiFrostGPUController
{
public:
    AcerBiFrostGPUController(i2c_smbus_interface* bus, acer_bifrost_dev_id dev, std::string dev_name);
    ~AcerBiFrostGPUController();

    std::string                     GetDeviceLocation();
    std::string                     GetDeviceName();

    int                             GetLampCount();

    void                            SetHostControl();
    void                            SetLedColorAll(RGBColor color);
    void                            SetLedColors(const RGBColor* colors, unsigned int count);

private:
    i2c_smbus_interface*            bus;
    acer_bifrost_dev_id             dev;
    std::string                     name;
    unsigned int                    lamp_count;
    bool                            host_control;
};

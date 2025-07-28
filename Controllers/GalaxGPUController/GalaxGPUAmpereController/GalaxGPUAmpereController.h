/*---------------------------------------------------------*\
| GalaxGPUAmpereController.h                                |
|                                                           |
|   Driver for Galax/KFA2 GPU                               |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "i2c_smbus.h"

typedef unsigned char galax_gpu_dev_id;

enum
{
    GALAX_AMPERE_RED_REGISTER                      = 0x02, /* Red register */
    GALAX_AMPERE_GREEN_REGISTER                    = 0x03, /* Green register */
    GALAX_AMPERE_BLUE_REGISTER                     = 0x04, /* Blue register */
    GALAX_AMPERE_MODE_REGISTER_1                   = 0x05, /* Mode register 1 */
    GALAX_AMPERE_MODE_REGISTER_2                   = 0x06, /* Mode register 2 */
    GALAX_AMPERE_SAVE_REGISTER                     = 0x08, /* Save register */
};

enum
{
    GALAX_AMPERE_MODE_STATIC                      = 0x00, /* Static mode */
    GALAX_AMPERE_MODE_BREATHING                   = 0x01, /* Breathing mode */
    GALAX_AMPERE_MODE_BREATHING_CYCLE             = 0x02, /* Breathing Cycle mode */
    GALAX_AMPERE_MODE_SPECTRUM_CYCLE              = 0x03, /* Spectrum Cycle mode */
    GALAX_AMPERE_MODE_OFF                         = 0x04, /* Off mode */
};

enum
{
    GALAX_AMPERE_MODE_STATIC_DATA_1               = 0x00,
    GALAX_AMPERE_MODE_STATIC_DATA_2               = 0x20,

    GALAX_AMPERE_MODE_BREATHING_DATA_1            = 0x04,
    GALAX_AMPERE_MODE_BREATHING_DATA_2            = 0x20,

    GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_1      = 0x84,
    GALAX_AMPERE_MODE_BREATHING_CYCLE_DATA_2      = 0x40,

    GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_1       = 0x84,
    GALAX_AMPERE_MODE_SPECTRUM_CYCLE_DATA_2       = 0x02,

    GALAX_AMPERE_MODE_OFF_DATA_1                  = 0x00,
    GALAX_AMPERE_MODE_OFF_DATA_2                  = 0x01,

    GALAX_AMPERE_SAVE_DATA                        = 0x5A,
};

class GalaxGPUAmpereController
{
public:
    GalaxGPUAmpereController(i2c_smbus_interface* bus, galax_gpu_dev_id dev);
    ~GalaxGPUAmpereController();

    std::string   GetDeviceName();
    std::string   GetDeviceLocation();

    unsigned char GetLEDRed();
    unsigned char GetLEDGreen();
    unsigned char GetLEDBlue();
    void          SetLEDColors(unsigned char red, unsigned char green, unsigned char blue);

    unsigned char GetMode();
    void          SetMode(unsigned char mode);
    void          SaveMode();

private:
    i2c_smbus_interface* bus;
    galax_gpu_dev_id     dev;
    std::string          device_name;

    unsigned char RegisterRead(unsigned char reg);
    void          RegisterWrite(unsigned char reg, unsigned char val);
};

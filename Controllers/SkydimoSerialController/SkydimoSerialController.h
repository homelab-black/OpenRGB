/*---------------------------------------------------------*\
| SkydimoSerialController.h                                |
|                                                           |
|   Driver for Skydimo serial devices                      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "RGBController.h"
#include "serial_port.h"

class SkydimoSerialController
{
public:
    SkydimoSerialController(const std::string& portname);
    ~SkydimoSerialController();

    bool                    IsPresent();

    void                    SetLEDs(const std::vector<RGBColor>& colors);
    void                    SetBlack();
    void                    SetLEDCount(unsigned int led_count);

    std::string             GetName() const;
    std::string             GetSerial() const;
    std::string             GetLocation() const;

    bool                    IsResizable() const;
    unsigned int            GetLEDCount() const;
    unsigned int            GetMinLEDCount() const;
    unsigned int            GetMaxLEDCount() const;

    unsigned int            GetMatrixWidth() const;
    unsigned int            GetMatrixHeight() const;
    const std::vector<unsigned int>& GetMatrixMap() const;

private:
    bool                    QueryInfo();
    void                    DrainInput();
    void                    ConfigureOutput();
    bool                    WriteFrame(const std::vector<RGBColor>& colors, unsigned int count);

    std::string             port_name;
    std::string             model_name;
    std::string             model_id;
    std::string             serial_id;

    bool                    port_open;
    bool                    present;
    bool                    resizable;

    unsigned int            leds_count;
    unsigned int            leds_min;
    unsigned int            leds_max;
    unsigned int            matrix_width;
    unsigned int            matrix_height;

    std::vector<unsigned int> matrix_map;
    serial_port             serialport;
};

/*---------------------------------------------------------*\
| FanBusController.h                                        |
|                                                           |
|   Driver for FanBus devices                               |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      12 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include "FanBusInterface.h"
#include "RGBController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define FANBUS_CFG_KEY "FanBusDevices"

struct FanBusDevice
{
    std::string port;
};

void to_json(json& j, const FanBusDevice& settings);
void from_json(const json& j, FanBusDevice& settings);

struct FanBusSettings
{
    std::vector<FanBusDevice> devices;
};

void to_json(json& j, const FanBusSettings& settings);
void from_json(const json& j, FanBusSettings& settings);

class FanBusController
{
public:
    FanBusController(FanBusInterface* bus_ptr, unsigned char dev_addr);
    ~FanBusController();

    std::string GetLocation();

    void        SetLEDs(std::vector<RGBColor> colors);

private:
    std::string         port_name;
    FanBusInterface*    bus;
    unsigned char       dev;
};

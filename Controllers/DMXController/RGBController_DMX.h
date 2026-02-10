/*---------------------------------------------------------*\
| RGBController_DMX.h                                       |
|                                                           |
|   RGBController for DMX devices                           |
|                                                           |
|   Adam Honse (CalcProgrammer1)                30 Apr 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include <thread>
#include "RGBController.h"
#include "serial_port.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define DMX_CFG_KEY "DMXDevices"

struct DMXDevice
{
    std::string name;
    std::string port;
    unsigned int keepalive_time = 0;
    unsigned int red_channel;
    unsigned int green_channel;
    unsigned int blue_channel;
    unsigned int brightness_channel;
};

void to_json(json& j, const DMXDevice& settings);
void from_json(const json& j, DMXDevice& settings);

struct DMXSettings
{
    std::vector<DMXDevice> devices;
};

void to_json(json& j, const DMXSettings& settings);
void from_json(const json& j, DMXSettings& settings);

class RGBController_DMX : public RGBController
{
public:
    RGBController_DMX(std::vector<DMXDevice> device_list);
    ~RGBController_DMX();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        KeepaliveThreadFunction();

private:
	std::vector<DMXDevice> 	                            devices;
    serial_port *                                       port;
    std::thread *                                       keepalive_thread;
    std::atomic<bool>                                   keepalive_thread_run;
    std::chrono::milliseconds                           keepalive_delay;
    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;
};

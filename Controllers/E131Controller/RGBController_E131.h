/*---------------------------------------------------------*\
| RGBController_E131.h                                      |
|                                                           |
|   RGBController for E1.31 devices                         |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Oct 2019 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include <thread>
#include <e131.h>
#include "RGBController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum E131RGBOrder
{
    RGB,
    RBG,
    GRB,
    GBR,
    BRG,
    BGR
};

void to_json(json& j, const E131RGBOrder& settings);
void from_json(const json& j, E131RGBOrder& settings);

enum E131MatrixOrder
{
    HORIZONTAL_TOP_LEFT,
    HORIZONTAL_TOP_RIGHT,
    HORIZONTAL_BOTTOM_LEFT,
    HORIZONTAL_BOTTOM_RIGHT,
    VERTICAL_TOP_LEFT,
    VERTICAL_TOP_RIGHT,
    VERTICAL_BOTTOM_LEFT,
    VERTICAL_BOTTOM_RIGHT
};

void to_json(json& j, const E131MatrixOrder& settings);
void from_json(const json& j, E131MatrixOrder& settings);

#define E131_SETTINGS_KEY "E131Devices"

struct E131Device
{
    std::string name = "";
    std::string ip = "";
    unsigned int num_leds = 0;
    unsigned int start_universe = 1;
    unsigned int start_channel = 1;
    unsigned int keepalive_time;
    E131RGBOrder rgb_order = E131RGBOrder::RBG;
    zone_type type = ZONE_TYPE_SINGLE;
    unsigned int matrix_width = 0;
    unsigned int matrix_height = 0;
    unsigned int universe_size = 512;
    E131MatrixOrder matrix_order = E131MatrixOrder::HORIZONTAL_TOP_LEFT;
};

void to_json(json& j, const E131Device& settings);
void from_json(const json& j, E131Device& settings);

struct E131Settings
{
    std::vector<E131Device> devices;
};

void to_json(json& j, const E131Settings& settings);
void from_json(const json& j, E131Settings& settings);

class RGBController_E131 : public RGBController
{
public:
    RGBController_E131(std::vector<E131Device> device_list);
    ~RGBController_E131();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        KeepaliveThreadFunction();

private:
	std::vector<E131Device> 	devices;
    std::vector<e131_packet_t> 	packets;
	std::vector<e131_addr_t> 	dest_addrs;
	std::vector<unsigned int> 	universes;
	int 						sockfd;
    std::thread *               keepalive_thread;
    std::atomic<bool>           keepalive_thread_run;
    std::chrono::milliseconds                           keepalive_delay;
    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;
};

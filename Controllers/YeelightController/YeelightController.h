/*---------------------------------------------------------*\
| YeelightController.h                                      |
|                                                           |
|   Driver for Yeelight                                     |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include "RGBController.h"
#include "net_port.h"

#define YEELIGHT_BRIGHTNESS_MAX   100
#define YEELIGHT_BRIGHTNESS_MIN   1

enum
{
    YEELIGHT_MODE_DIRECT = 0,
    YEELIGHT_MODE_WHITE2000K = 1,
    YEELIGHT_MODE_WHITE3000K = 2,
    YEELIGHT_MODE_WHITE4000K = 3,
    YEELIGHT_MODE_WHITE5000K = 4,
    YEELIGHT_MODE_WHITE6500K = 5
};

enum yeelight_protocol
{
    YEELIGHT_PROTOCOL_TCP,
    YEELIGHT_PROTOCOL_MUSIC,
    YEELIGHT_PROTOCOL_UDP
};

class YeelightController
{
public:
    YeelightController(std::string ip, std::string host_ip, yeelight_protocol protocol);
    ~YeelightController();

    std::string GetLocation();
    std::string GetName();
    std::string GetVersion();
    std::string GetManufacturer();
    std::string GetUniqueID();

    yeelight_protocol GetProtocol();
    static yeelight_protocol GetProtocolFromSettings(const nlohmann::json& settings);

    bool StartUDPSession();
    void SendKeepAlivePackets();
    void ListenForErrorsFunction();
    void SetPower(bool power = true, int transition_time = 50);
    void SetColor(unsigned char red, unsigned char green, unsigned char blue);
    void SetColorTemperature(int color_temperature, int brightness);

private:
    bool StartMusicMode();
    bool SetMusicMode();
    bool SendTCPCommand(const std::string& command);

    yeelight_protocol   protocol;
    std::string         host_ip;
    unsigned int        music_mode_port;
    net_port            music_mode_server;
    SOCKET*             music_mode_sock;
    std::string         location;
    std::string         token;
    net_port            port;
    std::chrono::steady_clock::time_point last_command_time;
    std::chrono::steady_clock::time_point last_keepalive_time;
    unsigned int        session_request_id;
    unsigned int        missed_keepalives;
    bool                keepalive_pending;
    std::mutex          state_mutex;
    std::condition_variable stop_condition;
    std::atomic<bool>   session_active;
    std::atomic<bool>   stop_requested;
    std::thread         listen_thread;
    std::thread         keepalive_thread;
    bool                port_open;
    bool                initial_power_pending;
    bool                current_power_state;
};

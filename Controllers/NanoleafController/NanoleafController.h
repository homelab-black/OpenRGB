/*---------------------------------------------------------*\
| NanoleafController.h                                      |
|                                                           |
|   Driver for Nanoleaf                                     |
|                                                           |
|   Nikita Rushmanov                            13 Jan 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "net_port.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define NANOLEAF_DIRECT_MODE_EFFECT_NAME    "*Dynamic*"
#define NANOLEAF_LIGHT_PANELS_MODEL         "NL22"
#define NANOLEAF_CANVAS_MODEL               "NL29"
#define NANOLEAF_SHAPES_MODEL               "NL42"

#define NANOLEAF_CFG_KEY "NanoleafDevices"

struct NanoleafDeviceSettings
{
    std::string ip;
    unsigned int port;
    std::string auth_token;
};

void to_json(json& j, const NanoleafDeviceSettings& settings);
void from_json(const json& j, NanoleafDeviceSettings& settings);

struct NanoleafSettings
{
    /*---------------------------------------------------------------*\
    | Map of devices, key is the location of the device               |
    | The location is defined as "<ip>:<port>",                       |
    | where <ip> and <port> are the device's ip and port respectively |
    \*---------------------------------------------------------------*/
    std::unordered_map<std::string, NanoleafDeviceSettings> devices;
};

void to_json(json& j, const NanoleafSettings& settings);
void from_json(const json& j, NanoleafSettings& settings);


class NanoleafController
{
public:
    NanoleafController(std::string a_address, int a_port, std::string a_auth_token);

    static std::string          Pair(std::string address, int port);
    static void                 Unpair(std::string address, int port, std::string auth_token);

    void                        SelectEffect(std::string effect_name);
    void                        StartExternalControl();
    void                        SetBrightness(int a_brightness);
    void                        UpdateLEDs(std::vector<RGBColor>& colors);

    std::string                 GetAuthToken();
    std::string                 GetName();
    std::string                 GetSerial();
    std::string                 GetManufacturer();
    std::string                 GetFirmwareVersion();
    std::string                 GetModel();
    std::vector<std::string>&   GetEffects();
    std::vector<int>&           GetPanelIds();
    std::string                 GetSelectedEffect();
    int                         GetBrightness();

private:
    net_port                    external_control_socket;

    std::string                 address;
    int                         port;
    std::string                 location;
    std::string                 auth_token;

    std::string                 name;
    std::string                 serial;
    std::string                 manufacturer;
    std::string                 firmware_version;
    std::string                 model;

    std::vector<std::string>    effects;
    std::vector<int>            panel_ids;

    std::string                 selectedEffect;
    int                         brightness;
};

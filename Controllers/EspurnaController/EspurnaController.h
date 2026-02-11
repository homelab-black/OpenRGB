/*---------------------------------------------------------*\
| EspurnaController.h                                       |
|                                                           |
|   Driver for Espurna                                      |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      11 Sep 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include "RGBController.h"
#include "net_port.h"
#include <nlohmann/json.hpp>

/*--------------------------*\
| what's wrong with `true`? |
\*--------------------------*/
#ifndef TRUE
#define TRUE true
#define FALSE false
#endif

#ifndef WIN32
#define LPSTR           char *
#define strtok_s        strtok_r
#endif

#define ESPURNA_CFG_KEY "EspurnaDevices"

struct EspurnaDevice
{
    std::string ip;
    /*---------------------------------*\
    | using a string for port to       |
    | preserve backwards compatibility |
    \*---------------------------------*/
    std::string port;
    std::string apikey;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EspurnaDevice,
    ip,
    port,
    apikey
);

struct EspurnaSettings
{
    std::vector<EspurnaDevice> devices;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(EspurnaSettings, devices);

class EspurnaController
{
public:
    EspurnaController();
    ~EspurnaController();

    void        Initialize(char* ledstring);
    void        InitializeEspurna(char* clientname, char* port, char * apikey);

    std::string GetLocation();

    void        SetLEDs(std::vector<RGBColor> colors);

private:
    std::string port_name;
    std::string client_name;
    char espurna_apikey[128];

    net_port *tcpport;
};

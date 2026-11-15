/*---------------------------------------------------------*\
| SteelSeriesGameSense.cpp                                  |
|                                                           |
|   Sends 8-zone keyboard lighting through SteelSeries      |
|   Engine's local GameSense API.                           |
|                                                           |
|   xmo4                                        12 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include "httplib.h"
#include "LogManager.h"
#include "SteelSeriesGameSense.h"

using json = nlohmann::json;
using namespace std::chrono_literals;

#define GAMESENSE_GAME              "OPENRGB"
#define GAMESENSE_EVENT             "LIGHTING"
#define GAMESENSE_HOST              "127.0.0.1"
#define GAMESENSE_ZONE_COUNT        8
#define GAMESENSE_DEINIT_MS         60000
#define GAMESENSE_RETRY_PERIOD      5s
#define GAMESENSE_KEEPALIVE_PERIOD  2s
#define GAMESENSE_MAX_PROPS_SIZE    4096

static const char* zone_names[GAMESENSE_ZONE_COUNT] =
{
    "one", "two", "three", "four", "five", "six", "seven", "eight"
};

static std::string ReadCoreProps()
{
#ifdef _WIN32
    const char* program_data = std::getenv("PROGRAMDATA");

    if(program_data == NULL)
    {
        return("");
    }

    std::ifstream file(std::string(program_data) + "\\SteelSeries\\SteelSeries Engine 3\\coreProps.json", std::ios::binary);
    std::string   contents(GAMESENSE_MAX_PROPS_SIZE, '\0');

    file.read(&contents[0], GAMESENSE_MAX_PROPS_SIZE);
    contents.resize((size_t)file.gcount());
    return(contents);
#else
    /*-----------------------------------------------------*\
    | Only implemented for Windows; elsewhere the device is |
    | always driven directly.                               |
    \*-----------------------------------------------------*/
    return("");
#endif
}

bool SteelSeriesGameSense::ParseLoopbackAddress(const std::string& core_props, int& port)
{
    json props = json::parse(core_props, nullptr, false);

    if(!props.is_object() || !props.contains("address") || !props["address"].is_string())
    {
        return(false);
    }

    const std::string address   = props["address"].get<std::string>();
    const std::string prefix    = GAMESENSE_HOST ":";

    if(address.compare(0, prefix.size(), prefix) != 0)
    {
        return(false);
    }

    const std::string port_str  = address.substr(prefix.size());

    if(port_str.empty() || port_str.size() > 5 || port_str.find_first_not_of("0123456789") != std::string::npos)
    {
        return(false);
    }

    int value = std::stoi(port_str);

    if(value < 1 || value > 65535)
    {
        return(false);
    }

    port = value;
    return(true);
}

SteelSeriesGameSense::SteelSeriesGameSense(GameSenseLostCallback callback, void* callback_arg)
{
    lost_callback           = callback;
    lost_callback_arg       = callback_arg;
    port                    = 0;
    event_value             = 0;
    registered              = false;
    active                  = false;

    keepalive_thread_run    = true;
    keepalive_thread        = std::thread(&SteelSeriesGameSense::KeepaliveThreadFunction, this);
}

SteelSeriesGameSense::~SteelSeriesGameSense()
{
    keepalive_thread_run = false;
    keepalive_thread.join();

    Stop();
}

bool SteelSeriesGameSense::Post(const char* uri, const std::string& body)
{
    if(port == 0)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Short timeouts, Engine is local and this is called    |
    | from the device update path                           |
    \*-----------------------------------------------------*/
    httplib::Client client(GAMESENSE_HOST, port);

    client.set_connection_timeout(0, 300000);
    client.set_read_timeout(1, 0);
    client.set_write_timeout(1, 0);

    httplib::Result result = client.Post(uri, body, "application/json");

    return(result && result->status == 200);
}

bool SteelSeriesGameSense::Register()
{
    registered = false;

    if(std::chrono::steady_clock::now() < retry_after)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Re-read every attempt: Engine picks a new port each   |
    | time it starts.                                       |
    \*-----------------------------------------------------*/
    if(!ParseLoopbackAddress(ReadCoreProps(), port))
    {
        port        = 0;
        retry_after = std::chrono::steady_clock::now() + GAMESENSE_RETRY_PERIOD;
        return(false);
    }

    json metadata =
    {
        { "game",                           GAMESENSE_GAME      },
        { "game_display_name",              "OpenRGB"           },
        { "developer",                      "OpenRGB"           },
        { "deinitialize_timer_length_ms",   GAMESENSE_DEINIT_MS }
    };

    json handlers = json::array();

    for(unsigned int zone_idx = 0; zone_idx < GAMESENSE_ZONE_COUNT; zone_idx++)
    {
        handlers.push_back(
        {
            { "device-type",        "rgb-8-zone"                        },
            { "zone",               zone_names[zone_idx]                },
            { "mode",               "context-color"                     },
            { "context-frame-key",  "z" + std::to_string(zone_idx)      }
        });
    }

    json binding =
    {
        { "game",           GAMESENSE_GAME  },
        { "event",          GAMESENSE_EVENT },
        { "value_optional", true            },
        { "handlers",       handlers        }
    };

    if(!Post("/game_metadata", metadata.dump()) || !Post("/bind_game_event", binding.dump()))
    {
        retry_after = std::chrono::steady_clock::now() + GAMESENSE_RETRY_PERIOD;
        return(false);
    }

    LOG_INFO("[SteelSeries GameSense] Registered with Engine at %s:%d", GAMESENSE_HOST, port);
    registered = true;
    return(true);
}

bool SteelSeriesGameSense::SendLastColors()
{
    json frame = json::object();

    for(unsigned int zone_idx = 0; zone_idx < GAMESENSE_ZONE_COUNT && zone_idx < last_colors.size(); zone_idx++)
    {
        frame["z" + std::to_string(zone_idx)] =
        {
            { "red",   RGBGetRValue(last_colors[zone_idx]) },
            { "green", RGBGetGValue(last_colors[zone_idx]) },
            { "blue",  RGBGetBValue(last_colors[zone_idx]) }
        };
    }

    /*-----------------------------------------------------*\
    | Value changes every event so Engine re-renders it.    |
    \*-----------------------------------------------------*/
    event_value = (event_value % 100000) + 1;

    json event =
    {
        { "game",  GAMESENSE_GAME  },
        { "event", GAMESENSE_EVENT },
        { "data",  { { "value", event_value }, { "frame", frame } } }
    };

    return(Post("/game_event", event.dump()));
}

bool SteelSeriesGameSense::SendColors(const std::vector<RGBColor>& colors)
{
    std::lock_guard<std::mutex> lock(mutex);

    last_colors = colors;

    /*-----------------------------------------------------*\
    | One re-register attempt covers an Engine restart      |
    | (new port, registration lost).                        |
    \*-----------------------------------------------------*/
    bool sent = (registered || Register()) && SendLastColors();

    if(!sent && registered)
    {
        sent = Register() && SendLastColors();
    }

    if(!sent)
    {
        registered = false;
    }

    /*-----------------------------------------------------*\
    | Stay active even if Engine isn't up yet (service      |
    | starts before login): the keepalive thread keeps      |
    | retrying and applies these colors once it appears.    |
    \*-----------------------------------------------------*/
    active = true;
    return(sent);
}

void SteelSeriesGameSense::Stop()
{
    std::lock_guard<std::mutex> lock(mutex);

    if(active && registered)
    {
        Post("/stop_game", json({ { "game", GAMESENSE_GAME } }).dump());
    }

    active = false;
}

void SteelSeriesGameSense::KeepaliveThreadFunction()
{
    while(keepalive_thread_run.load())
    {
        std::this_thread::sleep_for(GAMESENSE_KEEPALIVE_PERIOD);

        bool lost = false;

        {
            std::lock_guard<std::mutex> lock(mutex);

            if(!keepalive_thread_run.load() || !active)
            {
                continue;
            }

            /*---------------------------------------------*\
            | Re-send the colors rather than a bare         |
            | heartbeat: it also keeps the event alive, and |
            | it undoes Engine/Prism overriding them (e.g.  |
            | Prism loading its profile after we registered |
            | at login). If Engine went away, re-register   |
            | first.                                        |
            \*---------------------------------------------*/
            bool was_registered = registered;

            if(!(registered && SendLastColors()))
            {
                if(!(Register() && SendLastColors()))
                {
                    registered = false;
                }
            }

            lost = was_registered && !registered;
        }

        /*-------------------------------------------------*\
        | Engine resets the keyboard to its own effect when |
        | it exits. Call back without holding the mutex,    |
        | the owner may call SendColors() from it.          |
        \*-------------------------------------------------*/
        if(lost)
        {
            LOG_INFO("[SteelSeries GameSense] Lost Engine, writing the device directly");

            if(lost_callback != NULL)
            {
                lost_callback(lost_callback_arg);
            }
        }
    }
}

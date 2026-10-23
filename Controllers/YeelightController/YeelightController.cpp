/*---------------------------------------------------------*\
| YeelightController.cpp                                    |
|                                                           |
|   Driver for Yeelight                                     |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "YeelightController.h"
#include <nlohmann/json.hpp>
#include "LogManager.h"

using json = nlohmann::json;

YeelightController::YeelightController(std::string ip, std::string host_ip, yeelight_protocol protocol)
    : protocol(protocol)
    , host_ip(host_ip)
    , music_mode_port(55444)
    , music_mode_sock(nullptr)
    , session_request_id(0)
    , missed_keepalives(0)
    , keepalive_pending(false)
    , session_active(false)
    , stop_requested(false)
    , port_open(false)
    , initial_power_pending(true)
    , current_power_state(false)
{
    /*-----------------------------------------------------------------*\
    | Fill in location string with device's IP address                  |
    \*-----------------------------------------------------------------*/
    location    = "IP: " + ip;

    if(protocol != YEELIGHT_PROTOCOL_UDP)
    {
        port_open = port.tcp_client(ip.c_str(), "55443");

        if(port_open)
        {
            SetPower();

            if(protocol == YEELIGHT_PROTOCOL_MUSIC && !StartMusicMode())
            {
                LOG_WARNING("[YeelightController] Music mode unavailable. Using TCP.");
                this->protocol = YEELIGHT_PROTOCOL_TCP;
            }
        }

        return;
    }

    /*-----------------------------------------------------------------*
    | Open a UDP client sending to the device's IP, port 55444          |
    \*-----------------------------------------------------------------*/
    port_open = port.udp_client(ip.c_str(), "55444");

    if(!port_open)
    {
        LOG_ERROR("[YeelightController] Failed to open UDP socket for %s.", ip.c_str());
        return;
    }

    /*---------------------------------------------------------*\
    | Only accept UDP responses from this device.              |
    \*---------------------------------------------------------*/
    addrinfo  hints   = {};
    addrinfo* address = nullptr;
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if(getaddrinfo(ip.c_str(), "55444", &hints, &address) != 0)
    {
        port_open = false;
    }
    else
    {
        port_open = (connect(port.sock, address->ai_addr, (int)address->ai_addrlen) != SOCKET_ERROR);
        freeaddrinfo(address);
    }

    if(!port_open)
    {
        LOG_ERROR("[YeelightController] Failed to set UDP peer for %s.", ip.c_str());
        port.tcp_close();
        return;
    }

    port.set_receive_timeout(1, 0);

    /*-----------------------------------------------------------------*
    | Initialize session and power state                                |
    \*-----------------------------------------------------------------*/
    last_command_time = std::chrono::steady_clock::now();

    if(StartUDPSession())
    {
        SetPower();

        std::lock_guard<std::mutex> lock(state_mutex);
        initial_power_pending = false;
    }

    keepalive_thread = std::thread(&YeelightController::SendKeepAlivePackets, this);
    listen_thread    = std::thread(&YeelightController::ListenForErrorsFunction, this);
}

YeelightController::~YeelightController()
{
    {
        std::lock_guard<std::mutex> lock(state_mutex);
        stop_requested = true;
        session_active = false;
    }

    stop_condition.notify_all();

    if(listen_thread.joinable())
    {
        listen_thread.join();
    }

    if(keepalive_thread.joinable())
    {
        keepalive_thread.join();
    }

    if(music_mode_sock != nullptr)
    {
        closesocket(*music_mode_sock);
        delete music_mode_sock;
    }

    if(port_open && protocol == YEELIGHT_PROTOCOL_UDP)
    {
        port.tcp_close();
    }
}

std::string YeelightController::GetLocation()
{
    return(location);
}

std::string YeelightController::GetName()
{
    return("Yeelight");
}

std::string YeelightController::GetVersion()
{
    return("");
}

std::string YeelightController::GetManufacturer()
{
    return("Yeelight");
}

std::string YeelightController::GetUniqueID()
{
    return("");
}

yeelight_protocol YeelightController::GetProtocol()
{
    return(protocol);
}

yeelight_protocol YeelightController::GetProtocolFromSettings(const json& settings)
{
    if(settings.contains("protocol"))
    {
        if(settings["protocol"] == "Music")
        {
            return(YEELIGHT_PROTOCOL_MUSIC);
        }

        if(settings["protocol"] == "UDP")
        {
            return(YEELIGHT_PROTOCOL_UDP);
        }

        return(YEELIGHT_PROTOCOL_TCP);
    }

    /*---------------------------------------------------------*\
    | Preserve the protocol selected by older settings files.  |
    \*---------------------------------------------------------*/
    if(settings.contains("music_mode") && settings["music_mode"] == true)
    {
        return(YEELIGHT_PROTOCOL_MUSIC);
    }

    return(YEELIGHT_PROTOCOL_TCP);
}

bool YeelightController::SendTCPCommand(const std::string& command)
{
    if(!port_open || !port.tcp_client_connect())
    {
        return(false);
    }

    int written = port.tcp_client_write((char*)command.c_str(), (int)command.length());
    port.tcp_close();
    return(written == (int)command.length());
}

bool YeelightController::StartMusicMode()
{
    for(music_mode_port = 55444; music_mode_port <= 65535; music_mode_port++)
    {
        char port_string[8];
        snprintf(port_string, sizeof(port_string), "%u", music_mode_port);

        music_mode_server.sock = INVALID_SOCKET;
        if(!music_mode_server.tcp_server(port_string))
        {
            if(music_mode_server.sock != INVALID_SOCKET)
            {
                music_mode_server.tcp_close();
            }
            continue;
        }

        /*---------------------------------------------------------*\
        | Listen before requesting the bulb's callback connection. |
        | Limit the wait so an unavailable bulb cannot block scan. |
        \*---------------------------------------------------------*/
        bool connected = false;
        if(listen(music_mode_server.sock, 1) != SOCKET_ERROR && SetMusicMode())
        {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(music_mode_server.sock, &read_fds);
            timeval timeout = { 5, 0 };

            if(select((int)music_mode_server.sock + 1, &read_fds, nullptr, nullptr, &timeout) > 0)
            {
                music_mode_sock = music_mode_server.tcp_server_listen();
                connected = (*music_mode_sock != INVALID_SOCKET);
            }
        }

        music_mode_server.tcp_close();
        return(connected);
    }

    return(false);
}

bool YeelightController::SetMusicMode()
{
    json command;
    std::string ip_addr = host_ip;

    if(ip_addr.empty())
    {
        char hostname[256];
        if(gethostname(hostname, sizeof(hostname)) != 0)
        {
            return(false);
        }

        struct hostent* host_entry = gethostbyname(hostname);
        if(host_entry == nullptr || host_entry->h_addr_list[0] == nullptr)
        {
            return(false);
        }

        ip_addr = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    }

    command["id"]        = 1;
    command["method"]    = "set_music";
    command["params"][0] = 1;
    command["params"][1] = ip_addr;
    command["params"][2] = music_mode_port;

    return(SendTCPCommand(command.dump() + "\r\n"));
}

bool YeelightController::StartUDPSession()
{
    if(session_active)
    {
        LOG_DEBUG("[YeelightController] Session already active. Skipping reinitialization.");
        return(true);
    }

    if(stop_requested || !port_open)
    {
        return(false);
    }

    char recv_buf[1024];
    session_active = false;

    json command;

    command["id"]     = session_request_id++;
    command["method"] = "udp_sess_new";
    command["params"] = json::array();

    std::string command_str = command.dump() + "\r\n";

    {
        std::lock_guard<std::mutex> lock(state_mutex);

        if(stop_requested)
        {
            return(false);
        }

        if(port.udp_write((char*)command_str.c_str(), (int)command_str.length()) <= 0)
        {
            LOG_ERROR("[YeelightController] Failed to send session initialization command.");
            return(false);
        }

        last_command_time = std::chrono::steady_clock::now();
    }

    std::chrono::steady_clock::time_point deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);

    while(!stop_requested && std::chrono::steady_clock::now() < deadline)
    {
        int size = port.udp_listen(recv_buf, sizeof(recv_buf));

        if(size <= 0)
        {
            break;
        }

        json token_response = json::parse(std::string(recv_buf, size), nullptr, false);

        if(!token_response.is_object() || token_response.contains("error")
        || !token_response.contains("id") || !token_response["id"].is_number_integer()
        || token_response["id"] != command["id"]
        || !token_response.contains("method") || token_response["method"] != "udp_sess_token"
        || !token_response.contains("params") || !token_response["params"].is_object()
        || !token_response["params"].contains("token") || !token_response["params"]["token"].is_string())
        {
            continue;
        }

        std::string session_token = token_response["params"]["token"].get<std::string>();

        if(session_token.empty())
        {
            continue;
        }

        std::lock_guard<std::mutex> lock(state_mutex);

        if(stop_requested)
        {
            return(false);
        }

        token             = session_token;
        last_command_time = std::chrono::steady_clock::now();
        missed_keepalives = 0;
        keepalive_pending = false;
        session_active    = true;

        LOG_DEBUG("[YeelightController] Session successfully initialized.");
        return(true);
    }

    if(!stop_requested)
    {
        LOG_ERROR("[YeelightController] Failed to initialize session. Retrying...");
    }

    return(false);
}

void YeelightController::SendKeepAlivePackets()
{
    const int keep_alive_interval = 10;

    json command;

    LOG_DEBUG("[YeelightController] KeepAlive thread started.");

    std::unique_lock<std::mutex> lock(state_mutex);

    while(!stop_requested)
    {
        stop_condition.wait_for(lock, std::chrono::seconds(1), [this]()
        {
            return(stop_requested.load());
        });

        if(stop_requested)
        {
            break;
        }

        if(!session_active)
        {
            continue;
        }

        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

        if(keepalive_pending && now - last_keepalive_time >= std::chrono::seconds(keep_alive_interval))
        {
            keepalive_pending = false;
            missed_keepalives++;

            if(missed_keepalives > 4)
            {
                LOG_WARNING("[YeelightController] No keep-alive response. Restarting session.");
                session_active = false;
                continue;
            }
        }

        /*---------------------------------------------------------*\
        | Send keep-alive only when no other command was sent.     |
        \*---------------------------------------------------------*/
        if(std::chrono::duration_cast<std::chrono::seconds>(now - last_command_time).count() >= keep_alive_interval)
        {
            /*-----------------------------------------------------------------*\
            | Send keep-alive packet if the interval has elapsed                |
            \*-----------------------------------------------------------------*/
            command["id"] = 1;
            command["method"] = "udp_sess_keep_alive";
            command["params"] = { "keeplive_interval", keep_alive_interval };
            command["token"] = token;

            std::string command_str = command.dump() + "\r\n";

            if(port.udp_write((char*)command_str.c_str(), (int)command_str.length()) > 0)
            {
                last_command_time   = now;
                last_keepalive_time = now;
                keepalive_pending   = true;
            }
            else
            {
                LOG_ERROR("[YeelightController] Failed to send keep-alive packet.");
            }
        }
    }

    LOG_DEBUG("[YeelightController] KeepAlive packet sender exiting.");
}

void YeelightController::ListenForErrorsFunction()
{
    char recv_buf[1024];

    LOG_DEBUG("[YeelightController] Error listener started.");

    while(!stop_requested)
    {
        if(!session_active)
        {
            if(StartUDPSession())
            {
                bool power_on_initial_session;

                {
                    std::lock_guard<std::mutex> lock(state_mutex);
                    power_on_initial_session = initial_power_pending;
                    initial_power_pending    = false;
                }

                if(power_on_initial_session)
                {
                    SetPower();
                }
            }
            else
            {
                std::unique_lock<std::mutex> lock(state_mutex);
                stop_condition.wait_for(lock, std::chrono::seconds(5), [this]()
                {
                    return(stop_requested.load());
                });
            }

            continue;
        }

        /*-----------------------------------------------------------------*\
        | Receive packets with error                                        |
        \*-----------------------------------------------------------------*/
        int size = port.udp_listen(recv_buf, 1024);

        if(size > 0)
        {
            std::string response(recv_buf, size);

            try
            {
                /*-----------------------------------------------------------------*\
                | Convert response to JSON                                          |
                \*-----------------------------------------------------------------*/
                json response_json = json::parse(response);

                std::lock_guard<std::mutex> lock(state_mutex);

                if(!session_active || stop_requested)
                {
                    continue;
                }

                /*-----------------------------------------------------------------*\
                | Check for error in the response                                   |
                \*-----------------------------------------------------------------*/
                if(response_json.contains("error"))
                {
                    LOG_ERROR("[YeelightController] Invalid session detected. Restarting...");
                    session_active = false;
                }
                else if(keepalive_pending
                     && response_json.contains("id") && response_json["id"].is_number_integer()
                     && response_json["id"] == 1
                     && response_json.contains("token") && response_json["token"] == token
                     && response_json.contains("result") && response_json["result"] == json::array({ "ok" }))
                {
                    missed_keepalives = 0;
                    keepalive_pending = false;
                }
            }
            catch(const std::exception& e)
            {
                LOG_ERROR("[YeelightController] Failed to parse response: %s", e.what());
            }
        }
    }

    LOG_DEBUG("[YeelightController] Listening thread exiting.");
}

void YeelightController::SetPower(bool power, int transition_time)
{
    if(protocol != YEELIGHT_PROTOCOL_UDP)
    {
        json command;
        command["id"]        = 1;
        command["method"]    = "set_power";
        command["params"]    = { power ? "on" : "off", "sudden", 0, 2 };
        SendTCPCommand(command.dump() + "\r\n");
        return;
    }

    if(!session_active)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(state_mutex);

    if(!session_active || stop_requested)
    {
        return;
    }

    if(current_power_state == power)
    {
        LOG_DEBUG("[YeelightController] Skipping SetPower: already in the desired state.");
        return;
    }

    json command;

    /*-----------------------------------------------------------------*\
    | Fill in the set_rgb command with RGB information.                 |
    | The bulb will not respond to 0, 0, 0, so if all channels are zero,|
    | set the state to off.  Otherwise, set it to on.                   |
    \*-----------------------------------------------------------------*/
    command["id"]               = 1;
    command["method"]           = "set_power";
    command["params"][0]        = power ? "on" : "off";
    command["params"][1]        = "sudden";
    command["params"][2]        = transition_time;
    command["params"][3]        = 2;
    command["token"]            = token;

    /*-----------------------------------------------------------------*\
    | Convert the JSON object to a string and write it                  |
    \*-----------------------------------------------------------------*/
    std::string command_str     = command.dump().append("\r\n");

    if(port.udp_write((char*)command_str.c_str(), (int)command_str.length()) > 0)
    {
        last_command_time = std::chrono::steady_clock::now();
        current_power_state = power;
    }
    else
    {
        LOG_ERROR("[YeelightController] Failed to send power command.");
    }
}

void YeelightController::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    if(protocol == YEELIGHT_PROTOCOL_UDP && !session_active)
    {
        return;
    }

    json command;

    /*-----------------------------------------------------------------*\
    | Yeelight doesn't seem to support proper RGB, it just uses RGB to  |
    | calculate hue and saturation.  It doesn't affect brightness.  To  |
    | work around this, determine the highest value and scale to 100 to |
    | use as brightness                                                 |
    \*-----------------------------------------------------------------*/
    float bright = red;

    if(green > bright)
    {
        bright          = green;
    }

    if(blue > bright)
    {
        bright          = blue;
    }

    bright = (100.0f * (bright / 255.0f));

    if(protocol == YEELIGHT_PROTOCOL_UDP && bright == 0)
    {
        SetPower(false); // Turn off the light with transition time
        return;
    }

    /*-----------------------------------------------------------------*\
    | Calculate the RGB field as 0x00RRGGBB                             |
    \*-----------------------------------------------------------------*/
    unsigned int rgb = (red << 16) | (green << 8) | (blue << 0);

    if(protocol != YEELIGHT_PROTOCOL_UDP)
    {
        /*---------------------------------------------------------*\
        | Retain the single-frame color flow used by TCP and Music |
        | devices, including brightness and black color handling.  |
        \*---------------------------------------------------------*/
        std::string cf = "50,1," + std::to_string(rgb) + "," + std::to_string((int)bright);
        command["id"]        = 1;
        command["method"]    = "start_cf";
        command["params"]    = { 1, 1, cf };
        std::string command_str = command.dump() + "\r\n";

        std::lock_guard<std::mutex> lock(state_mutex);
        if(protocol == YEELIGHT_PROTOCOL_MUSIC && music_mode_sock != nullptr)
        {
            send(*music_mode_sock, command_str.c_str(), (int)command_str.length(), 0);
        }
        else
        {
            SendTCPCommand(command_str);
        }
        return;
    }

    /*-----------------------------------------------------------------*\
    | Fill in the set_scene command with the color.                     |
    \*-----------------------------------------------------------------*/
    command["id"]               = 1;
    command["method"]           = "set_scene";
    command["params"][0] = "color";
    command["params"][1] = rgb;
    command["params"][2] = (int)bright < YEELIGHT_BRIGHTNESS_MIN ? YEELIGHT_BRIGHTNESS_MIN : (int)bright;

    std::lock_guard<std::mutex> lock(state_mutex);

    if(!session_active || stop_requested)
    {
        return;
    }

    command["token"] = token;

    /*-----------------------------------------------------------------*\
    | Convert the JSON object to a string and write it                  |
    \*-----------------------------------------------------------------*/
    std::string command_str     = command.dump().append("\r\n");

    if(port.udp_write((char*)command_str.c_str(), (int)command_str.length()) > 0)
    {
        last_command_time   = std::chrono::steady_clock::now();
        current_power_state = true;
    }
    else
    {
        LOG_ERROR("[YeelightController] Failed to send color command.");
    }
}

void YeelightController::SetColorTemperature(int temperature, int brightness)
{
    if(!session_active)
    {
        return;
    }

    json command;

    /*-----------------------------------------------------------------*
    | Construct color temperature control command                      |
    \*-----------------------------------------------------------------*/
    command["id"]        = 1;
    command["method"]    = "set_scene";
    command["params"][0] = "ct";
    command["params"][1] = temperature;
    command["params"][2] = brightness;

    std::lock_guard<std::mutex> lock(state_mutex);

    if(!session_active || stop_requested)
    {
        return;
    }

    command["token"] = token;

    std::string command_str = command.dump().append("\r\n");

    if(port.udp_write((char*)command_str.c_str(), (int)command_str.length()) > 0)
    {
        last_command_time   = std::chrono::steady_clock::now();
        current_power_state = true;
        LOG_DEBUG("[YeelightController] Color temperature set to %dK.", temperature);
    }
    else
    {
        LOG_ERROR("[YeelightController] Failed to send color temperature command.");
    }
}

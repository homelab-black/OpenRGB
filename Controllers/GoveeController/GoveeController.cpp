/*---------------------------------------------------------*\
| GoveeController.cpp                                       |
|                                                           |
|   Driver for Govee wireless lighting devices              |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      01 Dec 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include "base64.hpp"
#include "GoveeController.h"
#include "JsonUtils.h"

using json = nlohmann::json;
using namespace std::chrono_literals;

#define GOVEE_STATUS_TIMEOUT    30
#define GOVEE_FRAME_TIMEOUT     55

base64::byte CalculateXorChecksum(std::vector<base64::byte> packet)
{
    base64::byte checksum = 0;

    for(unsigned int i = 0; i < packet.size(); i++)
    {
        checksum ^= packet[i];
    }

    return(checksum);
}

GoveeController::GoveeController(std::string ip)
{
    /*-----------------------------------------------------*\
    | Fill in location string with device's IP address      |
    \*-----------------------------------------------------*/
    ip_address          = ip;
    broadcast_received  = false;
    status_known        = false;
    powered             = true;
    recovery_pending    = false;
    frame_sent          = false;

    /*-----------------------------------------------------*\
    | Register callback for receiving broadcasts            |
    \*-----------------------------------------------------*/
    RegisterReceiveBroadcastCallback(this);

    /*-----------------------------------------------------*\
    | Request device information                            |
    \*-----------------------------------------------------*/
    SendScan();

    /*-----------------------------------------------------*\
    | Wait up to 5s for device information to be received   |
    \*-----------------------------------------------------*/
    for(unsigned int wait_count = 0; wait_count < 500; wait_count++)
    {
        if(broadcast_received)
        {
            break;
        }

        std::this_thread::sleep_for(10ms);
    }

    /*-----------------------------------------------------*\
    | Open a UDP client sending to the Govee device IP,     |
    | port 4003                                             |
    \*-----------------------------------------------------*/
    port.sock = INVALID_SOCKET;
    port.udp_client(ip_address.c_str(), "4003");
}

GoveeController::~GoveeController()
{
    UnregisterReceiveBroadcastCallback(this);
    if(port.sock != INVALID_SOCKET)
    {
        port.tcp_close();
    }
}

std::string GoveeController::GetLocation()
{
    return("IP: " + ip_address);
}

std::string GoveeController::GetSku()
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);
    return(sku);
}

std::string GoveeController::GetVersion()
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);
    return("BLE Hardware Version: "  + bleVersionHard  + "\r\n" +
           "BLE Software Version: "  + bleVersionSoft  + "\r\n" +
           "WiFi Hardware Version: " + wifiVersionHard + "\r\n" +
           "WiFI Software Version: " + wifiVersionSoft + "\r\n");
}

void GoveeController::ReceiveBroadcast(char* recv_buf, int size)
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);

    /*-----------------------------------------------------*\
    | UDP responses are not null-terminated.                |
    \*-----------------------------------------------------*/
    json response;
    if(size <= 0 || !JsonUtils::JsonParse(std::string(recv_buf, size), response)
       || !response.is_object() || !response.contains("msg") || !response["msg"].is_object())
    {
        return;
    }

    if(response["msg"].value("cmd", json()) == "status")
    {
        json data = response["msg"].value("data", json());
        if(data.is_object() && data.contains("onOff") && data["onOff"].is_number_integer()
           && (data["onOff"] == 0 || data["onOff"] == 1))
        {
            bool razer_state_known = false;
            bool razer_enabled     = false;

            if(data.contains("pt") && data["pt"].is_string())
            {
                try
                {
                    std::vector<base64::byte> pt = base64::decode(data["pt"]);
                    if(pt.size() == 6 && pt[0] == 0xBB && pt[1] == 0x00 && pt[2] == 0x01
                       && pt[3] == 0xB2 && (pt[4] == 0 || pt[4] == 1) && CalculateXorChecksum(pt) == 0)
                    {
                        razer_state_known = true;
                        razer_enabled     = pt[4] == 1;
                    }
                }
                catch(const std::runtime_error&)
                {
                    /*-----------------------------------------*\
                    | Ignore invalid PT data; onOff is usable.   |
                    \*-----------------------------------------*/
                }
            }
            bool power = data["onOff"] == 1;
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            bool status_timeout = status_known && now - last_status_time >= std::chrono::seconds(GOVEE_STATUS_TIMEOUT);

            /*-------------------------------------------------*\
            | Recover after a power cycle, a missing status     |
            | interval, or a reported loss of Razer mode.        |
            \*-------------------------------------------------*/
            if(status_timeout || (status_known && !powered && power)
               || (power && razer_state_known && !razer_enabled))
            {
                recovery_pending = true;
            }

            status_known     = true;
            powered          = power;
            last_status_time = now;
        }
        return;
    }

    if(broadcast_received)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Check if the response contains the method name        |
    \*-----------------------------------------------------*/
    if(response.contains("msg"))
    {
        /*-------------------------------------------------*\
        | Handle responses for scan command                 |
        | This command's response should contain a msg      |
        | object containing a data member with ip, device,  |
        | sku, among others.                                |
        \*-------------------------------------------------*/
        if(response["msg"].contains("cmd"))
        {
            if(response["msg"]["cmd"] == "scan")
            {
                if(response["msg"].contains("data"))
                {
                    if(response["msg"]["data"].contains("ip"))
                    {
                        if(response["msg"]["data"]["ip"] == ip_address)
                        {
                            if(response["msg"]["data"].contains("sku") && response["msg"]["data"]["sku"].is_string())
                            {
                                sku = response["msg"]["data"]["sku"];
                            }

                            if(response["msg"]["data"].contains("bleVersionHard") && response["msg"]["data"]["bleVersionHard"].is_string())
                            {
                                bleVersionHard = response["msg"]["data"]["bleVersionHard"];
                            }

                            if(response["msg"]["data"].contains("bleVersionSoft") && response["msg"]["data"]["bleVersionSoft"].is_string())
                            {
                                bleVersionSoft = response["msg"]["data"]["bleVersionSoft"];
                            }

                            if(response["msg"]["data"].contains("wifiVersionHard") && response["msg"]["data"]["wifiVersionHard"].is_string())
                            {
                                wifiVersionHard = response["msg"]["data"]["wifiVersionHard"];
                            }

                            if(response["msg"]["data"].contains("wifiVersionSoft") && response["msg"]["data"]["wifiVersionSoft"].is_string())
                            {
                                wifiVersionSoft = response["msg"]["data"]["wifiVersionSoft"];
                            }

                            broadcast_received = true;
                        }
                    }
                }
            }
        }
    }
}

void GoveeController::SetColor(unsigned char red, unsigned char green, unsigned char blue)
{
    json command;

    command["msg"]["cmd"]                       = "colorwc";
    command["msg"]["data"]["color"]["r"]        = red;
    command["msg"]["data"]["color"]["g"]        = green;
    command["msg"]["data"]["color"]["b"]        = blue;
    command["msg"]["data"]["colorTemInKelvin"]  = "0";

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1);
}

bool GoveeController::SendRazerData(RGBColor* colors, unsigned int size)
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);
    /*-----------------------------------------------------*\
    | Do not send an empty frame (this was producing        |
    | length=2, count=0)                                    |
    \*-----------------------------------------------------*/
    if(size == 0 || (status_known && !powered))
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | PT payload: BB [len_hi] [len_lo] B0 [gradient_off=1]  |
    | [led_count] (RGB * N) [xor]                           |
    | length = 2 + 3*N  (bytes after 0xB0: gradient_off +   |
    | led_count + RGB*count)                                |
    \*-----------------------------------------------------*/
    const unsigned int count = std::min(size, 255u);
    const unsigned int payload_len = 2 + (3 * count);

    /*-----------------------------------------------------*\
    | Create buffer with fixed size and fill sequentially   |
    \*-----------------------------------------------------*/

    std::vector<base64::byte> pkt;
    pkt.reserve(7 + (3 * count));

    pkt.push_back(0xBB);
    pkt.push_back(static_cast<base64::byte>((payload_len >> 8) & 0xFF)); /* len_hi */
    pkt.push_back(static_cast<base64::byte>(payload_len & 0xFF));        /* len_lo */
    pkt.push_back(0xB0);                                                 /* subcommand */
    pkt.push_back(0x01);                                                 /* gradient_off = 1 */
    pkt.push_back(static_cast<base64::byte>(count));                     /* led_count */

    for(std::size_t led_idx = 0; led_idx < count; led_idx++)
    {
        pkt.push_back(RGBGetRValue(colors[led_idx]));
        pkt.push_back(RGBGetGValue(colors[led_idx]));
        pkt.push_back(RGBGetBValue(colors[led_idx]));
    }

    pkt.push_back(CalculateXorChecksum(pkt));

    json command;
    command["msg"]["cmd"]                       = "razer";
    command["msg"]["data"]["pt"]                = base64::encode(pkt);

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    if(NeedsRazerRecovery() && !SendRazerEnable())
    {
        return(false);
    }
    if(port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1) > 0)
    {
        frame_sent      = true;
        last_frame_time = std::chrono::steady_clock::now();
        return(true);
    }
    return(false);
}

void GoveeController::SendRazerDisable()
{
    const std::vector<base64::byte> pkt = { 0xBB, 0x00, 0x01, 0xB1, 0x00, 0x0B };
    json command;

    command["msg"]["cmd"]                       = "razer";
    command["msg"]["data"]["pt"]                = base64::encode(pkt);

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1);
}

bool GoveeController::SendRazerEnable(bool new_session)
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);
    const std::vector<base64::byte> pkt = { 0xBB, 0x00, 0x01, 0xB1, 0x01, 0x0A };
    json command;

    command["msg"]["cmd"]                       = "razer";
    command["msg"]["data"]["pt"]                = base64::encode(pkt);

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    if(port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1) > 0)
    {
        recovery_pending = false;
        frame_sent       = true;
        last_frame_time  = std::chrono::steady_clock::now();

        /*-------------------------------------------------*\
        | Mode changes start a new status timeout. Recovery |
        | keeps the last reply time to detect an outage.     |
        \*-------------------------------------------------*/
        if(new_session)
        {
            powered          = true;
            last_status_time = last_frame_time;
        }
        return(true);
    }

    recovery_pending = true;
    return(false);
}

void GoveeController::RequestStatus()
{
    char command[] = "{\"msg\":{\"cmd\":\"status\",\"data\":{}}}";
    port.udp_write(command, sizeof(command));
}

bool GoveeController::NeedsRazerRecovery()
{
    std::lock_guard<std::recursive_mutex> lock(session_mutex);

    if(status_known && !powered)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Razer mode expires after about a minute without data. |
    \*-----------------------------------------------------*/
    bool frame_timeout = frame_sent && std::chrono::steady_clock::now() - last_frame_time >= std::chrono::seconds(GOVEE_FRAME_TIMEOUT);
    return(recovery_pending || frame_timeout);
}

void GoveeController::SetBrightness(unsigned int brightness)
{
    json command;

    command["msg"]["cmd"]                       = "brightness";
    command["msg"]["data"]["value"]             = std::min(brightness, 100u);

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1);
}

void GoveeController::SetPower(bool enabled)
{
    json command;

    command["msg"]["cmd"]                       = "turn";
    command["msg"]["data"]["value"]             = enabled ? 1 : 0;

    std::string command_str                     = command.dump();

    port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1);
}

void GoveeController::SendScan()
{
    if(!ReceiveThreadRun)
    {
        return;
    }

    json command;

    command["msg"]["cmd"]                       = "scan";
    /*-----------------------------------------------------*\
    | Matches what Govee devices commonly accept for LAN    |
    | scan                                                  |
    \*-----------------------------------------------------*/
    command["msg"]["data"]["account_topic"]     = "reserve";

    /*-----------------------------------------------------*\
    | Convert the JSON object to a string and write it      |
    \*-----------------------------------------------------*/
    std::string command_str                     = command.dump();

    broadcast_port.udp_write((char *)command_str.c_str(), (int)command_str.length() + 1);
}

/*---------------------------------------------------------*\
| Static class members for shared broadcast receiver        |
\*---------------------------------------------------------*/
net_port                        GoveeController::broadcast_port;
std::vector<GoveeController*>   GoveeController::callbacks;
std::thread*                    GoveeController::ReceiveThread;
std::atomic<bool>               GoveeController::ReceiveThreadRun;
std::mutex                      GoveeController::callbacks_mutex;
std::mutex                      GoveeController::receiver_lifecycle_mutex;

void GoveeController::ReceiveBroadcastThreadFunction()
{
    char recv_buf[1024];

    broadcast_port.set_receive_timeout(1, 0);

    while(ReceiveThreadRun.load())
    {
        /*-------------------------------------------------*\
        | Receive up to 1024 bytes from the device with a   |
        | 1s timeout                                        |
        \*-------------------------------------------------*/
        sockaddr_in sender = {};
#ifdef WIN32
        int sender_size = sizeof(sender);
#else
        socklen_t sender_size = sizeof(sender);
#endif
        int size = recvfrom(broadcast_port.sock, recv_buf, sizeof(recv_buf), 0,
                            (sockaddr*)&sender, &sender_size);

        /*-------------------------------------------------*\
        | Status replies have no device ID, so match the    |
        | source IP to the registered controller.            |
        \*-------------------------------------------------*/
        if(size > 0)
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex);
            for(std::size_t callback_idx = 0; callback_idx < callbacks.size(); callback_idx++)
            {
                GoveeController* controller = callbacks[callback_idx];

                if(sender.sin_addr.s_addr == inet_addr(controller->ip_address.c_str()))
                {
                    controller->ReceiveBroadcast(recv_buf, size);
                }
            }
        }
    }
}

void GoveeController::RegisterReceiveBroadcastCallback(GoveeController* controller_ptr)
{
    std::lock_guard<std::mutex> lifecycle_lock(receiver_lifecycle_mutex);
    std::lock_guard<std::mutex> lock(callbacks_mutex);
    if(callbacks.empty())
    {
        broadcast_port.sock = INVALID_SOCKET;
        if(broadcast_port.udp_client("239.255.255.250", "4001", "4002"))
        {
            broadcast_port.udp_join_multicast_group("239.255.255.250");
            ReceiveThreadRun = true;
            ReceiveThread = new std::thread(&GoveeController::ReceiveBroadcastThreadFunction);
        }
        else if(broadcast_port.sock != INVALID_SOCKET)
        {
            broadcast_port.tcp_close();
        }
    }
    callbacks.push_back(controller_ptr);
}

void GoveeController::UnregisterReceiveBroadcastCallback(GoveeController* controller_ptr)
{
    std::lock_guard<std::mutex> lifecycle_lock(receiver_lifecycle_mutex);
    std::unique_lock<std::mutex> lock(callbacks_mutex);
    for(std::size_t callback_idx = 0; callback_idx < callbacks.size(); callback_idx++)
    {
        if(callbacks[callback_idx] == controller_ptr)
        {
            callbacks.erase(callbacks.begin() + callback_idx);
            break;
        }
    }
    if(callbacks.empty() && ReceiveThreadRun)
    {
        ReceiveThreadRun = false;
        lock.unlock();
        ReceiveThread->join();
        delete ReceiveThread;
        ReceiveThread = nullptr;
        broadcast_port.tcp_close();
    }
}

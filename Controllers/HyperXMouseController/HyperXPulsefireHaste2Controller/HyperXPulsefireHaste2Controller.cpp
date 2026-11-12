/*---------------------------------------------------------*\
| HyperXPulsefireHaste2Controller.cpp                       |
|                                                           |
|   Driver for HyperX Pulsefire Haste 2 Wireless            |
|                                                           |
|   Plamen Marinov (PMarinov1994)               14 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdio>
#include <cstring>
#include "HyperXPulsefireHaste2Controller.h"
#include "LogManager.h"

HyperXPulsefireHaste2Controller::HyperXPulsefireHaste2Controller(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev                 = dev_handle;
    location            = path;
    name                = dev_name;

    mode_applied        = false;
    link_up             = true;
    reapply_pending     = false;
    active_mode         = HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT;
    active_speed        = HYPERX_PULSEFIRE_HASTE_2_SPEED_DEFAULT;
    active_brightness   = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    active_color        = 0;

    last_packet_time    = std::chrono::steady_clock::now();
    last_direct_time    = last_packet_time;
    last_link_poll_time = last_packet_time;

    {
        std::lock_guard<std::mutex> lock(dev_mutex);

        DrainInputReports();
        ReadDeviceInformation();
        ReadConnectionStatus();
    }

    keepalive_running   = true;
    keepalive_thread    = std::thread(&HyperXPulsefireHaste2Controller::KeepaliveThreadFunction, this);
}

HyperXPulsefireHaste2Controller::~HyperXPulsefireHaste2Controller()
{
    /*-----------------------------------------------------*\
    | Stop and join the keepalive thread before taking      |
    | dev_mutex, which the thread also locks                |
    \*-----------------------------------------------------*/
    keepalive_running = false;

    if(keepalive_thread.joinable())
    {
        keepalive_thread.join();
    }

    std::lock_guard<std::mutex> lock(dev_mutex);

    hid_device* dev_to_close = dev;
    dev = nullptr;

    if(dev_to_close != nullptr)
    {
        hid_close(dev_to_close);
    }
}

std::string HyperXPulsefireHaste2Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string HyperXPulsefireHaste2Controller::GetNameString()
{
    return(name);
}

std::string HyperXPulsefireHaste2Controller::GetSerialString()
{
    return(serial);
}

void HyperXPulsefireHaste2Controller::SetMode(unsigned char mode, unsigned char speed, RGBColor color, unsigned char brightness)
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    active_mode         = mode;
    active_speed        = speed;
    active_color        = color;
    active_brightness   = brightness;
    mode_applied        = true;

    ApplyMode();
}

void HyperXPulsefireHaste2Controller::SetDirectColor(RGBColor color)
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    active_color = color;

    if(mode_applied && (active_mode == HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT))
    {
        SendDirectFrame(color);
    }
}

void HyperXPulsefireHaste2Controller::SaveMode(unsigned char mode, unsigned char speed, RGBColor color, unsigned char brightness)
{
    std::lock_guard<std::mutex> lock(dev_mutex);

    /*-----------------------------------------------------*\
    | Write the onboard profile, which the mouse loads into |
    | the live profile when it is switched on.  Direct mode |
    | is stored as a one frame FrameLight effect.  Off has  |
    | no brightness, so the stored brightness is kept       |
    \*-----------------------------------------------------*/
    if(mode != HYPERX_PULSEFIRE_HASTE_2_MODE_OFF)
    {
        SendBrightness(HYPERX_PULSEFIRE_HASTE_2_PROFILE_ONBOARD, brightness);
    }

    if(mode == HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT)
    {
        SendFrameLight(HYPERX_PULSEFIRE_HASTE_2_PROFILE_ONBOARD, color);
    }
    else
    {
        SendPreset(HYPERX_PULSEFIRE_HASTE_2_PROFILE_ONBOARD, mode, speed, color);
    }
}

/*-------------------------------------------------------------------------------------------------*\
| Private functions.  All of them expect the caller to hold dev_mutex.                              |
\*-------------------------------------------------------------------------------------------------*/

void HyperXPulsefireHaste2Controller::KeepaliveThreadFunction()
{
    while(keepalive_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(HYPERX_PULSEFIRE_HASTE_2_KEEPALIVE_POLL_MS));

        std::lock_guard<std::mutex> lock(dev_mutex);

        /*-------------------------------------------------*\
        | Read pending reports to track the wireless link   |
        \*-------------------------------------------------*/
        DrainInputReports();

        if(!mode_applied)
        {
            continue;
        }

        /*-------------------------------------------------*\
        | While asleep, poll the connection status so that  |
        | a missed wake up event does not leave Direct mode |
        | without keepalive frames                          |
        \*-------------------------------------------------*/
        if(!link_up)
        {
            if((std::chrono::steady_clock::now() - last_link_poll_time) >= std::chrono::milliseconds(HYPERX_PULSEFIRE_HASTE_2_LINK_POLL_INTERVAL_MS))
            {
                last_link_poll_time = std::chrono::steady_clock::now();

                ReadConnectionStatus();
            }

            continue;
        }

        /*-------------------------------------------------*\
        | The mouse went to sleep and woke up again, so     |
        | apply the current mode again                      |
        \*-------------------------------------------------*/
        if(reapply_pending)
        {
            reapply_pending = false;

            ApplyMode();
        }
        else if((active_mode == HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT)
             && ((std::chrono::steady_clock::now() - last_direct_time) >= std::chrono::milliseconds(HYPERX_PULSEFIRE_HASTE_2_KEEPALIVE_INTERVAL_MS)))
        {
            SendDirectFrame(active_color);
        }
    }
}

void HyperXPulsefireHaste2Controller::ApplyMode()
{
    if(active_mode == HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT)
    {
        SendBrightness(HYPERX_PULSEFIRE_HASTE_2_PROFILE_LIVE, active_brightness);
        SendDirectFrame(active_color);
    }
    else if(active_mode == HYPERX_PULSEFIRE_HASTE_2_MODE_OFF)
    {
        SendPreset(HYPERX_PULSEFIRE_HASTE_2_PROFILE_LIVE, active_mode, active_speed, active_color);
    }
    else
    {
        SendBrightness(HYPERX_PULSEFIRE_HASTE_2_PROFILE_LIVE, active_brightness);
        SendPreset(HYPERX_PULSEFIRE_HASTE_2_PROFILE_LIVE, active_mode, active_speed, active_color);
    }
}

void HyperXPulsefireHaste2Controller::ReadDeviceInformation()
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];
    unsigned char reply[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_DEVICE_INFO;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_DEVICE_INFO;

    if(!SendPacket(buf, reply))
    {
        LOG_WARNING("[%s] No reply to the device information request", name.c_str());
        return;
    }

    /*-----------------------------------------------------*\
    | The dongle has no USB serial number string, so use    |
    | the device ID from the device information reply       |
    \*-----------------------------------------------------*/
    char id_string[(HYPERX_PULSEFIRE_HASTE_2_DEVICE_ID_LENGTH * 2) + 1];

    for(unsigned int byte_idx = 0; byte_idx < HYPERX_PULSEFIRE_HASTE_2_DEVICE_ID_LENGTH; byte_idx++)
    {
        snprintf(&id_string[byte_idx * 2], 3, "%02X", reply[HYPERX_PULSEFIRE_HASTE_2_DEVICE_ID_OFFSET + byte_idx]);
    }

    serial = id_string;
}

void HyperXPulsefireHaste2Controller::ReadConnectionStatus()
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];
    unsigned char reply[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_CONNECTION;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_CONNECTION;

    /*-----------------------------------------------------*\
    | Reply is 53 01 <connected>.  Without a reply keep the |
    | current state.  When the mouse comes back, apply the  |
    | current mode again                                    |
    \*-----------------------------------------------------*/
    if(!SendPacket(buf, reply))
    {
        return;
    }

    bool linked = (reply[0x02] != 0x00);

    if(linked != link_up)
    {
        link_up = linked;

        if(link_up)
        {
            reapply_pending = true;
        }

        LOG_DEBUG("[%s] Mouse %s", name.c_str(), link_up ? "linked" : "asleep");
    }
}

void HyperXPulsefireHaste2Controller::SendBrightness(unsigned char profile, unsigned char brightness)
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    memset(buf, 0x00, sizeof(buf));

    /*-----------------------------------------------------*\
    | 40 01 <profile u16> <brightness>                      |
    \*-----------------------------------------------------*/
    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_BRIGHTNESS;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_BRIGHTNESS_SET;
    buf[0x02]   = profile;
    buf[0x04]   = brightness;

    SendPacket(buf, NULL);
}

void HyperXPulsefireHaste2Controller::SendPreset(unsigned char profile, unsigned char mode, unsigned char speed, RGBColor color)
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];
    unsigned char reply[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    memset(buf, 0x00, sizeof(buf));

    /*-----------------------------------------------------*\
    | 42 02 <profile u16> <mode> <duration u16> <direction> |
    |       <color type> <R G B>                            |
    | Only the low byte of the duration is used             |
    \*-----------------------------------------------------*/
    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_PRESET;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_PRESET_SET;
    buf[0x02]   = profile;
    buf[0x04]   = mode;

    switch(mode)
    {
        case HYPERX_PULSEFIRE_HASTE_2_MODE_STATIC:
            buf[0x08]   = HYPERX_PULSEFIRE_HASTE_2_COLOR_TYPE_SINGLE;
            buf[0x09]   = RGBGetRValue(color);
            buf[0x0A]   = RGBGetGValue(color);
            buf[0x0B]   = RGBGetBValue(color);
            break;

        case HYPERX_PULSEFIRE_HASTE_2_MODE_BREATHING:
            buf[0x05]   = speed;
            buf[0x08]   = HYPERX_PULSEFIRE_HASTE_2_COLOR_TYPE_SINGLE;
            buf[0x09]   = RGBGetRValue(color);
            buf[0x0A]   = RGBGetGValue(color);
            buf[0x0B]   = RGBGetBValue(color);
            break;

        case HYPERX_PULSEFIRE_HASTE_2_MODE_SPECTRUM_CYCLE:
            buf[0x05]   = speed;
            buf[0x08]   = HYPERX_PULSEFIRE_HASTE_2_COLOR_TYPE_SPECTRUM;
            break;

        default:
            break;
    }

    SendPacket(buf, reply);
}

void HyperXPulsefireHaste2Controller::SendDirectFrame(RGBColor color)
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    memset(buf, 0x00, sizeof(buf));

    /*-----------------------------------------------------*\
    | 44 02 <page u16> <R G B>.  The mouse shows the color  |
    | only while frames keep arriving.  The 45 02 reply is  |
    | not waited for so that effects can stream             |
    \*-----------------------------------------------------*/
    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_DIRECT;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_DIRECT_COLOR;
    buf[0x04]   = RGBGetRValue(color);
    buf[0x05]   = RGBGetGValue(color);
    buf[0x06]   = RGBGetBValue(color);

    SendPacket(buf, NULL);

    last_direct_time = std::chrono::steady_clock::now();
}

void HyperXPulsefireHaste2Controller::SendFrameLight(unsigned char profile, RGBColor color)
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];
    unsigned char reply[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    /*-----------------------------------------------------*\
    | 44 03 <frames> <packets u16> <profile u16>            |
    | One frame in one packet holds a static color          |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_DIRECT;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_FRAME_HEADER;
    buf[0x02]   = 0x01;
    buf[0x03]   = 0x01;
    buf[0x05]   = profile;

    SendPacket(buf, NULL);

    /*-----------------------------------------------------*\
    | 44 04 <packet index u16> <R G B>, the mouse replies   |
    | with 45 04 once it has the frame                      |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_DIRECT;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_FRAME_DATA;
    buf[0x04]   = RGBGetRValue(color);
    buf[0x05]   = RGBGetGValue(color);
    buf[0x06]   = RGBGetBValue(color);

    SendPacket(buf, reply);

    /*-----------------------------------------------------*\
    | Switch the profile to play the uploaded frames        |
    \*-----------------------------------------------------*/
    memset(buf, 0x00, sizeof(buf));

    buf[0x00]   = HYPERX_PULSEFIRE_HASTE_2_CMD_PRESET;
    buf[0x01]   = HYPERX_PULSEFIRE_HASTE_2_SUB_PRESET_SET;
    buf[0x02]   = profile;
    buf[0x04]   = HYPERX_PULSEFIRE_HASTE_2_MODE_FRAMELIGHT;
    buf[0x05]   = HYPERX_PULSEFIRE_HASTE_2_SPEED_DEFAULT;

    SendPacket(buf, reply);
}

bool HyperXPulsefireHaste2Controller::SendPacket(unsigned char* buf, unsigned char* reply)
{
    if(dev == nullptr)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Keep packets at least 10ms apart                      |
    \*-----------------------------------------------------*/
    std::chrono::steady_clock::time_point next_packet_time = last_packet_time + std::chrono::milliseconds(HYPERX_PULSEFIRE_HASTE_2_PACKET_INTERVAL_MS);

    std::this_thread::sleep_until(next_packet_time);

    hid_write(dev, buf, HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE);

    last_packet_time = std::chrono::steady_clock::now();

    if(reply == NULL)
    {
        return(true);
    }

    /*-----------------------------------------------------*\
    | Wait for the reply (command + 1, same sub command).   |
    | Sending the next packet before it arrives makes the   |
    | mouse drop packets over the wireless link             |
    \*-----------------------------------------------------*/
    std::chrono::steady_clock::time_point deadline = last_packet_time + std::chrono::milliseconds(HYPERX_PULSEFIRE_HASTE_2_REPLY_TIMEOUT_MS);

    while(std::chrono::steady_clock::now() < deadline)
    {
        int remaining_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now()).count();
        int bytes_read   = hid_read_timeout(dev, reply, HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE, remaining_ms);

        if(bytes_read <= 0)
        {
            break;
        }

        HandleInputReport(reply);

        if((reply[0x00] == (buf[0x00] + 1)) && (reply[0x01] == buf[0x01]))
        {
            return(true);
        }
    }

    LOG_DEBUG("[%s] No reply to %02X %02X", name.c_str(), buf[0x00], buf[0x01]);

    return(false);
}

void HyperXPulsefireHaste2Controller::DrainInputReports()
{
    unsigned char buf[HYPERX_PULSEFIRE_HASTE_2_REPORT_SIZE];

    if(dev == nullptr)
    {
        return;
    }

    while(hid_read_timeout(dev, buf, sizeof(buf), 0) > 0)
    {
        HandleInputReport(buf);
    }
}

void HyperXPulsefireHaste2Controller::HandleInputReport(unsigned char* buf)
{
    /*-----------------------------------------------------*\
    | FB 0A <state>: 01 = mouse linked, 00 = link lost      |
    \*-----------------------------------------------------*/
    if((buf[0x00] == HYPERX_PULSEFIRE_HASTE_2_REPORT_EVENT) && (buf[0x01] == HYPERX_PULSEFIRE_HASTE_2_EVENT_LINK))
    {
        link_up = (buf[0x02] != 0x00);

        if(link_up)
        {
            reapply_pending = true;
        }

        LOG_DEBUG("[%s] Mouse %s", name.c_str(), link_up ? "linked" : "asleep");
    }
}

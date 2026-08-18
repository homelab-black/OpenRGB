/*---------------------------------------------------------*\
| AulaHero68Controller.h                                    |
|                                                           |
|   Driver for AULA Hero 68 keyboard                        |
|                                                           |
|   Reverse engineered with user-provided USB captures      |
|   Original author: dunn1o, 2026-08-09                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <hidapi.h>

#define AULA_HERO_68_PACKET_SIZE 64

struct AulaHero68KeyUpdate
{
    uint16_t key_id;
    uint8_t  red;
    uint8_t  green;
    uint8_t  blue;
};

class AulaHero68Controller
{
public:
    AulaHero68Controller(hid_device* dev_handle, const char* path);
    ~AulaHero68Controller();

    std::string GetDeviceLocation();
    std::string GetDeviceName();
    std::string GetSerialString();

    void SetModeOff();
    void SetModeStatic(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
    void SetModeReactive(uint8_t red, uint8_t green, uint8_t blue, uint8_t speed);
    void EnterCustomMode(uint8_t red, uint8_t green, uint8_t blue);
    void SetSingleKey(uint16_t key_id, uint8_t red, uint8_t green, uint8_t blue, bool reliable = true);
    void SetKeysBatch(const std::vector<AulaHero68KeyUpdate>& updates);

private:
    hid_device*                     dev;
    std::string                     location;
    std::string                     name;
    bool                            custom_mode_initialized;

    void BuildStaticPacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
    void BuildReactivePacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed);
    void BuildOffPacket(uint8_t* payload);
    void BuildCustomIntroPacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue);
    void BuildCustomAckPacket(uint8_t* payload);
    void BuildCustomKeyTablePacket(uint8_t* payload, const std::vector<uint16_t>& key_ids);
    void BuildPerKeyPacket(uint8_t* payload, uint16_t key_id, uint8_t red, uint8_t green, uint8_t blue);
    void BuildBatchPacket(uint8_t* payload, uint8_t total_packets, uint8_t packet_index, const std::vector<AulaHero68KeyUpdate>& updates, size_t start_index, size_t count);

    void SendPayload(const uint8_t* payload, bool duplicate = false, int delay_ms = 15);
    void FinalizeChecksum(uint8_t* payload);
    void ResetCustomModeState();
};

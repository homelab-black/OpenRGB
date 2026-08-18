/*---------------------------------------------------------*\
| AulaHero68Controller.cpp                                  |
|                                                           |
|   Driver for AULA Hero 68 keyboard                        |
|                                                           |
|   Reverse engineered with user-provided USB captures      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AulaHero68Controller.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

#include "StringUtils.h"

AulaHero68Controller::AulaHero68Controller(hid_device* dev_handle, const char* path)
{
    dev                     = dev_handle;
    location                = path;
    custom_mode_initialized = false;

    wchar_t name_string[128];

    if(hid_get_manufacturer_string(dev, name_string, 128) == 0)
    {
        name = StringUtils::wstring_to_string(name_string);
    }

    if(hid_get_product_string(dev, name_string, 128) == 0)
    {
        if(!name.empty())
        {
            name.append(" ");
        }

        name.append(StringUtils::wstring_to_string(name_string));
    }

    if(name.empty())
    {
        name = "AULA Hero 68";
    }
}

AulaHero68Controller::~AulaHero68Controller()
{
    hid_close(dev);
}

std::string AulaHero68Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string AulaHero68Controller::GetDeviceName()
{
    return(name);
}

std::string AulaHero68Controller::GetSerialString()
{
    wchar_t serial_string[128];

    if(hid_get_serial_number_string(dev, serial_string, 128) != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void AulaHero68Controller::SetModeOff()
{
    uint8_t payload[AULA_HERO_68_PACKET_SIZE];

    ResetCustomModeState();
    BuildOffPacket(payload);
    SendPayload(payload);
}

void AulaHero68Controller::SetModeStatic(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
{
    uint8_t payload[AULA_HERO_68_PACKET_SIZE];

    ResetCustomModeState();
    BuildStaticPacket(payload, red, green, blue, brightness);
    SendPayload(payload);
}

void AulaHero68Controller::SetModeReactive(uint8_t red, uint8_t green, uint8_t blue, uint8_t speed)
{
    uint8_t payload[AULA_HERO_68_PACKET_SIZE];

    ResetCustomModeState();
    BuildReactivePacket(payload, red, green, blue, speed);
    SendPayload(payload);
}

void AulaHero68Controller::EnterCustomMode(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t payload[AULA_HERO_68_PACKET_SIZE];

    if(custom_mode_initialized)
    {
        return;
    }

    BuildCustomIntroPacket(payload, red, green, blue);
    SendPayload(payload);
    BuildCustomAckPacket(payload);
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x0001, 0x0062, 0x000F, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017 });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F, 0x0020, 0x0021, 0x0022 });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x0063, 0x002A, 0x002B, 0x002C });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0066 });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F, 0x0040, 0x0041 });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x0042, 0x004A, 0x0067, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x004C, 0x004B });
    SendPayload(payload);
    BuildCustomKeyTablePacket(payload, { 0x004D, 0x0049 });
    SendPayload(payload);

    custom_mode_initialized = true;
}

void AulaHero68Controller::SetSingleKey(uint16_t key_id, uint8_t red, uint8_t green, uint8_t blue, bool reliable)
{
    uint8_t payload[AULA_HERO_68_PACKET_SIZE];

    BuildPerKeyPacket(payload, key_id, red, green, blue);

    if(reliable)
    {
        SendPayload(payload, true, 15);
    }
    else
    {
        SendPayload(payload, false, 1);
    }
}

void AulaHero68Controller::SetKeysBatch(const std::vector<AulaHero68KeyUpdate>& updates)
{
    if(updates.empty())
    {
        return;
    }

    const size_t updates_per_packet = 11;
    const uint8_t total_packets = static_cast<uint8_t>((updates.size() + updates_per_packet - 1) / updates_per_packet);

    for(uint8_t packet_index = 0; packet_index < total_packets; packet_index++)
    {
        uint8_t payload[AULA_HERO_68_PACKET_SIZE];
        size_t start_index = packet_index * updates_per_packet;
        size_t count = std::min(updates_per_packet, updates.size() - start_index);

        BuildBatchPacket(payload, total_packets, packet_index, updates, start_index, count);
        SendPayload(payload, false, 1);
    }
}

void AulaHero68Controller::BuildStaticPacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x04;
    payload[0x02] = 0x01;
    payload[0x04] = 0x01;
    payload[0x06] = 0x07;
    payload[0x07] = 0x01;
    payload[0x08] = 0x00;
    payload[0x09] = red;
    payload[0x0A] = green;
    payload[0x0B] = blue;
    payload[0x0C] = brightness;
    payload[0x0D] = 0x02;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildReactivePacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x04;
    payload[0x02] = 0x01;
    payload[0x04] = 0x01;
    payload[0x06] = 0x07;
    payload[0x07] = 0x02;
    payload[0x08] = 0x07;
    payload[0x09] = red;
    payload[0x0A] = green;
    payload[0x0B] = blue;
    payload[0x0C] = 0x0A;
    payload[0x0D] = speed;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildOffPacket(uint8_t* payload)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x04;
    payload[0x02] = 0x01;
    payload[0x04] = 0x01;
    payload[0x06] = 0x07;
    payload[0x07] = 0x00;
    payload[0x08] = 0x07;
    payload[0x09] = 0xFF;
    payload[0x0A] = 0x0D;
    payload[0x0B] = 0x62;
    payload[0x0C] = 0x0A;
    payload[0x0D] = 0x02;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildCustomIntroPacket(uint8_t* payload, uint8_t red, uint8_t green, uint8_t blue)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x04;
    payload[0x02] = 0x01;
    payload[0x04] = 0x01;
    payload[0x06] = 0x07;
    payload[0x07] = 0x13;
    payload[0x08] = 0x07;
    payload[0x09] = red;
    payload[0x0A] = green;
    payload[0x0B] = blue;
    payload[0x0C] = 0x0A;
    payload[0x0D] = 0x02;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildCustomAckPacket(uint8_t* payload)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x84;
    payload[0x02] = 0x01;
    payload[0x04] = 0x01;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildCustomKeyTablePacket(uint8_t* payload, const std::vector<uint16_t>& key_ids)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x86;
    payload[0x04] = 0x01;
    payload[0x06] = static_cast<uint8_t>(key_ids.size() * 2);

    size_t offset = 0x08;

    for(size_t key_index = 0; key_index < key_ids.size(); key_index++)
    {
        uint16_t key_id = key_ids[key_index];
        payload[offset++] = static_cast<uint8_t>(key_id & 0xFF);
        payload[offset++] = static_cast<uint8_t>((key_id >> 8) & 0xFF);
    }

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildPerKeyPacket(uint8_t* payload, uint16_t key_id, uint8_t red, uint8_t green, uint8_t blue)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x06;
    payload[0x04] = 0x01;
    payload[0x06] = 0x05;
    payload[0x08] = static_cast<uint8_t>(key_id & 0xFF);
    payload[0x09] = red;
    payload[0x0A] = green;
    payload[0x0B] = blue;

    FinalizeChecksum(payload);

}

void AulaHero68Controller::BuildBatchPacket(uint8_t* payload, uint8_t total_packets, uint8_t packet_index, const std::vector<AulaHero68KeyUpdate>& updates, size_t start_index, size_t count)
{
    memset(payload, 0, AULA_HERO_68_PACKET_SIZE);

    payload[0x00] = 0x09;
    payload[0x01] = 0x06;
    payload[0x04] = total_packets;
    payload[0x05] = packet_index;
    payload[0x06] = static_cast<uint8_t>(count * 5);

    size_t offset = 0x08;

    for(size_t i = 0; i < count; i++)
    {
        const AulaHero68KeyUpdate& update = updates[start_index + i];

        payload[offset++] = static_cast<uint8_t>(update.key_id & 0xFF);
        payload[offset++] = update.red;
        payload[offset++] = update.green;
        payload[offset++] = update.blue;
        payload[offset++] = 0x00;
    }

    FinalizeChecksum(payload);

}

void AulaHero68Controller::SendPayload(const uint8_t* payload, bool duplicate, int delay_ms)
{
    hid_write(dev, payload, AULA_HERO_68_PACKET_SIZE);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));

    if(duplicate)
    {
        hid_write(dev, payload, AULA_HERO_68_PACKET_SIZE);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
}

void AulaHero68Controller::FinalizeChecksum(uint8_t* payload)
{
    uint16_t sum = 0;

    for(size_t i = 0; i < AULA_HERO_68_PACKET_SIZE - 1; i++)
    {
        sum += payload[i];
    }

    payload[63] = static_cast<uint8_t>(0xFF - (sum & 0xFF));
}

void AulaHero68Controller::ResetCustomModeState()
{
    custom_mode_initialized = false;
}

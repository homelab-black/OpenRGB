/*---------------------------------------------------------*\
| MSIKeyboard1603Controller.cpp                             |
|                                                           |
|   Driver for MSI Mystic Light MS-1603 keyboards           |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "MSIKeyboard1603Controller.h"

#include <algorithm>
#include <chrono>
#include <thread>
#include <utility>

#include "LogManager.h"
#include "StringUtils.h"

namespace
{
const std::array<MSIKeyboard1603Controller::ZoneMask, MSIKeyboard1603Controller::ZoneCount> zone_masks =
{{
    {{0x01, 0x00, 0x00}},
    {{0x02, 0x00, 0x00}},
    {{0x04, 0x00, 0x00}},
    {{0x08, 0x00, 0x00}},
    {{0x10, 0x00, 0x00}},
    {{0x20, 0x00, 0x00}},
    {{0x40, 0x00, 0x00}},
    {{0x80, 0x00, 0x00}},
    {{0x00, 0x01, 0x00}},
    {{0x00, 0x02, 0x00}},
    {{0x00, 0x04, 0x00}},
    {{0x00, 0x08, 0x00}},
    {{0x00, 0x10, 0x00}},
    {{0x00, 0x20, 0x00}},
    {{0x00, 0x40, 0x00}},
    {{0x00, 0x80, 0x00}},
    {{0x00, 0x00, 0x01}},
    {{0x00, 0x00, 0x02}},
    {{0x00, 0x00, 0x04}},
    {{0x00, 0x00, 0x08}},
    {{0x00, 0x00, 0x10}},
    {{0x00, 0x00, 0x20}},
    {{0x00, 0x00, 0x40}},
    {{0x00, 0x00, 0x80}},
}};

const std::array<std::array<unsigned char, 2>, 3> breathing_speeds =
{{
    {{0x20, 0x03}},
    {{0x58, 0x02}},
    {{0x90, 0x01}},
}};

const std::array<std::array<unsigned char, 2>, 3> dance_speeds =
{{
    {{0x2C, 0x01}},
    {{0xC8, 0x00}},
    {{0x64, 0x00}},
}};

const std::array<std::array<unsigned char, 2>, 3> wheel_speeds =
{{
    {{0xF4, 0x01}},
    {{0x5E, 0x01}},
    {{0xC8, 0x00}},
}};

const std::array<std::array<unsigned char, 2>, 3> wave_speeds =
{{
    {{0xF4, 0x01}},
    {{0x5E, 0x01}},
    {{0xC8, 0x00}},
}};

const std::array<MSI1603Color, 7> dance_colors =
{{
    {0xFF, 0x00, 0x00},
    {0xFF, 0xFF, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0xFF, 0xFF},
    {0x00, 0x00, 0xFF},
    {0xFF, 0x00, 0xFF},
    {0xFF, 0x00, 0x00},
}};

const std::array<unsigned char, 6> dance_markers = {{0x11, 0x21, 0x32, 0x43, 0x53, 0x64}};

const std::array<unsigned char, 4> wheel_markers = {{0x0F, 0x32, 0x55, 0x64}};

const std::array<unsigned char, 6> rainbow_wheel_markers = {{0x11, 0x21, 0x32, 0x43, 0x53, 0x64}};

constexpr unsigned int wheel_direction_right = 1;
constexpr unsigned int wave_direction_right  = 1;

const std::array<unsigned char, 5> wave_markers = {{0x14, 0x28, 0x3C, 0x50, 0x64}};
const std::array<unsigned char, 3> color_cycle_markers = {{0x21, 0x42, 0x64}};

const std::array<MSI1603Color, 7> rainbow_wheel_colors =
{{
    {0xFF, 0x00, 0x00},
    {0xFF, 0xFF, 0x00},
    {0x00, 0xFF, 0x00},
    {0x00, 0xFF, 0xFF},
    {0x00, 0x00, 0xFF},
    {0xFF, 0x00, 0xFF},
    {0xFF, 0x00, 0x00},
}};

const std::array<std::pair<MSIKeyboard1603Controller::ZoneMask, MSI1603Color>, MSIKeyboard1603Controller::ZoneCount> rainbow_breathing_sequence =
{{
    {{{0x01, 0x00, 0x00}}, {0xFF, 0x00, 0x00}},
    {{{0x00, 0x08, 0x00}}, {0xFF, 0x00, 0x00}},
    {{{0x00, 0x10, 0x00}}, {0xFF, 0x00, 0x00}},
    {{{0x02, 0x00, 0x00}}, {0xFF, 0x80, 0x00}},
    {{{0x00, 0x20, 0x00}}, {0xFF, 0x80, 0x00}},
    {{{0x00, 0x40, 0x00}}, {0xFF, 0x80, 0x00}},
    {{{0x04, 0x00, 0x00}}, {0x80, 0xFF, 0x00}},
    {{{0x00, 0x80, 0x00}}, {0x80, 0xFF, 0x00}},
    {{{0x08, 0x00, 0x00}}, {0x00, 0xFF, 0x00}},
    {{{0x00, 0x00, 0x01}}, {0x00, 0xFF, 0x00}},
    {{{0x10, 0x00, 0x00}}, {0x00, 0xFF, 0x80}},
    {{{0x00, 0x00, 0x02}}, {0x00, 0xFF, 0x00}},
    {{{0x20, 0x00, 0x00}}, {0x00, 0x80, 0xFF}},
    {{{0x00, 0x00, 0x04}}, {0x00, 0x80, 0xFF}},
    {{{0x40, 0x00, 0x00}}, {0x00, 0x00, 0xFF}},
    {{{0x00, 0x00, 0x08}}, {0x00, 0x80, 0xFF}},
    {{{0x80, 0x00, 0x00}}, {0x80, 0x00, 0xFF}},
    {{{0x00, 0x01, 0x00}}, {0x80, 0x00, 0xFF}},
    {{{0x00, 0x00, 0x10}}, {0x80, 0x00, 0xFF}},
    {{{0x00, 0x00, 0x20}}, {0x80, 0x00, 0xFF}},
    {{{0x00, 0x02, 0x00}}, {0xFF, 0x00, 0x80}},
    {{{0x00, 0x04, 0x00}}, {0xFF, 0x00, 0x80}},
    {{{0x00, 0x00, 0x40}}, {0xFF, 0x00, 0x80}},
    {{{0x00, 0x00, 0x80}}, {0xFF, 0x00, 0x80}},
}};
}

MSIKeyboard1603Controller::MSIKeyboard1603Controller
(
    hid_device* handle,
    const char* path
)
{
    dev = handle;

    if(dev && path)
    {
        location = path;
    }
}

MSIKeyboard1603Controller::~MSIKeyboard1603Controller()
{
    if(dev)
    {
        hid_close(dev);
    }
}

void MSIKeyboard1603Controller::SetStatic(MSI1603Color color, std::size_t zone)
{
    ApplyStaticColor(zone, color);
}

void MSIKeyboard1603Controller::SetStatic(const std::vector<MSI1603Color>& colors)
{
    const std::size_t count = std::min(colors.size(), ZoneCount);

    for(std::size_t zone = 0; zone < count; zone++)
    {
        ApplyStaticColor(zone, colors[zone]);
    }
}

void MSIKeyboard1603Controller::SetBreathing(MSI1603Color color, unsigned int speed)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildBreathingPacket(color, speed));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetBreathing(MSI1603Color primary, MSI1603Color secondary, unsigned int speed)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildBreathingPacket(primary, secondary, speed));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetRainbowBreathing(unsigned int speed)
{
    for(const auto& step : rainbow_breathing_sequence)
    {
        Packet selector = {};
        selector[0] = 0x02;
        selector[1] = 0x01;
        selector[2] = step.first[0];
        selector[3] = step.first[1];
        selector[4] = step.first[2];

        SendPacket(selector);
        std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
        SendPacket(BuildBreathingPacket(step.second, speed));
        std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    }
}

void MSIKeyboard1603Controller::SetBouncyWall(MSI1603Color color)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildBouncyWallPacket(color));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetRainbowBouncyWall()
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildRainbowBouncyWallPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetColorCycle(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildColorCyclePacket(primary, secondary, tertiary, speed));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetRainbowColorCycle(unsigned int speed)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildRainbowColorCyclePacket(speed));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetWave(const std::array<MSI1603Color, 5>& colors, unsigned int speed, unsigned int direction)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildWavePacket(colors, speed, direction));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetRainbowWave(unsigned int speed, unsigned int direction)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildRainbowWavePacket(speed, direction));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetDance(unsigned int speed)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildOffPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildDancePacket(speed));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetWheel(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed, unsigned int direction)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildWheelPacket(primary, secondary, tertiary, speed, direction));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetRainbowWheel(unsigned int speed, unsigned int direction)
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildRainbowWheelPacket(speed, direction));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::SetOff()
{
    SendPacket(BuildAllSelectorPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
    SendPacket(BuildOffPacket());
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

void MSIKeyboard1603Controller::Save()
{
    SendPacket(BuildSavePacket());
}

std::string MSIKeyboard1603Controller::GetDeviceName()
{
    std::string manufacturer = "MSI";
    std::string product      = "Mystic Light MS-1603";
    wchar_t tname[256]       = {};

    if(dev && (hid_get_manufacturer_string(dev, tname, 256) == 0) && (tname[0] != L'\0'))
    {
        manufacturer = StringUtils::wstring_to_string(tname);
    }
    else
    {
        LOG_WARNING("[MSIKeyboard1603Controller] Failed to read the HID manufacturer string");
    }

    tname[0] = L'\0';

    if(dev && (hid_get_product_string(dev, tname, 256) == 0) && (tname[0] != L'\0'))
    {
        product = StringUtils::wstring_to_string(tname);
    }
    else
    {
        LOG_WARNING("[MSIKeyboard1603Controller] Failed to read the HID product string");
    }

    return(manufacturer + " " + product);
}

std::string MSIKeyboard1603Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string MSIKeyboard1603Controller::GetFWVersion()
{
    return("");
}

std::string MSIKeyboard1603Controller::GetSerial()
{
    wchar_t serial_string[128] = {};

    if(!dev || (hid_get_serial_number_string(dev, serial_string, 128) != 0))
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool MSIKeyboard1603Controller::SendPacket(const Packet& packet)
{
    if(!dev)
    {
        LOG_ERROR("[MSIKeyboard1603Controller] Cannot send a HID feature report without an open device");
        return(false);
    }

    const int bytes_written = hid_send_feature_report(dev, packet.data(), packet.size());

    if(bytes_written != static_cast<int>(packet.size()))
    {
        const wchar_t* error = hid_error(dev);

        LOG_ERROR("[MSIKeyboard1603Controller] Failed to send HID feature report (%d/%u bytes): %ls",
                  bytes_written,
                  static_cast<unsigned int>(packet.size()),
                  error ? error : L"unknown HID error");
        return(false);
    }

    return(true);
}

void MSIKeyboard1603Controller::ApplyStaticColor(std::size_t zone, MSI1603Color color)
{
    if(zone >= ZoneCount)
    {
        return;
    }

    SendPacket(BuildSelectorPacket(zone));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));

    SendPacket(BuildStaticPacket(color));
    std::this_thread::sleep_for(std::chrono::milliseconds(PacketDelayMs));
}

std::size_t MSIKeyboard1603Controller::NormalizeSpeed(unsigned int speed)
{
    return(std::min<std::size_t>(speed, 2));
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildSavePacket() const
{
    Packet packet = {};

    packet[0] = 0x02;
    packet[1] = 0xA0;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildOffPacket() const
{
    Packet packet = {};

    packet[0] = 0x02;
    packet[1] = 0x02;
    packet[2] = 0x00;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildSelectorPacket(std::size_t zone) const
{
    Packet packet = {};
    const ZoneMask& mask = GetZoneMasks()[zone];

    packet[0] = 0x02;
    packet[1] = 0x01;
    packet[2] = mask[0];
    packet[3] = mask[1];
    packet[4] = mask[2];

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildAllSelectorPacket() const
{
    Packet packet = {};

    packet[0] = 0x02;
    packet[1] = 0x01;

    for(std::size_t idx = 2; idx < 34; idx++)
    {
        packet[idx] = 0xFF;
    }

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildStaticPacket(MSI1603Color color) const
{
    Packet packet = {};

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x01;
    packet[3]  = 0x90;
    packet[4]  = 0x01;
    packet[5]  = 0x00;
    packet[6]  = 0x00;
    packet[7]  = 0x0A;
    packet[8]  = 0x01;
    packet[9]  = 0x00;
    packet[10] = 0x00;
    packet[11] = color.R;
    packet[12] = color.G;
    packet[13] = color.B;
    packet[14] = 0x64;
    packet[15] = color.R;
    packet[16] = color.G;
    packet[17] = color.B;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildBreathingPacket(MSI1603Color color, unsigned int speed) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = breathing_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x02;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x01;
    packet[9]  = 0x01;
    packet[10] = 0x00;
    packet[11] = color.R;
    packet[12] = color.G;
    packet[13] = color.B;
    packet[14] = 0x64;
    packet[15] = color.R;
    packet[16] = color.G;
    packet[17] = color.B;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildBreathingPacket(MSI1603Color primary, MSI1603Color secondary, unsigned int speed) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = breathing_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x02;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x01;
    packet[9]  = 0x01;
    packet[10] = 0x00;
    packet[11] = primary.R;
    packet[12] = primary.G;
    packet[13] = primary.B;
    packet[14] = 0x32;
    packet[15] = secondary.R;
    packet[16] = secondary.G;
    packet[17] = secondary.B;
    packet[18] = 0x64;
    packet[19] = primary.R;
    packet[20] = primary.G;
    packet[21] = primary.B;
    packet[22] = 0x00;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildBouncyWallPacket(MSI1603Color color) const
{
    Packet packet = {};

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x07;
    packet[3]  = 0x64;
    packet[4]  = 0x00;
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x02;
    packet[8]  = 0x00;
    packet[9]  = 0x00;
    packet[10] = 0x00;
    packet[11] = 0x00;
    packet[12] = 0x00;
    packet[13] = 0x00;
    packet[14] = 0x32;
    packet[15] = color.R;
    packet[16] = color.G;
    packet[17] = color.B;
    packet[18] = 0x64;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildRainbowBouncyWallPacket() const
{
    Packet packet = {};

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x07;
    packet[3]  = 0x64;
    packet[4]  = 0x00;
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x02;
    packet[8]  = 0x01;
    packet[9]  = 0x00;
    packet[10] = 0x00;
    packet[11] = 0x00;
    packet[12] = 0x00;
    packet[13] = 0x00;
    packet[14] = 0x64;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildColorCyclePacket(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = breathing_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x03;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x00;
    packet[9]  = 0x00;
    packet[10] = 0x00;
    packet[11] = primary.R;
    packet[12] = primary.G;
    packet[13] = primary.B;
    packet[14] = color_cycle_markers[0];
    packet[15] = secondary.R;
    packet[16] = secondary.G;
    packet[17] = secondary.B;
    packet[18] = color_cycle_markers[1];
    packet[19] = tertiary.R;
    packet[20] = tertiary.G;
    packet[21] = tertiary.B;
    packet[22] = color_cycle_markers[2];
    packet[23] = primary.R;
    packet[24] = primary.G;
    packet[25] = primary.B;

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildRainbowColorCyclePacket(unsigned int speed) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = breathing_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x03;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x00;
    packet[9]  = 0x00;
    packet[10] = 0x00;
    packet[11] = rainbow_wheel_colors[0].R;
    packet[12] = rainbow_wheel_colors[0].G;
    packet[13] = rainbow_wheel_colors[0].B;

    std::size_t offset = 14;

    for(std::size_t idx = 0; idx < rainbow_wheel_markers.size(); idx++)
    {
        packet[offset]     = rainbow_wheel_markers[idx];
        packet[offset + 1] = rainbow_wheel_colors[idx + 1].R;
        packet[offset + 2] = rainbow_wheel_colors[idx + 1].G;
        packet[offset + 3] = rainbow_wheel_colors[idx + 1].B;
        offset += 4;
    }

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildWavePacket(const std::array<MSI1603Color, 5>& colors, unsigned int speed, unsigned int direction) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = wave_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x04;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x00;
    packet[9]  = direction == wave_direction_right ? 0x01 : 0x00;
    packet[10] = 0x00;
    packet[11] = colors[0].R;
    packet[12] = colors[0].G;
    packet[13] = colors[0].B;

    std::size_t offset = 14;

    for(std::size_t idx = 0; idx < wave_markers.size(); idx++)
    {
        packet[offset]     = wave_markers[idx];
        packet[offset + 1] = colors[(idx + 1) % colors.size()].R;
        packet[offset + 2] = colors[(idx + 1) % colors.size()].G;
        packet[offset + 3] = colors[(idx + 1) % colors.size()].B;
        offset += 4;
    }

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildRainbowWavePacket(unsigned int speed, unsigned int direction) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = wave_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x04;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x00;
    packet[9]  = direction == wave_direction_right ? 0x01 : 0x00;
    packet[10] = 0x00;
    packet[11] = rainbow_wheel_colors[0].R;
    packet[12] = rainbow_wheel_colors[0].G;
    packet[13] = rainbow_wheel_colors[0].B;

    std::size_t offset = 14;

    for(std::size_t idx = 0; idx < rainbow_wheel_markers.size(); idx++)
    {
        packet[offset]     = rainbow_wheel_markers[idx];
        packet[offset + 1] = rainbow_wheel_colors[idx + 1].R;
        packet[offset + 2] = rainbow_wheel_colors[idx + 1].G;
        packet[offset + 3] = rainbow_wheel_colors[idx + 1].B;
        offset += 4;
    }

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildDancePacket(unsigned int speed) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = dance_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x05;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x32;
    packet[7]  = 0x08;
    packet[8]  = 0x01;
    packet[9]  = 0x01;
    packet[10] = 0x00;
    packet[11] = dance_colors[0].R;
    packet[12] = dance_colors[0].G;
    packet[13] = dance_colors[0].B;

    std::size_t offset = 14;

    for(std::size_t idx = 0; idx < dance_markers.size(); idx++)
    {
        packet[offset]     = dance_markers[idx];
        packet[offset + 1] = dance_colors[idx + 1].R;
        packet[offset + 2] = dance_colors[idx + 1].G;
        packet[offset + 3] = dance_colors[idx + 1].B;
        offset += 4;
    }

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildWheelPacket(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed, unsigned int direction) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = wheel_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x06;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x00;
    packet[7]  = 0x03;
    packet[8]  = 0x00;
    packet[9]  = direction == wheel_direction_right ? 0x00 : 0x01;
    packet[10] = 0x00;
    packet[11] = 0x00;
    packet[12] = 0x00;
    packet[13] = 0x00;
    packet[14] = wheel_markers[0];
    packet[15] = primary.R;
    packet[16] = primary.G;
    packet[17] = primary.B;
    packet[18] = wheel_markers[1];
    packet[19] = secondary.R;
    packet[20] = secondary.G;
    packet[21] = secondary.B;
    packet[22] = wheel_markers[2];
    packet[23] = tertiary.R;
    packet[24] = tertiary.G;
    packet[25] = tertiary.B;
    packet[26] = wheel_markers[3];

    return(packet);
}

MSIKeyboard1603Controller::Packet MSIKeyboard1603Controller::BuildRainbowWheelPacket(unsigned int speed, unsigned int direction) const
{
    Packet packet = {};
    const std::array<unsigned char, 2>& selected_speed = wheel_speeds[NormalizeSpeed(speed)];

    packet[0]  = 0x02;
    packet[1]  = 0x02;
    packet[2]  = 0x06;
    packet[3]  = selected_speed[0];
    packet[4]  = selected_speed[1];
    packet[5]  = 0x00;
    packet[6]  = 0x00;
    packet[7]  = 0x05;
    packet[8]  = 0x01;
    packet[9]  = direction == wheel_direction_right ? 0x00 : 0x01;
    packet[10] = 0x00;
    packet[11] = rainbow_wheel_colors[0].R;
    packet[12] = rainbow_wheel_colors[0].G;
    packet[13] = rainbow_wheel_colors[0].B;

    std::size_t offset = 14;

    for(std::size_t idx = 0; idx < rainbow_wheel_markers.size(); idx++)
    {
        packet[offset]     = rainbow_wheel_markers[idx];
        packet[offset + 1] = rainbow_wheel_colors[idx + 1].R;
        packet[offset + 2] = rainbow_wheel_colors[idx + 1].G;
        packet[offset + 3] = rainbow_wheel_colors[idx + 1].B;
        offset += 4;
    }

    return(packet);
}

const std::array<MSIKeyboard1603Controller::ZoneMask, MSIKeyboard1603Controller::ZoneCount>& MSIKeyboard1603Controller::GetZoneMasks()
{
    return(zone_masks);
}

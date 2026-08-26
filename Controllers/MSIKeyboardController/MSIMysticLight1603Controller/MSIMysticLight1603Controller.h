/*---------------------------------------------------------*\
| MSIMysticLight1603Controller.h                            |
|                                                           |
|   Driver for MSI Mystic Light MS-1603 keyboards           |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <hidapi.h>

struct MSI1603Color
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
};

class MSIMysticLight1603Controller
{
public:
    using ZoneMask = std::array<unsigned char, 3>;
    using Packet   = std::array<unsigned char, 64>;

    static constexpr std::size_t ZoneCount = 24;

    MSIMysticLight1603Controller
        (
        hid_device* handle,
        const char* path
        );
    ~MSIMysticLight1603Controller();

    void            SetStatic(MSI1603Color color, std::size_t zone);
    void            SetStatic(const std::vector<MSI1603Color>& colors);
    void            SetBreathing(MSI1603Color color, unsigned int speed);
    void            SetBreathing(MSI1603Color primary, MSI1603Color secondary, unsigned int speed);
    void            SetRainbowBreathing(unsigned int speed);
    void            SetBouncyWall(MSI1603Color color);
    void            SetRainbowBouncyWall();
    void            SetColorCycle(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed);
    void            SetRainbowColorCycle(unsigned int speed);
    void            SetWave(const std::array<MSI1603Color, 5>& colors, unsigned int speed, unsigned int direction);
    void            SetRainbowWave(unsigned int speed, unsigned int direction);
    void            SetDance(unsigned int speed);
    void            SetWheel(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed, unsigned int direction);
    void            SetRainbowWheel(unsigned int speed, unsigned int direction);
    void            SetOff();
    void            Save();

    std::string     GetDeviceName();
    std::string     GetDeviceLocation();
    std::string     GetFWVersion();
    std::string     GetSerial();

private:
    static constexpr unsigned int PacketDelayMs = 10;

    bool                        SendPacket(const Packet& packet);
    void                        ApplyStaticColor(std::size_t zone, MSI1603Color color);
    static std::size_t          NormalizeSpeed(unsigned int speed);
    Packet                      BuildSavePacket() const;
    Packet                      BuildOffPacket() const;
    Packet                      BuildSelectorPacket(std::size_t zone) const;
    Packet                      BuildAllSelectorPacket() const;
    Packet                      BuildStaticPacket(MSI1603Color color) const;
    Packet                      BuildBreathingPacket(MSI1603Color color, unsigned int speed) const;
    Packet                      BuildBreathingPacket(MSI1603Color primary, MSI1603Color secondary, unsigned int speed) const;
    Packet                      BuildBouncyWallPacket(MSI1603Color color) const;
    Packet                      BuildRainbowBouncyWallPacket() const;
    Packet                      BuildColorCyclePacket(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed) const;
    Packet                      BuildRainbowColorCyclePacket(unsigned int speed) const;
    Packet                      BuildWavePacket(const std::array<MSI1603Color, 5>& colors, unsigned int speed, unsigned int direction) const;
    Packet                      BuildRainbowWavePacket(unsigned int speed, unsigned int direction) const;
    Packet                      BuildDancePacket(unsigned int speed) const;
    Packet                      BuildWheelPacket(MSI1603Color primary, MSI1603Color secondary, MSI1603Color tertiary, unsigned int speed, unsigned int direction) const;
    Packet                      BuildRainbowWheelPacket(unsigned int speed, unsigned int direction) const;
    static const std::array<ZoneMask, ZoneCount>& GetZoneMasks();

    hid_device*                 dev;
    std::string                 location;
};

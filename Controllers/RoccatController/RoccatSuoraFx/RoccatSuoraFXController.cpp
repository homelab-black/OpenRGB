/*---------------------------------------------------------*\
|  RoccatSuoraFXController.cpp                              |
|                                                           |
|  Driver for Roccat Suora FX Keyboard                      |
|                                                           |
|  AirPe                                         12 Feb 2026|
|                                                           |
|  This file is part of the OpenRGB project                 |
|  SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#include "RoccatSuoraFXController.h"
#include <cstring>
#include <thread>
#include <chrono>

RoccatSuoraFXController::RoccatSuoraFXController(hid_device* dev_handle, const char* path)
{
    handle   = dev_handle;
    location = path;
}

RoccatSuoraFXController::~RoccatSuoraFXController()
{
    hid_close(handle);
}

std::string RoccatSuoraFXController::GetDeviceLocation()
{
    return location;
}

unsigned char RoccatSuoraFXController::CalculateChecksum(unsigned char* data, int len)
{
    unsigned int sum = 0;
    for(int i = 0; i < len; i++)
    {
        sum += data[i];
    }
    return (unsigned char)((0xFF - sum) & 0xFF);
}

void RoccatSuoraFXController::SendControlReport(unsigned char* data, int len)
{
    unsigned char buffer[9];
    std::memset(buffer, 0, sizeof(buffer));

    /* Report ID 3 is used for Suora FX control features */
    buffer[0] = 0x03;
    std::memcpy(&buffer[1], data, len);
    buffer[len + 1] = CalculateChecksum(data, len);

    hid_send_feature_report(handle, buffer, len + 2);
}

void RoccatSuoraFXController::SendInterruptPacket(unsigned char* data, int len)
{
    unsigned char buffer[65];
    std::memset(buffer, 0, sizeof(buffer));

    /* Report ID 0 is often used for raw interrupt OUT packets in HIDAPI */
    buffer[0] = 0x00;
    std::memcpy(&buffer[1], data, std::min(len, 64));

    hid_write(handle, buffer, 65);
}

/* 
   New Function: SendKeyMap
   Sends the static key mapping table. This should only be done once
   during initialization, not on every frame update.
*/
void RoccatSuoraFXController::SendKeyMap()
{
    /* Key Assignment Block 1 */
    unsigned char key_head1[] = { 0x11, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00 };
    SendControlReport(key_head1, sizeof(key_head1));
    
    unsigned char key_map1[64] = { 0 };
    unsigned char key_map_data1[] = {
        0x00, 0x01, 0x01, 0xe5, 0x01, 0x26, 0x81, 0x26, 0x01, 0x27, 0x81, 0x27, 0x81, 0xe5, 0x01, 0x50, 
        0x81, 0x50, 0x01, 0xe5, 0x01, 0x16, 0x81, 0x16, 0x81, 0xe5, 0x01, 0x08, 0x81, 0x08, 0x01, 0x06, 
        0x81, 0x06, 0x01, 0x17, 0x81, 0x17, 0x01, 0x12, 0x81, 0x12, 0x01, 0x15, 0x81, 0x15, 0x01, 0x2c, 
        0x81, 0x2c
    };
    std::memcpy(key_map1, key_map_data1, sizeof(key_map_data1));
    SendInterruptPacket(key_map1, 64);
    SendInterruptPacket(key_map1, 0); // Empty padding packets
    SendInterruptPacket(key_map1, 0);

    /* Key Assignment Block 2 */
    unsigned char key_head2[] = { 0x11, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00 };
    SendControlReport(key_head2, sizeof(key_head2));

    unsigned char key_map2[64] = { 0 };
    unsigned char key_map_data2[] = {
        0x00, 0x01, 0x01, 0xe2, 0x01, 0xe0, 0x01, 0x0b, 0x81, 0x0b, 0x81, 0xe2, 0x81, 0xe0
    };
    std::memcpy(key_map2, key_map_data2, sizeof(key_map_data2));
    SendInterruptPacket(key_map2, 64);
    SendInterruptPacket(key_map2, 0);
    SendInterruptPacket(key_map2, 0);
}

void RoccatSuoraFXController::InitializeCustomMode()
{
    /* 1. Unlock Command (Custom Mode 0x05) */
    unsigned char unlock[] = { 0x08, 0x01, 0x01, 0x05, 0x00, 0x08, 0x01 };
    SendControlReport(unlock, sizeof(unlock));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    /* 
       2. Submit Key Map once on init.
       We assume the Key Map configuration persists until power loss
       or a specific reset command. Sending this huge block every frame
       causes flickering.
    */
    SendKeyMap();
}

void RoccatSuoraFXController::SendLightingData(unsigned char* colors)
{
    /* 
       Frame Start: Color Header (0x12)
       Allows the device to sync the following color data.
       We MUST send this every frame before the color packets.
    */
    unsigned char header[] = { 0x12, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00 };
    SendControlReport(header, sizeof(header));

    /* Send 8 interrupt packets (64 bytes each) */
    for(int i = 0; i < 8; i++)
    {
        SendInterruptPacket(&colors[i * 64], 64);
    }
}

void RoccatSuoraFXController::SetEffect(unsigned char effect, unsigned char speed, unsigned char brightness, unsigned char color, unsigned char variant)
{
    unsigned char buffer[7];
    buffer[0] = 0x08;
    buffer[1] = 0x02;
    buffer[2] = effect;
    buffer[3] = speed;
    buffer[4] = brightness;
    buffer[5] = color;
    buffer[6] = variant;

    SendControlReport(buffer, sizeof(buffer));
}

void RoccatSuoraFXController::ApplyLighting()
{
    /* 
       Final Apply Command (Effect 0x33)
       Byte 3 is Speed. 0x00 = Instant/Fastest. 0x05 = Medium.
       We MUST use 0x00 to prevent fading between frames (flicker).
    */
    unsigned char apply[] = { 0x08, 0x02, 0x33, 0x00, 0x32, 0x08, 0x01 };
    SendControlReport(apply, sizeof(apply));
}

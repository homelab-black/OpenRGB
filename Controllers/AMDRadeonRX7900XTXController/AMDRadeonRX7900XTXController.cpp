/*---------------------------------------------------------*\
| AMDRadeonRX7900XTXController.cpp                          |
|                                                           |
|   AMD Radeon RX 7900 XTX reference RGB over SMU I2C       |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "AMDRadeonRX7900XTXController.h"
#include "pci_ids.h"

AMDRadeonRX7900XTXController::AMDRadeonRX7900XTXController(i2c_smbus_interface* bus_ptr)
{
    bus         = bus_ptr;
    detected    = false;
    initialized = false;
}

bool AMDRadeonRX7900XTXController::IsSupportedBus(const i2c_smbus_interface* bus)
{
    /*-----------------------------------------------------*\
    | This is NOT the display/DDC/OEM I2C bus used by AIB   |
    | cards. Never probe those buses, SMU 0, or other cards.|
    \*-----------------------------------------------------*/
    const char bus_name[] = "AMDGPU SMU 1";

    return bus->info.pci_vendor           == AMD_GPU_VEN
        && bus->info.pci_device           == AMD_NAVI31_DEV
        && bus->info.pci_subsystem_vendor == AMD_GPU_VEN
        && bus->info.pci_subsystem_device == AMD_RX7900XTX_REFERENCE_SUB_DEV
        && strncmp(bus->info.device_name, bus_name, sizeof(bus_name) - 1) == 0
        && (bus->info.device_name[sizeof(bus_name) - 1] == '\0'
         || strncmp(bus->info.device_name + sizeof(bus_name) - 1, " (/dev/i2c-", 11) == 0);
}

std::string AMDRadeonRX7900XTXController::GetLocation()
{
    return "I2C: " + std::string(bus->info.device_name) + ", address 0x5A";
}

bool AMDRadeonRX7900XTXController::Detect()
{
    std::lock_guard<std::mutex> lock(mutex);
    detected    = false;
    initialized = false;

    if(!IsSupportedBus(bus))
    {
        return false;
    }

    /*-----------------------------------------------------*\
    | Captured read-only identification query. No lighting  |
    | changes or initialization writes during detection.    |
    \*-----------------------------------------------------*/
    unsigned char request[64]  = { 0x12, 0x02 };
    unsigned char response[64] = {};

    detected = Transfer(request, response)
            && response[4] == 0x01;
    return detected;
}

bool AMDRadeonRX7900XTXController::Transfer(const unsigned char* request, unsigned char* response)
{
    /*-----------------------------------------------------*\
    | Frame each command as three independent raw writes.   |
    | Do not use SMBus block framing or repeated starts.    |
    | See issue #3037, note 1710647123, debug_logs.zip.     |
    \*-----------------------------------------------------*/
    const unsigned char write_headers[] = { 0x40, 0x57, 0xAE };
    const int payload_sizes[]           = { 23, 23, 18 };
    unsigned char reply[64]             = {};
    int offset                         = 0;

    for(unsigned int packet = 0; packet < 3; packet++)
    {
        unsigned char buffer[24] = {};
        buffer[0] = write_headers[packet];
        memcpy(buffer + 1, request + offset, payload_sizes[packet]);

        if(bus->i2c_write_block(AMD_RX7900XTX_RGB_ADDR, payload_sizes[packet] + 1, buffer) < 0)
        {
            return false;
        }

        offset += payload_sizes[packet];
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    /*-----------------------------------------------------*\
    | Every read starts with 0xFF minus the payload offset. |
    | The Windows tool reads 23/23/18 payload bytes.        |
    | Long reads failed on the tested Linux SMU adapter.    |
    | Single-payload-byte reads work at its fixed 400 kHz.  |
    \*-----------------------------------------------------*/
    for(unsigned int byte = 0; byte < sizeof(reply); byte++)
    {
        unsigned char buffer[2] = {};
        int size = sizeof(buffer);

        if(bus->i2c_read_block(AMD_RX7900XTX_RGB_ADDR, &size, buffer) < 0
        || size != 2
        || buffer[0] != 0xFF - byte)
        {
            return false;
        }

        reply[byte] = buffer[1];
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /*-----------------------------------------------------*\
    | All captured replies echo the command/subcommand.     |
    \*-----------------------------------------------------*/
    if(memcmp(request, reply, 4) != 0)
    {
        return false;
    }

    if(response)
    {
        memcpy(response, reply, sizeof(reply));
    }

    return true;
}

bool AMDRadeonRX7900XTXController::Apply()
{
    unsigned char request[64] = { 0x51, 0x28, 0x00, 0x00, 0xE0 };
    return Transfer(request);
}

bool AMDRadeonRX7900XTXController::SetMode(unsigned char mode, unsigned char brightness, RGBColor color)
{
    std::lock_guard<std::mutex> lock(mutex);

    if(!detected
    || (mode != AMD_RX7900XTX_MODE_STATIC && mode != AMD_RX7900XTX_MODE_BREATHING
     && mode != AMD_RX7900XTX_MODE_CYCLE  && mode != AMD_RX7900XTX_MODE_OFF))
    {
        return false;
    }

    if(!UpdateMode(mode, brightness, color))
    {
        /*-------------------------------------------------*\
        | Stop on any failed/invalid reply. Require a fresh |
        | detection rather than continuing partial writes.  |
        \*-------------------------------------------------*/
        detected = false;
        return false;
    }

    return true;
}

bool AMDRadeonRX7900XTXController::UpdateMode(unsigned char mode, unsigned char brightness, RGBColor color)
{
    if(!initialized)
    {
        unsigned char enable[64] = { 0x41, 0x80 };
        unsigned char reset[64]  = { 0x51, 0x96 };

        if(!Transfer(enable) || !Transfer(reset) || !Apply())
        {
            return false;
        }

        initialized = true;
    }

    if(mode != AMD_RX7900XTX_MODE_OFF)
    {
        unsigned char effect[64];
        memset(effect, 0xFF, sizeof(effect));

        effect[0]  = 0x51;
        effect[1]  = 0x2C;
        effect[2]  = 0x01;
        effect[3]  = 0x00;
        effect[4]  = mode;
        effect[5]  = (mode == AMD_RX7900XTX_MODE_BREATHING) ? 0x31
                   : (mode == AMD_RX7900XTX_MODE_CYCLE) ? 0x80 : 0xFF;
        effect[6]  = (mode == AMD_RX7900XTX_MODE_BREATHING) ? 0x20 : 0x00;
        effect[7]  = (mode == AMD_RX7900XTX_MODE_BREATHING) ? 0x03 : 0xFF;
        effect[9]  = brightness;
        effect[10] = RGBGetRValue(color);
        effect[11] = RGBGetGValue(color);
        effect[12] = RGBGetBValue(color);
        memset(effect + 13, 0x00, 3);

        if(!Transfer(effect))
        {
            return false;
        }
    }

    /*-----------------------------------------------------*\
    | Map all 15 physical LEDs to the same effect channel.  |
    | Expose one logical light bar, not an unverified map.  |
    \*-----------------------------------------------------*/
    unsigned char remap[64] = { 0x51, 0xA0, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x05, 0x06 };
    memset(remap + 10, mode, 15);

    return Transfer(remap) && Apply();
}

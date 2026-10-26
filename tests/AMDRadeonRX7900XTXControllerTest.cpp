/*---------------------------------------------------------*\
| AMDRadeonRX7900XTXControllerTest.cpp                      |
|                                                           |
|   Hardware-free tests for reference RX 7900 XTX RGB       |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <vector>
#include "AMDRadeonRX7900XTXController.h"

#define TRANSFERS_PER_COMMAND 67

struct CapturedCommand
{
    unsigned char data[64];
};

class MockBus : public i2c_smbus_interface
{
public:
    std::vector<CapturedCommand> commands;
    unsigned char request[64] = {};
    int calls                 = 0;
    int fail_at               = -1;
    int bad_at                = -1;
    int short_at              = -1;
    int phase                 = 0;
    bool bad_echo             = false;
    bool wrong_id             = false;

    MockBus()
    {
        memset(&info, 0, sizeof(info));
        info.pci_vendor             = 0x1002;
        info.pci_device             = 0x744C;
        info.pci_subsystem_vendor   = 0x1002;
        info.pci_subsystem_device   = 0x0E3B;
        strcpy(info.device_name, "AMDGPU SMU 1 (/dev/i2c-4)");
    }

    s32 i2c_smbus_xfer(u8, char, u8, int, i2c_smbus_data*) override
    {
        assert(false && "Must use raw I2C, not SMBus framing");
        return -1;
    }

    s32 i2c_xfer(u8 address, char read_write, int* size, u8* data) override
    {
        const unsigned char write_headers[] = { 0x40, 0x57, 0xAE };
        const int offsets[]                 = { 0, 23, 46 };
        const int sizes[]                   = { 24, 24, 19 };
        assert(address == 0x5A);
        assert(*size == (phase < 3 ? sizes[phase] : 2));
        assert(read_write == (phase < 3 ? I2C_SMBUS_WRITE : I2C_SMBUS_READ));

        int call = calls++;
        if(call == fail_at)
        {
            return -1;
        }

        if(read_write == I2C_SMBUS_WRITE)
        {
            assert(data[0] == write_headers[phase]);
            memcpy(request + offsets[phase], data + 1, *size - 1);
            if(phase == 2)
            {
                CapturedCommand command;
                memcpy(command.data, request, sizeof(request));
                commands.push_back(command);
            }
        }
        else
        {
            int byte = phase - 3;
            data[0] = (call == bad_at) ? 0x00 : 0xFF - byte;
            data[1] = request[byte];
            if(request[0] == 0x12 && byte == 4)
            {
                data[1] = wrong_id ? 0x00 : 0x01;
            }
            if(bad_echo && byte == 0)
            {
                data[1] ^= 0x01;
            }
            if(call == short_at)
            {
                --*size;
            }
        }
        phase = (phase + 1) % TRANSFERS_PER_COMMAND;
        return 1;
    }
};

static void CheckCommand(const CapturedCommand& command, const unsigned char* expected)
{
    assert(memcmp(command.data, expected, sizeof(command.data)) == 0);
}

static void TestBusGating()
{
    MockBus bus;
    assert(AMDRadeonRX7900XTXController::IsSupportedBus(&bus));
    const char* rejected[] =
    {
        "AMDGPU SMU 0 (/dev/i2c-3)", "AMDGPU SMU 10", "AMDGPU SMU 1x",
        "AMDGPU DM i2c hw bus 0", "AMDGPU DM i2c OEM bus", "AMDGPU DM aux hw bus 0", "AMD ADL"
    };
    for(unsigned int i = 0; i < sizeof(rejected) / sizeof(rejected[0]); i++)
    {
        strcpy(bus.info.device_name, rejected[i]);
        AMDRadeonRX7900XTXController controller(&bus);
        assert(!controller.Detect());
        assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
        assert(bus.calls == 0);
    }
    strcpy(bus.info.device_name, "AMDGPU SMU 1");
    assert(AMDRadeonRX7900XTXController::IsSupportedBus(&bus));
    strcpy(bus.info.device_name, "AMDGPU SMU 1 (/dev/i2c-42)");
    assert(AMDRadeonRX7900XTXController::IsSupportedBus(&bus));
    int* ids[] = { &bus.info.pci_vendor, &bus.info.pci_device,
                   &bus.info.pci_subsystem_vendor, &bus.info.pci_subsystem_device };
    for(unsigned int i = 0; i < 4; i++)
    {
        int original = *ids[i];
        *ids[i] = 0;
        AMDRadeonRX7900XTXController controller(&bus);
        assert(!controller.Detect());
        assert(bus.calls == 0);
        *ids[i] = original;
    }
}

static void TestCapturedCommands()
{
    MockBus bus;
    AMDRadeonRX7900XTXController controller(&bus);
    assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
    assert(bus.calls == 0);
    assert(controller.Detect());
    assert(bus.commands.size() == 1);
    unsigned char query[64] = { 0x12, 0x02 };
    CheckCommand(bus.commands[0], query);

    /*-----------------------------------------------------*\
    | Full 64-byte commands reconstructed from the captures.|
    | Transport headers/lengths are checked by MockBus.     |
    \*-----------------------------------------------------*/
    const unsigned char captured_red_prefix[] =
    {
        0x51, 0x2C, 0x01, 0x00, 0x00, 0xFF, 0x00, 0xFF,
        0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    unsigned char red[64];
    memset(red, 0xFF, sizeof(red));
    memcpy(red, captured_red_prefix, sizeof(captured_red_prefix));
    unsigned char remap[64]  = { 0x51, 0xA0, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00, 0x05, 0x06 };
    unsigned char apply[64]  = { 0x51, 0x28, 0x00, 0x00, 0xE0 };
    unsigned char enable[64] = { 0x41, 0x80 };
    unsigned char reset[64]  = { 0x51, 0x96 };
    assert(controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, ToRGBColor(255, 0, 0)));
    assert(bus.commands.size() == 7);
    CheckCommand(bus.commands[1], enable);
    CheckCommand(bus.commands[2], reset);
    CheckCommand(bus.commands[3], apply);
    CheckCommand(bus.commands[4], red);
    CheckCommand(bus.commands[5], remap);
    CheckCommand(bus.commands[6], apply);

    assert(controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, ToRGBColor(0, 0, 255)));
    red[10] = 0;
    red[12] = 255;
    CheckCommand(bus.commands[7], red);
    assert(bus.commands.size() == 10);

    const unsigned char brightness_values[] = { 51, 102, 153, 204 };
    for(unsigned int i = 0; i < sizeof(brightness_values); i++)
    {
        assert(controller.SetMode(AMD_RX7900XTX_MODE_STATIC, brightness_values[i], ToRGBColor(255, 255, 255)));
        red[9]  = brightness_values[i];
        red[10] = 255;
        red[11] = 255;
        CheckCommand(bus.commands[bus.commands.size() - 3], red);
    }

    assert(controller.SetMode(AMD_RX7900XTX_MODE_BREATHING, 255, ToRGBColor(255, 0, 0)));
    red[4]  = 1;
    red[5]  = 0x31;
    red[6]  = 0x20;
    red[7]  = 3;
    red[9]  = 255;
    red[11] = 0;
    red[12] = 0;
    memset(remap + 10, 1, 15);
    CheckCommand(bus.commands[bus.commands.size() - 3], red);
    CheckCommand(bus.commands[bus.commands.size() - 2], remap);

    assert(controller.SetMode(AMD_RX7900XTX_MODE_CYCLE, 127, ToRGBColor(255, 255, 255)));
    red[4]  = 2;
    red[5]  = 0x80;
    red[6]  = 0;
    red[7]  = 255;
    red[9]  = 127;
    red[11] = 255;
    red[12] = 255;
    memset(remap + 10, 2, 15);
    CheckCommand(bus.commands[bus.commands.size() - 3], red);
    CheckCommand(bus.commands[bus.commands.size() - 2], remap);

    assert(controller.SetMode(AMD_RX7900XTX_MODE_OFF, 0, 0));
    memset(remap + 10, 0xFE, 15);
    CheckCommand(bus.commands[bus.commands.size() - 2], remap);
    CheckCommand(bus.commands.back(), apply);
    int calls = bus.calls;
    assert(!controller.SetMode(0xFF, 255, 0));
    assert(bus.calls == calls);
}

static void TestFailures()
{
    /*-----------------------------------------------------*\
    | Every failed identification transfer stops detection. |
    \*-----------------------------------------------------*/
    for(int phase = 0; phase < TRANSFERS_PER_COMMAND; phase++)
    {
        MockBus bus;
        bus.fail_at = phase;
        AMDRadeonRX7900XTXController controller(&bus);
        assert(!controller.Detect());
        assert(bus.calls == phase + 1);
        assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
        assert(bus.calls == phase + 1);
    }
    for(int phase = 3; phase < TRANSFERS_PER_COMMAND; phase++)
    {
        MockBus bus;
        bus.bad_at = phase;
        AMDRadeonRX7900XTXController controller(&bus);
        assert(!controller.Detect());
        assert(bus.calls == phase + 1);
        assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
        assert(bus.calls == phase + 1);
        MockBus short_bus;
        short_bus.short_at = phase;
        AMDRadeonRX7900XTXController short_controller(&short_bus);
        assert(!short_controller.Detect());
        assert(short_bus.calls == phase + 1);
        assert(!short_controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
        assert(short_bus.calls == phase + 1);
    }
    for(int kind = 0; kind < 2; kind++)
    {
        MockBus bus;
        bus.bad_echo = kind == 0;
        bus.wrong_id = kind == 1;
        AMDRadeonRX7900XTXController controller(&bus);
        assert(!controller.Detect());
    }

    /*-----------------------------------------------------*\
    | Fail write/read boundaries in each update command.    |
    \*-----------------------------------------------------*/
    const int positions[] = { 0, 3, TRANSFERS_PER_COMMAND - 1 };
    for(int command = 0; command < 6; command++)
    {
        for(unsigned int position = 0; position < 3; position++)
        {
            MockBus bus;
            AMDRadeonRX7900XTXController controller(&bus);
            assert(controller.Detect());
            bus.fail_at = bus.calls + command * TRANSFERS_PER_COMMAND + positions[position];
            assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
            int calls = bus.calls;
            assert(calls == bus.fail_at + 1);
            assert(!controller.SetMode(AMD_RX7900XTX_MODE_STATIC, 255, 0));
            assert(bus.calls == calls);
        }
    }
}

int main()
{
    TestBusGating();
    TestCapturedCommands();
    TestFailures();
    return 0;
}

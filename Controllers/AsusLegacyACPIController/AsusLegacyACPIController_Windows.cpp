/*---------------------------------------------------------*\
| AsusLegacyACPIController_Windows.cpp                      |
|                                                           |
|   PawnIO ACPI EC controller for legacy motherboard RGB    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "AsusLegacyACPIController_Windows.h"
#include "i2c_smbus_pawnio.h"
#include "LogManager.h"
#include "PawnIOLib.h"

#define ASUS_LEGACY_ACPI_EC_DATA_PORT                0x62
#define ASUS_LEGACY_ACPI_EC_COMMAND_PORT             0x66
#define ASUS_LEGACY_ACPI_EC_STATUS_OUTPUT_FULL       0x01
#define ASUS_LEGACY_ACPI_EC_STATUS_INPUT_FULL        0x02
#define ASUS_LEGACY_ACPI_EC_READ_COMMAND             0x90
#define ASUS_LEGACY_ACPI_EC_WRITE_COMMAND            0x91

#define ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT          0x6B
#define ASUS_LEGACY_ACPI_EC_REGISTER_MODE            0x6C
#define ASUS_LEGACY_ACPI_EC_REGISTER_RED             0x6D
#define ASUS_LEGACY_ACPI_EC_REGISTER_BLUE            0x6E
#define ASUS_LEGACY_ACPI_EC_REGISTER_GREEN           0x6F
#define ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_RED      0x70
#define ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_BLUE     0x71
#define ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_GREEN    0x72
#define ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT          0xFF

#define ASUS_LEGACY_ACPI_EC_TIMEOUT                  100
#define ASUS_LEGACY_ACPI_EC_DELAY                    1

AsusLegacyACPIController_Windows::AsusLegacyACPIController_Windows()
    : pawnio_handle(NULL), ec_mutex(NULL), direct_mode(false), cached_mode(0xFF)
{
}

AsusLegacyACPIController_Windows::~AsusLegacyACPIController_Windows()
{
    if(ec_mutex != NULL)
    {
        CloseHandle(ec_mutex);
    }

    if(pawnio_handle != NULL)
    {
        pawnio_close(pawnio_handle);
    }
}

bool AsusLegacyACPIController_Windows::OpenPawnIO()
{
    if(pawnio_handle != NULL)
    {
        return true;
    }

    const HRESULT status = i2c_smbus_pawnio::start_pawnio("LpcACPIEC.bin", &pawnio_handle);
    if(status != S_OK)
    {
        pawnio_handle = NULL;
        LOG_ERROR("[AsusLegacyACPI] Unable to load LpcACPIEC.bin (HRESULT 0x%08lX)",
            (unsigned long)status);
        return false;
    }

    ec_mutex = CreateMutexA(NULL, FALSE, "Global\\Access_EC");
    if(ec_mutex == NULL)
    {
        LOG_ERROR("[AsusLegacyACPI] Unable to create Access_EC mutex (error %lu)", GetLastError());
        pawnio_close(pawnio_handle);
        pawnio_handle = NULL;
        return false;
    }

    LOG_INFO("[AsusLegacyACPI] PawnIO LpcACPIEC initialized");
    return true;
}

bool AsusLegacyACPIController_Windows::LockEC()
{
    if(!OpenPawnIO())
    {
        return false;
    }

    const DWORD wait_result = WaitForSingleObject(ec_mutex, INFINITE);
    if(wait_result != WAIT_OBJECT_0 && wait_result != WAIT_ABANDONED)
    {
        LOG_ERROR("[AsusLegacyACPI] Unable to acquire Access_EC mutex (error %lu)", GetLastError());
        return false;
    }

    return true;
}

bool AsusLegacyACPIController_Windows::ReadPort(unsigned short port, unsigned char *value)
{
    const ULONG64 input[1] = { port };
    ULONG64 output[1] = {};
    SIZE_T return_size = 0;
    const HRESULT status = pawnio_execute(pawnio_handle, "ioctl_pio_read", input, 1, output, 1, &return_size);

    if(status != S_OK || return_size != 1)
    {
        LOG_ERROR("[AsusLegacyACPI] PawnIO read from port 0x%04X failed (HRESULT 0x%08lX)",
            port, (unsigned long)status);
        return false;
    }

    *value = (unsigned char)output[0];
    return true;
}

bool AsusLegacyACPIController_Windows::WritePort(unsigned short port, unsigned char value)
{
    const ULONG64 input[2] = { port, value };
    SIZE_T return_size = 0;
    const HRESULT status = pawnio_execute(pawnio_handle, "ioctl_pio_write", input, 2, NULL, 0, &return_size);

    if(status != S_OK)
    {
        LOG_ERROR("[AsusLegacyACPI] PawnIO write to port 0x%04X failed (HRESULT 0x%08lX)",
            port, (unsigned long)status);
        return false;
    }

    return true;
}

bool AsusLegacyACPIController_Windows::WaitForInputBufferEmpty()
{
    for(unsigned int attempt = 0; attempt < ASUS_LEGACY_ACPI_EC_TIMEOUT; attempt++)
    {
        unsigned char status = 0;
        if(!ReadPort(ASUS_LEGACY_ACPI_EC_COMMAND_PORT, &status))
        {
            return false;
        }

        if((status & ASUS_LEGACY_ACPI_EC_STATUS_INPUT_FULL) == 0)
        {
            return true;
        }

        Sleep(ASUS_LEGACY_ACPI_EC_DELAY);
    }

    LOG_ERROR("[AsusLegacyACPI] Timed out waiting for the EC input buffer");
    return false;
}

bool AsusLegacyACPIController_Windows::WaitForOutputBufferFull()
{
    for(unsigned int attempt = 0; attempt < ASUS_LEGACY_ACPI_EC_TIMEOUT; attempt++)
    {
        unsigned char status = 0;
        if(!ReadPort(ASUS_LEGACY_ACPI_EC_COMMAND_PORT, &status))
        {
            return false;
        }

        if((status & ASUS_LEGACY_ACPI_EC_STATUS_OUTPUT_FULL) != 0)
        {
            return true;
        }

        Sleep(ASUS_LEGACY_ACPI_EC_DELAY);
    }

    LOG_ERROR("[AsusLegacyACPI] Timed out waiting for the EC output buffer");
    return false;
}

bool AsusLegacyACPIController_Windows::ReadRegister(unsigned char address, unsigned char *value)
{
    unsigned char status = 0;
    if(!ReadPort(ASUS_LEGACY_ACPI_EC_COMMAND_PORT, &status))
    {
        return false;
    }

    /*---------------------------------------------------------*\
    | Drain stale output before beginning a new transaction    |
    \*---------------------------------------------------------*/
    if((status & ASUS_LEGACY_ACPI_EC_STATUS_OUTPUT_FULL) != 0)
    {
        unsigned char stale = 0;
        if(!ReadPort(ASUS_LEGACY_ACPI_EC_DATA_PORT, &stale))
        {
            return false;
        }
    }

    if(!WaitForInputBufferEmpty()
    || !WritePort(ASUS_LEGACY_ACPI_EC_COMMAND_PORT, ASUS_LEGACY_ACPI_EC_READ_COMMAND))
    {
        return false;
    }

    Sleep(ASUS_LEGACY_ACPI_EC_DELAY);

    if(!WaitForInputBufferEmpty()
    || !WritePort(ASUS_LEGACY_ACPI_EC_DATA_PORT, address))
    {
        return false;
    }

    if(!WaitForOutputBufferFull())
    {
        return false;
    }

    return ReadPort(ASUS_LEGACY_ACPI_EC_DATA_PORT, value);
}

bool AsusLegacyACPIController_Windows::ReadRegisters(
    const unsigned char *addresses,
    unsigned char *values,
    size_t count)
{
    if(!LockEC())
    {
        return false;
    }

    bool success = true;

    for(size_t index = 0; index < count; index++)
    {
        if(!ReadRegister(addresses[index], &values[index]))
        {
            success = false;
            break;
        }
    }

    ReleaseMutex(ec_mutex);
    return success;
}

bool AsusLegacyACPIController_Windows::WriteRegister(unsigned char address, unsigned char value)
{
    const unsigned char transaction[] =
    {
        ASUS_LEGACY_ACPI_EC_WRITE_COMMAND,
        address,
        value
    };

    const unsigned short ports[] =
    {
        ASUS_LEGACY_ACPI_EC_COMMAND_PORT,
        ASUS_LEGACY_ACPI_EC_DATA_PORT,
        ASUS_LEGACY_ACPI_EC_DATA_PORT
    };

    for(size_t index = 0; index < sizeof(transaction) / sizeof(transaction[0]); index++)
    {
        if(!WaitForInputBufferEmpty() || !WritePort(ports[index], transaction[index]))
        {
            return false;
        }

        /*-------------------------------------------------*\
        | Give the EC time to latch the byte and raise IBF  |
        \*-------------------------------------------------*/
        Sleep(ASUS_LEGACY_ACPI_EC_DELAY);
    }

    return true;
}

bool AsusLegacyACPIController_Windows::WriteRegisters(
    const asus_legacy_acpi_register *registers,
    size_t count)
{
    if(!LockEC())
    {
        return false;
    }

    bool success = true;

    for(size_t index = 0; index < count; index++)
    {
        if(!WriteRegister(registers[index].address, registers[index].value))
        {
            success = false;
            break;
        }
    }

    ReleaseMutex(ec_mutex);
    return success;
}

std::string AsusLegacyACPIController_Windows::GetBoardName()
{
    if(!board_name.empty())
    {
        return board_name;
    }

    char board_product[256] = {};
    DWORD board_product_size = sizeof(board_product);
    const LSTATUS query_result = RegGetValueA(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\BIOS",
        "BaseBoardProduct",
        RRF_RT_REG_SZ,
        NULL,
        board_product,
        &board_product_size);

    if(query_result == ERROR_SUCCESS)
    {
        board_name = board_product;
    }
    else
    {
        LOG_ERROR("[AsusLegacyACPI] Unable to read BaseBoardProduct (error %ld)", query_result);
    }

    return board_name;
}

bool AsusLegacyACPIController_Windows::IsSupported()
{
    const std::string detected_board = GetBoardName();
    LOG_INFO(
        "[AsusLegacyACPI] Baseboard Product: \"%s\"",
        detected_board.empty() ? "<empty>" : detected_board.c_str());

    if(detected_board.find("MAXIMUS VIII FORMULA") == std::string::npos)
    {
        LOG_INFO("[AsusLegacyACPI] Unsupported baseboard");
        return false;
    }

    return OpenPawnIO();
}

bool AsusLegacyACPIController_Windows::GetColor(RGBColor *color)
{
    if(color == NULL)
    {
        return false;
    }

    const unsigned char addresses[] =
    {
        ASUS_LEGACY_ACPI_EC_REGISTER_RED,
        ASUS_LEGACY_ACPI_EC_REGISTER_GREEN,
        ASUS_LEGACY_ACPI_EC_REGISTER_BLUE
    };
    unsigned char values[sizeof(addresses) / sizeof(addresses[0])] = { 0 };
    const bool success = ReadRegisters(addresses, values, sizeof(addresses) / sizeof(addresses[0]));

    if(!success)
    {
        LOG_WARNING("[AsusLegacyACPI] Unable to read the current EC color");
        return false;
    }

    *color = ToRGBColor(values[0], values[1], values[2]);
    LOG_INFO("[AsusLegacyACPI] Read current color R=%u G=%u B=%u", values[0], values[1], values[2]);
    return true;
}

bool AsusLegacyACPIController_Windows::GetMode(unsigned char *mode)
{
    if(mode == NULL)
    {
        return false;
    }

    const unsigned char addresses[] =
    {
        ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT,
        ASUS_LEGACY_ACPI_EC_REGISTER_MODE
    };
    unsigned char values[sizeof(addresses) / sizeof(addresses[0])] = { 0 };
    const bool success = ReadRegisters(addresses, values, sizeof(addresses) / sizeof(addresses[0]));

    if(success)
    {
        *mode = values[0] == 0x01 ? ASUS_LEGACY_ACPI_MODE_STATIC : values[1];
        LOG_INFO("[AsusLegacyACPI] Read current hardware mode %u", *mode);
    }
    return success;
}

void AsusLegacyACPIController_Windows::SetMode(unsigned char mode, RGBColor color)
{
    if(!direct_mode && cached_mode == mode)
    {
        if(!WriteColorRegisters(color))
        {
            LOG_ERROR("[AsusLegacyACPI] Failed to update motherboard LED color");
        }
        return;
    }

    const unsigned char red   = RGBGetRValue(color);
    const unsigned char green = RGBGetGValue(color);
    const unsigned char blue  = RGBGetBValue(color);

    const asus_legacy_acpi_register registers[] =
    {
        { ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT, 0x00 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT, 0x00 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT, 0x00 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT, 0x00 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT, 0x00 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_RED,    red  },
        { ASUS_LEGACY_ACPI_EC_REGISTER_GREEN,  green},
        { ASUS_LEGACY_ACPI_EC_REGISTER_BLUE,   blue },
        { ASUS_LEGACY_ACPI_EC_REGISTER_MODE,   mode },
        { ASUS_LEGACY_ACPI_EC_REGISTER_COMMIT, 0x00 }
    };

    if(!WriteRegisters(registers, sizeof(registers) / sizeof(registers[0])))
    {
        LOG_ERROR("[AsusLegacyACPI] Failed to apply motherboard LED mode");
    }
    else
    {
        direct_mode = false;
        cached_mode = mode;
        LOG_INFO("[AsusLegacyACPI] Applied hardware mode %u, color R=%u G=%u B=%u", mode, red, green, blue);
    }
}

bool AsusLegacyACPIController_Windows::WriteColorRegisters(RGBColor color)
{
    const asus_legacy_acpi_register registers[] =
    {
        { ASUS_LEGACY_ACPI_EC_REGISTER_RED,   RGBGetRValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_GREEN, RGBGetGValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_BLUE,  RGBGetBValue(color) }
    };

    return WriteRegisters(registers, sizeof(registers) / sizeof(registers[0]));
}

bool AsusLegacyACPIController_Windows::WriteDirectColorRegisters(RGBColor color)
{
    const asus_legacy_acpi_register registers[] =
    {
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_RED,   RGBGetRValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_GREEN, RGBGetGValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_BLUE,  RGBGetBValue(color) }
    };

    return WriteRegisters(registers, sizeof(registers) / sizeof(registers[0]));
}

void AsusLegacyACPIController_Windows::SetDirectColor(RGBColor color)
{
    if(direct_mode)
    {
        if(!WriteDirectColorRegisters(color))
        {
            LOG_ERROR("[AsusLegacyACPI] Failed to stream motherboard LED color");
        }
        return;
    }

    const asus_legacy_acpi_register registers[] =
    {
        { ASUS_LEGACY_ACPI_EC_REGISTER_MODE,         ASUS_LEGACY_ACPI_MODE_STATIC },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT,       0x01 },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_RED,   RGBGetRValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_GREEN, RGBGetGValue(color) },
        { ASUS_LEGACY_ACPI_EC_REGISTER_DIRECT_BLUE,  RGBGetBValue(color) }
    };

    if(!WriteRegisters(registers, sizeof(registers) / sizeof(registers[0])))
    {
        LOG_ERROR("[AsusLegacyACPI] Failed to enter direct motherboard LED mode");
        return;
    }

    direct_mode = true;
    cached_mode = 0xFF;
    LOG_INFO("[AsusLegacyACPI] Direct color streaming enabled");
}

void AsusLegacyACPIController_Windows::SetOff()
{
    SetMode(ASUS_LEGACY_ACPI_MODE_STATIC, 0);
}

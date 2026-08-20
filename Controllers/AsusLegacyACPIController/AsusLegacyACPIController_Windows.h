/*---------------------------------------------------------*\
| AsusLegacyACPIController_Windows.h                        |
|                                                           |
|   PawnIO ACPI EC controller for legacy motherboard RGB    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include <Windows.h>
#include <cstddef>
#include <string>

#define ASUS_LEGACY_ACPI_MODE_DIRECT                 0x00
#define ASUS_LEGACY_ACPI_MODE_STATIC                 0x01
#define ASUS_LEGACY_ACPI_MODE_BREATHING              0x02
#define ASUS_LEGACY_ACPI_MODE_STROBE                 0x03
#define ASUS_LEGACY_ACPI_MODE_RAINBOW                0x04
#define ASUS_LEGACY_ACPI_MODE_OFF                    0xFF

struct asus_legacy_acpi_register
{
    unsigned char address;
    unsigned char value;
};

class AsusLegacyACPIController_Windows
{
public:
    AsusLegacyACPIController_Windows();
    ~AsusLegacyACPIController_Windows();

    bool IsSupported();
    std::string GetBoardName();

    bool GetColor(RGBColor *color);
    bool GetMode(unsigned char *mode);
    void SetMode(unsigned char mode, RGBColor color);
    void SetDirectColor(RGBColor color);
    void SetOff();

private:
    bool OpenPawnIO();
    bool LockEC();
    bool ReadPort(unsigned short port, unsigned char *value);
    bool WritePort(unsigned short port, unsigned char value);
    bool WaitForInputBufferEmpty();
    bool WaitForOutputBufferFull();
    bool ReadRegister(unsigned char address, unsigned char *value);
    bool ReadRegisters(const unsigned char *addresses, unsigned char *values, size_t count);
    bool WriteRegister(unsigned char address, unsigned char value);
    bool WriteRegisters(const asus_legacy_acpi_register *registers, size_t count);
    bool WriteColorRegisters(RGBColor color);
    bool WriteDirectColorRegisters(RGBColor color);

    std::string board_name;
    HANDLE pawnio_handle;
    HANDLE ec_mutex;
    bool direct_mode;
    unsigned char cached_mode;
};

/*---------------------------------------------------------*\
| SamsungSSDController_Windows_Linux.h                      |
|                                                           |
|   Driver for Samsung SSD heatsink LED                     |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
typedef HANDLE  samsung_ssd_handle;
#else
typedef int     samsung_ssd_handle;
#endif

/*---------------------------------------------------------*\
| The LED state goes to the drive in an NVMe Security Send  |
| command with the Samsung vendor protocol. Magician sends  |
| other commands over the same protocol, so only this       |
| exact protocol specific value is allowed.                 |
\*---------------------------------------------------------*/
#define SAMSUNG_SSD_SECURITY_PROTOCOL   0xFE
#define SAMSUNG_SSD_SPSP_LED            0x0033
#define SAMSUNG_SSD_LED_PAYLOAD_SIZE    512

/*---------------------------------------------------------*\
| Model prefix of the drives that have the heatsink LED     |
\*---------------------------------------------------------*/
#define SAMSUNG_SSD_990_PRO_HEATSINK    "Samsung SSD 990 PRO with Heatsink"

enum
{
    SAMSUNG_SSD_LED_TOGGLE_OFF          = 0x01,
    SAMSUNG_SSD_LED_TOGGLE_ON           = 0x02,
};

enum
{
    SAMSUNG_SSD_LED_MODE_STATIC         = 0x01,
    SAMSUNG_SSD_LED_MODE_BLINKING       = 0x02,
    SAMSUNG_SSD_LED_MODE_BREATHING      = 0x03,
};

/*---------------------------------------------------------*\
| Payload layout, the remaining bytes are zero:             |
|   0x00 uint32 toggle (little endian)                      |
|   0x04 uint32 mode   (little endian)                      |
|   0x08 blue, 0x09 green, 0x0A red                         |
\*---------------------------------------------------------*/
enum
{
    SAMSUNG_SSD_LED_IDX_TOGGLE          = 0x00,
    SAMSUNG_SSD_LED_IDX_MODE            = 0x04,
    SAMSUNG_SSD_LED_IDX_BLUE            = 0x08,
    SAMSUNG_SSD_LED_IDX_GREEN           = 0x09,
    SAMSUNG_SSD_LED_IDX_RED             = 0x0A,
    SAMSUNG_SSD_LED_STATE_SIZE          = 0x0B,
};

class SamsungSSDController
{
public:
    SamsungSSDController(samsung_ssd_handle dev_handle, const std::string& dev_path, const std::string& dev_model, const std::string& dev_serial, const std::string& dev_firmware);
    ~SamsungSSDController();

    std::string GetFirmwareVersion();
    std::string GetLocation();
    std::string GetName();
    std::string GetSerial();

    void        SetLED(bool on, unsigned char mode, unsigned char red, unsigned char green, unsigned char blue);

private:
    samsung_ssd_handle  handle;
    std::string         path;
    std::string         model;
    std::string         serial;
    std::string         firmware;

    unsigned char       last_state[SAMSUNG_SSD_LED_STATE_SIZE];
    bool                last_state_valid;

    bool                SecuritySend(unsigned char* data, unsigned int size);
};

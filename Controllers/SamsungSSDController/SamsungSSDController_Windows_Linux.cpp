/*---------------------------------------------------------*\
| SamsungSSDController_Windows_Linux.cpp                    |
|                                                           |
|   Driver for Samsung SSD heatsink LED                     |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "LogManager.h"
#include "SamsungSSDController_Windows_Linux.h"

SamsungSSDController::SamsungSSDController(samsung_ssd_handle dev_handle, const std::string& dev_path, const std::string& dev_model, const std::string& dev_serial, const std::string& dev_firmware)
{
    handle              = dev_handle;
    path                = dev_path;
    model               = dev_model;
    serial              = dev_serial;
    firmware            = dev_firmware;
    last_state_valid    = false;

    memset(last_state, 0, sizeof(last_state));
}

std::string SamsungSSDController::GetFirmwareVersion()
{
    return(firmware);
}

std::string SamsungSSDController::GetLocation()
{
    return("NVMe: " + path);
}

std::string SamsungSSDController::GetName()
{
    return(model);
}

std::string SamsungSSDController::GetSerial()
{
    return(serial);
}

void SamsungSSDController::SetLED(bool on, unsigned char mode, unsigned char red, unsigned char green, unsigned char blue)
{
    /*-----------------------------------------------------*\
    | NVMe data buffers must be dword aligned, and the      |
    | Windows NVMe adapter reports an alignment mask of 3   |
    \*-----------------------------------------------------*/
    alignas(16) unsigned char payload[SAMSUNG_SSD_LED_PAYLOAD_SIZE];

    memset(payload, 0, sizeof(payload));

    payload[SAMSUNG_SSD_LED_IDX_TOGGLE] = on ? SAMSUNG_SSD_LED_TOGGLE_ON : SAMSUNG_SSD_LED_TOGGLE_OFF;
    payload[SAMSUNG_SSD_LED_IDX_MODE]   = mode;
    payload[SAMSUNG_SSD_LED_IDX_BLUE]   = blue;
    payload[SAMSUNG_SSD_LED_IDX_GREEN]  = green;
    payload[SAMSUNG_SSD_LED_IDX_RED]    = red;

    /*-----------------------------------------------------*\
    | Skip a state that the drive already has. It is not   |
    | known if the drive keeps the state in flash memory,   |
    | so repeated writes are not sent.                      |
    \*-----------------------------------------------------*/
    if(last_state_valid && memcmp(last_state, payload, SAMSUNG_SSD_LED_STATE_SIZE) == 0)
    {
        return;
    }

    if(SecuritySend(payload, sizeof(payload)))
    {
        memcpy(last_state, payload, SAMSUNG_SSD_LED_STATE_SIZE);
        last_state_valid = true;
    }
    else
    {
        last_state_valid = false;
        LOG_WARNING("[Samsung SSD] LED Security Send failed on %s", path.c_str());
    }
}

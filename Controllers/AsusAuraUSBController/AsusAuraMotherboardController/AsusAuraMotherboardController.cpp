/*---------------------------------------------------------*\
| AsusAuraMotherboardController.cpp                             |
|                                                           |
|   Driver for ASUS Aura motherboard                            |
|                                                           |
|   Mola19                                      08 Mar 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <math.h>
#include "AsusAuraMotherboardController.h"
#include "LogManager.h"
#include "StringUtils.h"

AuraMotherboardController::AuraMotherboardController(hid_device* dev_handle, const char* path, uint16_t pid, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    device_pid  = pid;
    name        = dev_name;
}

AuraMotherboardController::~AuraMotherboardController()
{
    hid_close(dev);
}

std::string AuraMotherboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string AuraMotherboardController::GetNameString()
{
    return(name);
}

std::string AuraMotherboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void AuraMotherboardController::SetGen1()
{
    LOG_TRACE("[AsusAuraMotherboard] SetGen1");

    uint8_t usb_buf[AURA_MOTHERBOARD_REPORT_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, AURA_MOTHERBOARD_REPORT_SIZE);

    /*
    usb_buf[0]   = AURA_MOTHERBOARD_REPORT_ID;

    int bytes = hid_write(dev, usb_buf, AURA_MOTHERBOARD_REPORT_SIZE);

    if(bytes < 0)
    {
        LOG_ERROR("[AsusAuraMotherboard] hid_error: %ls", hid_error(dev));
    }
    */
}

void AuraMotherboardController::SendDirect(unsigned int zone_idx, const std::vector<RGBColor>& colors, unsigned int start_idx, unsigned int leds_count)
{
    LOG_TRACE("[AsusAuraMotherboard] SendDirectToZone %d, start_idx: %d, LEDs: %d\n", zone_idx, start_idx, leds_count);

    /*-----------------------------------------------------*\
    | Create and zero out buffer                            |
    \*-----------------------------------------------------*/
    uint8_t usb_buf[AURA_MOTHERBOARD_REPORT_SIZE];
    memset(usb_buf, 0x00, AURA_MOTHERBOARD_REPORT_SIZE);

    /*-----------------------------------------------------*\
    | Constant part                                         |
    \*-----------------------------------------------------*/
    usb_buf[0]   = AURA_MOTHERBOARD_REPORT_ID;
    usb_buf[1]   = 0x05;

    /*-----------------------------------------------------*\
    | Zone selector                                         |
    \*-----------------------------------------------------*/
    usb_buf[4]   = zone_idx;

    /*-----------------------------------------------------*\
    | Sends leds, by chunks of 15                           |
    \*-----------------------------------------------------*/
    unsigned int total_sent = 0;
    unsigned int offset = 0;

    while(total_sent < leds_count)
    {
        unsigned int chunk_size = std::min<unsigned int>(15, leds_count - offset);

        usb_buf[14] = offset & 0xFF; // LED offset
        usb_buf[16] = chunk_size;    // LED count

        for (unsigned int i = 0; i < chunk_size; i++)
        {
            unsigned int src = offset + i + start_idx;
            unsigned int dst = 18 + (i * 3);

            usb_buf[dst]     = RGBGetRValue(colors[src]);
            usb_buf[dst + 1] = RGBGetGValue(colors[src]);
            usb_buf[dst + 2] = RGBGetBValue(colors[src]);
        }

        total_sent += chunk_size;
        offset = total_sent;

        // debug - TODO : remove
        for (unsigned int i = 0; i < sizeof(usb_buf); i++)
        {
            printf("%02X ", usb_buf[i]);
        }

        printf("\n");
        // --debug

        int bytes = hid_write(dev, usb_buf, AURA_MOTHERBOARD_REPORT_SIZE);

        LOG_TRACE("[AsusAuraMotherboard] SendDirectToZone %d, offset: %d, LEDs: %d, bytes written: %d",
                  zone_idx, offset, chunk_size, bytes);

        if(bytes < 0)
        {
            LOG_ERROR("[AsusAuraMotherboard] hid_error: %ls", hid_error(dev));
        }
    }

}





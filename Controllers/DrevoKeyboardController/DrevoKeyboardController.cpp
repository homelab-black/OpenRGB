/*---------------------------------------------------------*\
| DrevoKeyboardController.cpp                               |
|                                                           |
|   Driver for Drevo keyboards                              |
|                                                           |
|   Dennis Blokland                             02 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "LogManager.h"
#include "DrevoKeyboardController.h"

/*---------------------------------------------------------*\
| Direct mode packet layout                                 |
|                                                           |
|   Byte  0       Report ID                                 |
|   Bytes 1 - 6   Fixed direct mode command header          |
|   Bytes 7 - 10  HID usage code of each key in the packet  |
|   Bytes 12 - 27 RGB triplet per key, 4 byte stride        |
|                                                           |
| Each packet carries up to four keys                       |
\*---------------------------------------------------------*/
#define DREVO_PACKET_SIZE               32
#define DREVO_KEYS_PER_PACKET           4
#define DREVO_KEY_CODE_OFFSET           7
#define DREVO_COLOR_OFFSET              12
#define DREVO_COLOR_STRIDE              4

DrevoKeyboardController::DrevoKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;

    LOG_DEBUG("[%s] Opened device at %s", DREVO_CONTROLLER_NAME, location.c_str());
}

DrevoKeyboardController::~DrevoKeyboardController()
{
    hid_close(dev);
}

std::string DrevoKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string DrevoKeyboardController::GetDeviceName()
{
    return(name);
}

void DrevoKeyboardController::SendDirect(const std::vector<RGBColor>& colors, const std::vector<led>& leds)
{
    unsigned char   buf[DREVO_PACKET_SIZE];
    unsigned int    count       = 0;

    memset(buf, 0x00, DREVO_PACKET_SIZE);

    buf[0x00]   = 0x06;
    buf[0x01]   = 0xBE;
    buf[0x02]   = 0x19;
    buf[0x03]   = 0x00;
    buf[0x04]   = 0x01;
    buf[0x05]   = 0x01;
    buf[0x06]   = 0x0E;

    for(unsigned int led_idx = 0; led_idx < colors.size(); led_idx++)
    {
        unsigned int color_offset = DREVO_COLOR_OFFSET + (count * DREVO_COLOR_STRIDE);

        buf[DREVO_KEY_CODE_OFFSET + count]  = (unsigned char)leds[led_idx].value;
        buf[color_offset + 0]               = RGBGetRValue(colors[led_idx]);
        buf[color_offset + 1]               = RGBGetGValue(colors[led_idx]);
        buf[color_offset + 2]               = RGBGetBValue(colors[led_idx]);

        count++;

        if(count == DREVO_KEYS_PER_PACKET)
        {
            if(hid_write(dev, buf, DREVO_PACKET_SIZE) < 0)
            {
                LOG_ERROR("[%s] Write failed at key %u of %u, aborting update", DREVO_CONTROLLER_NAME, led_idx, (unsigned int)colors.size());
                return;
            }

            count = 0;
        }
    }

    /*---------------------------------------------------------*\
    | The packet always carries four key slots.  Pad a trailing |
    |   partial packet by repeating its last key so the unused  |
    |   slots address a real key with the colour it has just    |
    |   been given, rather than leaving whichever key occupied  |
    |   that slot in the previous packet                        |
    \*---------------------------------------------------------*/
    if(count > 0)
    {
        unsigned int last_offset = DREVO_COLOR_OFFSET + ((count - 1) * DREVO_COLOR_STRIDE);

        for(unsigned int pad_idx = count; pad_idx < DREVO_KEYS_PER_PACKET; pad_idx++)
        {
            unsigned int pad_offset = DREVO_COLOR_OFFSET + (pad_idx * DREVO_COLOR_STRIDE);

            buf[DREVO_KEY_CODE_OFFSET + pad_idx]    = buf[DREVO_KEY_CODE_OFFSET + (count - 1)];
            buf[pad_offset + 0]                     = buf[last_offset + 0];
            buf[pad_offset + 1]                     = buf[last_offset + 1];
            buf[pad_offset + 2]                     = buf[last_offset + 2];
        }

        if(hid_write(dev, buf, DREVO_PACKET_SIZE) < 0)
        {
            LOG_ERROR("[%s] Write failed on the final packet", DREVO_CONTROLLER_NAME);
        }
    }
}

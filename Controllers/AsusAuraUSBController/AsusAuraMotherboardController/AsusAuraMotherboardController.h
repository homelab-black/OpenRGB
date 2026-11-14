/*---------------------------------------------------------*\
| AsusAuraMotherboardController.h                               |
|                                                           |
|   Driver for ASUS Aura motherboard                            |
|                                                           |
|   Mola19                                      08 Mar 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include "RGBControllerInterface.h"

/*
NOTE:
• The Addressable Gen2 header supports addressable RGB LED strips (5V/Data/Ground), with
a maximum power rating of 3A (5V). Additionally, the addressable header on this board can
support up to 120 LEDs per header in Gen1 mode, or a combined maximum of 500 LEDs in
Gen2 mode.
• Actual lighting and color will vary with LED strip.
• If your LED strip does not light up, check if the addressable RGB LED strip is connected in the
correct orientation, and the 5V connector is aligned with the 5V header on the motherboard.
• The addressable RGB LED strip will only light up when the system is powered on.
*/

#define AURA_MOTHERBOARD_REPORT_ID            0x41
#define AURA_MOTHERBOARD_REPORT_SIZE            65
#define AURA_MOTHERBOARD_HEADERS_COUNT           3
#define AURA_MOTHERBOARD_MAX_LED_PER_HEADERS   120
#define AURA_MOTHERBOARD_LOGO_LEDS_COUNT         1

class AuraMotherboardController
{
public:
    AuraMotherboardController(hid_device* dev_handle, const char* path, uint16_t pid, std::string dev_name);
    virtual ~AuraMotherboardController();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();

    void SetGen1();
    void SendDirect(unsigned int zone_idx, const std::vector<RGBColor>& colors, unsigned int start_idx, unsigned int leds_count);

    uint16_t    device_pid;

private:
    hid_device* dev;
    std::string location;
    std::string name;
};

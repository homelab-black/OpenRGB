/*---------------------------------------------------------*\
| QMKOpenRGBRevDController.h                                |
|                                                           |
|   Driver for OpenRGB QMK Keyboard Protocol Revision F     |
|                                                           |
|   reklrekl                                    06 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "QMKOpenRGBBaseController.h"

class QMKOpenRGBRevFController : public QMKOpenRGBBaseController
{
public:
    QMKOpenRGBRevFController(hid_device *dev_handle, const char *path, const wchar_t *product_string, const wchar_t *manufacturer_string);
    ~QMKOpenRGBRevFController();

    //Virtual function implementations
    void            GetLEDInfo(unsigned int leds_count);
    void            DirectModeSetSingleLED(unsigned int led, unsigned char red, unsigned char green, unsigned char blue);
    void            DirectModeSetLEDs(std::vector<RGBColor> colors, unsigned int num_colors);

    //Protocol Specific functions
    std::vector<uint8_t>        GetEnabledModes();
    std::vector<unsigned int>   GetLEDValues();
    unsigned int                GetFeatures();
    unsigned int                GetKeyLEDCount();
    unsigned int                GetUnderglowLEDCount();
    std::string                 GetEffectName(unsigned int index);

private:
    void                        GetDeviceInfoEX();

    std::vector<uint8_t>        GetEnabledModesPage(uint8_t page);
    void                        FetchLEDColors();
    std::vector<unsigned int>   led_values;
    std::vector<unsigned int>   qmk_to_openrgb;

    unsigned int    features;

    unsigned int    key_led_count;
    unsigned int    underglow_led_count;

    unsigned int    mode_count;
    unsigned int    packet_size;
    unsigned int    modes_per_page;
    unsigned int    leds_per_info_page;
    unsigned int    leds_per_color_page;
    unsigned int    leds_per_stream_packet;
};

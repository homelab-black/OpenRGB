/*---------------------------------------------------------*\
| QMKOpenRGBRevDController.cpp                              |
|                                                           |
|   Driver for OpenRGB QMK Keyboard Protocol Revision F     |
|                                                           |
|   reklrekl                                    06 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "QMKKeycodes.h"
#include "QMKOpenRGBRevFController.h"

using namespace std::chrono_literals;

QMKOpenRGBRevFController::QMKOpenRGBRevFController(hid_device *dev_handle, const char *path, const wchar_t *product_string, const wchar_t *manufacturer_string) :
    QMKOpenRGBBaseController(dev_handle, path, 9)
{
    /*-----------------------------------------------------*\
    | Use HIDAPI strings instead of firmware strings          |
    \*-----------------------------------------------------*/
    device_name.clear();
    device_vendor.clear();

    if(product_string != nullptr)
    {
        for(int i = 0; product_string[i] != 0; i++)
        {
            device_name.push_back((char)product_string[i]);
        }
    }

    if(manufacturer_string != nullptr)
    {
        for(int i = 0; manufacturer_string[i] != 0; i++)
        {
            device_vendor.push_back((char)manufacturer_string[i]);
        }
    }

    /*-----------------------------------------------------*\
    | Fetch device info + features via EX command             |
    \*-----------------------------------------------------*/
    key_led_count        = 0;
    underglow_led_count  = 0;

    GetDeviceInfoEX();
    GetModeInfo();
}

QMKOpenRGBRevFController::~QMKOpenRGBRevFController()
{
}

std::vector<unsigned int>  QMKOpenRGBRevFController::GetLEDValues()
{
    return led_values;
}

unsigned int QMKOpenRGBRevFController::GetKeyLEDCount()
{
    return key_led_count;
}

unsigned int QMKOpenRGBRevFController::GetUnderglowLEDCount()
{
    return underglow_led_count;
}

void QMKOpenRGBRevFController::GetDeviceInfoEX()
{
    unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

    memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

    usb_buf[0x00] = 0x00;
    usb_buf[0x01] = QMK_OPENRGB_GET_DEVICE_INFO;

    int bytes_read = 0;
    do
    {
        hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
        bytes_read = hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
        } while(bytes_read <= 0);

        unsigned int exponent = usb_buf[3];

        if(exponent > 6)
        {
            LOG_WARNING("[%s] Packet size config %u too large, falling back to 64", device_name.c_str(), exponent);
            packet_size = 64;
        }
        else if(exponent < 5)
        {
            LOG_WARNING("[%s] Packet size config %u too small, falling back to 32", device_name.c_str(), exponent);
            packet_size = 32;
        }
        else
        {
            packet_size = 1 << exponent;
        }

        if(usb_buf[packet_size - 1] == QMK_OPENRGB_FAILURE)
        {
            LOG_WARNING("[%s] GetDeviceInfoEX: firmware reported failure", device_name.c_str());
            return;
        }

        total_number_of_leds  = usb_buf[1];
        mode_count            = usb_buf[4];
        features              = usb_buf[5] | (usb_buf[6] << 8) | (usb_buf[7] << 16) | (usb_buf[8] << 24);

        modes_per_page          = packet_size - 1;                          // CMD byte
        leds_per_info_page      = (packet_size - 1) / 5;                   // 5b per LED (x,y,flags,kc_lo,kc_hi)
        leds_per_color_page     = (packet_size - 1) / 3;                   // 3b per LED (r,g,b)
        leds_per_stream_packet  = (packet_size - 3) / 3;                   // CMD + index + count, 3b per LED

        if(leds_per_info_page == 0)      leds_per_info_page = 1;
        if(leds_per_color_page == 0)     leds_per_color_page = 1;
        if(leds_per_stream_packet == 0)  leds_per_stream_packet = 1;
}

void QMKOpenRGBRevFController::GetLEDInfo(unsigned int leds_count)
{
    unsigned int leds_sent               = 0;
    unsigned int leds_per_update_info    = leds_per_info_page;

    std::vector<point_t>        underglow_points;
    std::vector<unsigned int>   underglow_flags;
    std::vector<std::string>    underglow_names;
    std::vector<RGBColor>       underglow_colors;
    std::vector<unsigned int>   underglow_values;

    while (leds_sent < leds_count)
    {
        if ((leds_count - leds_sent) < leds_per_update_info)
        {
            leds_per_update_info = leds_count - leds_sent;
        }

        unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

        /*-----------------------------------------------------*\
        | Zero out buffer                                       |
        \*-----------------------------------------------------*/
        memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

        /*-----------------------------------------------------*\
        | Set up config table request packet                    |
        \*-----------------------------------------------------*/
        usb_buf[0x00] = 0x00;
        usb_buf[0x01] = QMK_OPENRGB_GET_LED_INFO;
        usb_buf[0x02] = leds_sent;
        usb_buf[0x03] = leds_per_update_info;

        int bytes_read = 0;
        do
        {
            hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
            bytes_read = hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
        } while(bytes_read <= 0);

        for (unsigned int led_idx = 0; led_idx < leds_per_update_info; led_idx++)
        {
            if(usb_buf[packet_size - 1] != QMK_OPENRGB_FAILURE)
            {
                unsigned int offset = led_idx * 5;

                unsigned int x        = usb_buf[offset + 1];
                unsigned int y        = usb_buf[offset + 2];
                unsigned int flag     = usb_buf[offset + 3];
                uint16_t keycode;
                keycode = usb_buf[offset + 4] | (usb_buf[offset + 5] << 8);

                if(flag == 0)
                {
                    LOG_WARNING("[%s] Unknown LED found at index %u.", device_name.c_str(), led_idx);
                    continue;
                }
                if(flag &8)
                {
                    LOG_WARNING("[%s] Indicator LED found at index %u.", device_name.c_str(), led_idx);
                }

                unsigned int qmk_index = leds_sent + led_idx;

                if(flag & 2)
                {
                    underglow_points.push_back(point_t{(uint8_t)x, (uint8_t)y});
                    underglow_flags.push_back(flag);
                    underglow_colors.push_back(0);
                    underglow_values.push_back(qmk_index);
                    underglow_led_count++;
                }
                else if(flag & 4)
                {
                    led_points.push_back(point_t{(uint8_t)x, (uint8_t)y});
                    led_flags.push_back(flag);
                    led_colors.push_back(0);
                    led_values.push_back(qmk_index);
                    key_led_count++;
                }

                if(keycode != 0)
                {
                    // hack to remap the FN key
                    // this range covers every momentary Layer key in QMK
                    // OpenRGB won't differitate between different layers here,
                    // everything gets remapped to "Right FN"
                    if (keycode >= 0x5220 && keycode < 0x5240)  // QMK "FN" button range
                    {
                        keycode = QMK_KC_TRANSPARENT;
                    }
                    if(qmk_keynames.count(keycode) > 0)
                    {
                        if(flag & 2)
                        {
                            underglow_names.push_back(qmk_keynames[keycode]);
                        }
                        else if(flag & 4)
                        {
                            led_names.push_back(qmk_keynames[keycode]);
                        }
                    }
                    else
                    {
                        LOG_DEBUG("[%s] Key code: %d (%04X) @ LED %d was not found in the QMK keyname map",
                                  device_name.c_str(), keycode, keycode, leds_sent + led_idx);

                        if(flag & 2)
                        {
                            underglow_names.push_back(KEY_EN_UNUSED);
                        }
                        else if(flag & 4)
                        {
                            led_names.push_back(KEY_EN_UNUSED);
                        }
                    }
                }
                else if(flag & 2)
                {
                    underglow_names.push_back("Underglow: " + std::to_string(underglow_names.size() + led_names.size()));
                }
            }
        }
        if(usb_buf[packet_size - 1] == QMK_OPENRGB_FAILURE)
        {
            break;
        }
        leds_sent += leds_per_update_info;
    }

    led_points.insert(led_points.end(), underglow_points.begin(), underglow_points.end());
    led_flags.insert(led_flags.end(), underglow_flags.begin(), underglow_flags.end());
    led_colors.insert(led_colors.end(), underglow_colors.begin(), underglow_colors.end());
    led_names.insert(led_names.end(), underglow_names.begin(), underglow_names.end());
    led_values.insert(led_values.end(), underglow_values.begin(), underglow_values.end());

    if(led_values.size() != total_number_of_leds)
    {
        LOG_WARNING("[%s] LED count mismatch: firmware announced %u, received %u",
                    device_name.c_str(), total_number_of_leds, (unsigned int)led_values.size());
        total_number_of_leds = (unsigned int)led_values.size();
    }


    /*-----------------------------------------------------*\
    | Build inverse mapping: QMK index → OpenRGB index       |
    | Size to leds_count (firmware's LED count) not          |
    | led_values.size(), as QMK indices can exceed           |
    | led_values.size() when phantom LEDs are present.       |
    \*-----------------------------------------------------*/
    qmk_to_openrgb.resize(leds_count);
    for (unsigned int i = 0; i < led_values.size(); i++)
    {
        if(led_values[i] >= leds_count)
        {
            LOG_WARNING("[%s] QMK index %u exceeds firmware LED count %u at slot %u",
                        device_name.c_str(), led_values[i], leds_count, i);
        }
        qmk_to_openrgb[led_values[i]] = i;
    }

    /*---------------------------------------------------------*\
    | Initialize colors from device values (buffered only)      |
    \*---------------------------------------------------------*/
    if(features & QMK_OPENRGB_FEATURE_READ)
    {
        FetchLEDColors();
    }
}

void QMKOpenRGBRevFController::FetchLEDColors()
{
    if(!(features & QMK_OPENRGB_FEATURE_READ))
    {
        return;
    }

    unsigned int leds_sent               = 0;
    unsigned int leds_per_update_color   = leds_per_color_page;
    unsigned int leds_count              = total_number_of_leds;

    while (leds_sent < leds_count)
    {
        if ((leds_count - leds_sent) < leds_per_update_color)
        {
            leds_per_update_color = leds_count - leds_sent;
        }

        unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

        /*-----------------------------------------------------*\
        | Zero out buffer                                       |
        \*-----------------------------------------------------*/
        memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

        /*-----------------------------------------------------*\
        | Set up config table request packet                    |
        \*-----------------------------------------------------*/
        usb_buf[0x00] = 0x00;
        usb_buf[0x01] = QMK_OPENRGB_GET_LED_COLOR;
        usb_buf[0x02] = leds_sent;
        usb_buf[0x03] = leds_per_update_color;

        int bytes_read = 0;
        do
        {
            hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
            bytes_read = hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
        } while(bytes_read <= 0);

        for (unsigned int led_idx = 0; led_idx < leds_per_update_color; led_idx++)
        {
            unsigned int offset = led_idx * 3;

            if(usb_buf[packet_size - 1] != QMK_OPENRGB_FAILURE)
            {
                unsigned int r = usb_buf[offset + 1];
                unsigned int g = usb_buf[offset + 2];
                unsigned int b = usb_buf[offset + 3];

                led_colors[leds_sent + led_idx] = ToRGBColor(r, g, b);
            }
        }

        if(usb_buf[packet_size - 1] == QMK_OPENRGB_FAILURE)
        {
            break;
        }

        leds_sent += leds_per_update_color;
    }
}

std::vector<uint8_t> QMKOpenRGBRevFController::GetEnabledModes()
{
    std::vector<uint8_t> all_ids;
    unsigned int pages = (mode_count + modes_per_page - 1) / modes_per_page;

    for (unsigned int page = 0; page < pages; page++)
    {
        std::vector<uint8_t> page_ids = GetEnabledModesPage(page);
        all_ids.insert(all_ids.end(), page_ids.begin(), page_ids.end());
    }
    return all_ids;
}

std::vector<uint8_t> QMKOpenRGBRevFController::GetEnabledModesPage(uint8_t page)
{
    unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

    /*-----------------------------------------------------*\
    | Set up config table request packet                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00] = 0x00;
    usb_buf[0x01] = QMK_OPENRGB_GET_ENABLED_MODES;
    usb_buf[0x02] = page;

    int bytes_read = 0;
    do
    {
        hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
        bytes_read = hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
    } while(bytes_read <= 0);

    std::vector<uint8_t> mode_ids;

    if(usb_buf[packet_size - 1] == QMK_OPENRGB_FAILURE)
    {
        LOG_WARNING("[%s] GetEnabledModesPage: firmware reported failure on page %u", device_name.c_str(), page);
        return mode_ids;
    }

    /*-----------------------------------------------------*\
    | Calculate exact count for this page                    |
    \*-----------------------------------------------------*/
    unsigned int start = page * modes_per_page;
    unsigned int remaining = (start < mode_count) ? mode_count - start : 0;
    unsigned int count = (remaining < modes_per_page) ? remaining : modes_per_page;


    for (unsigned int i = 0; i < count; i++)
    {
        mode_ids.push_back(usb_buf[1 + i]);
    }
    return mode_ids;
}

unsigned int QMKOpenRGBRevFController::GetFeatures()
{
    return features;
}

std::string QMKOpenRGBRevFController::GetEffectName(unsigned int index)
{
    unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

    /*-----------------------------------------------------*\
    | Set up config table request packet                    |
    \*-----------------------------------------------------*/
    usb_buf[0x00] = 0x00;
    usb_buf[0x01] = QMK_OPENRGB_GET_EFFECT_NAME;
    usb_buf[0x02] = index;

    int bytes_read = 0;
    do
    {
        hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
        bytes_read = hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
    } while(bytes_read <= 0);

    if(usb_buf[packet_size - 1] == QMK_OPENRGB_FAILURE)
    {
        LOG_WARNING("[%s] GetEffectName: firmware reported failure for index %u", device_name.c_str(), index);
        return "";
    }

    std::string effect_name;
    int i = 1;
    while (i < QMK_OPENRGB_PACKET_SIZE && usb_buf[i] != 0)
    {
        effect_name.push_back(usb_buf[i]);
        i++;
    }
    return effect_name;
}

void QMKOpenRGBRevFController::DirectModeSetSingleLED(unsigned int led, unsigned char red, unsigned char green, unsigned char blue)
{
    if(led >= total_number_of_leds)
    {
        return;
    }

    unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

    /*-----------------------------------------------------*\
    | Set up config table request packet                    |
    \*-----------------------------------------------------*/

    usb_buf[0x00] = 0x00;
    usb_buf[0x01] = QMK_OPENRGB_DIRECT_MODE_SET_SINGLE_LED;
    usb_buf[0x02] = led_values[led];
    usb_buf[0x03] = red;
    usb_buf[0x04] = green;
    usb_buf[0x05] = blue;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE, QMK_OPENRGB_HID_READ_TIMEOUT);
}

void QMKOpenRGBRevFController::DirectModeSetLEDs(std::vector<RGBColor> colors, unsigned int /*leds_count*/)
{
    unsigned int slot        = 0;
    unsigned int total_slots = colors.size();

    while (slot < total_slots)
    {
        /*-----------------------------------------------------*\
        | Start batch at current slot's QMK index                |
        \*-----------------------------------------------------*/
        unsigned int qmk_start = led_values[slot];
        unsigned int batch     = 1;

        /*-----------------------------------------------------*\
        | Extend batch while QMK indices are consecutive and     |
        | we haven't hit the page limit                          |
        \*-----------------------------------------------------*/
        while (batch < leds_per_stream_packet
               && slot + batch < total_slots
               && led_values[slot + batch] == led_values[slot + batch - 1] + 1)
        {
            batch++;
        }

        /*-----------------------------------------------------*\
        | Build and send packet with correct QMK start           |
        \*-----------------------------------------------------*/
        unsigned char usb_buf[QMK_OPENRGB_PACKET_SIZE];
        memset(usb_buf, 0x00, QMK_OPENRGB_PACKET_SIZE);

        usb_buf[0x00] = 0x00;
        usb_buf[0x01] = QMK_OPENRGB_DIRECT_MODE_SET_LEDS;
        usb_buf[0x02] = qmk_start;
        usb_buf[0x03] = batch;

        for (unsigned int i = 0; i < batch; i++)
        {
            usb_buf[(i * 3) + 4] = RGBGetRValue(colors[slot + i]);
            usb_buf[(i * 3) + 5] = RGBGetGValue(colors[slot + i]);
            usb_buf[(i * 3) + 6] = RGBGetBValue(colors[slot + i]);
        }

        hid_write(dev, usb_buf, QMK_OPENRGB_PACKET_SIZE);

        if(delay > 0ms)
        {
            std::this_thread::sleep_for(delay);
        }

        slot += batch;
    }
}

/*---------------------------------------------------------*\
| IlovbeeB87Controller.cpp                                 |
|                                                           |
|   Driver for ilovbee B87 keyboard lighting                |
|                                                           |
|   w568w                                       18 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "IlovbeeB87Controller.h"
#include "LogManager.h"
#include "StringUtils.h"

IlovbeeB87Controller::IlovbeeB87Controller(hid_device* dev_handle, const hid_device_info& info, std::string dev_name)
{
    dev         = dev_handle;
    location    = info.path;
    name        = dev_name;
}

IlovbeeB87Controller::~IlovbeeB87Controller()
{
    hid_close(dev);
}

bool IlovbeeB87Controller::Initialize()
{
    return(ReadLightingState());
}

std::string IlovbeeB87Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string IlovbeeB87Controller::GetDeviceName()
{
    return(name);
}

std::string IlovbeeB87Controller::GetSerialString()
{
    wchar_t serial_string[128];
    if(hid_get_serial_number_string(dev, serial_string, 128) != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

unsigned char IlovbeeB87Controller::GetCurrentMode()
{
    return(led_config[ILOVBEE_B87_CURRENT_MODE_INDEX]);
}

unsigned char IlovbeeB87Controller::GetModeBrightness(unsigned char mode_value)
{
    unsigned int offset = ILOVBEE_B87_MODE_CONFIG_BASE + (mode_value * 2);

    if(offset >= ILOVBEE_B87_LED_CONFIG_SIZE)
    {
        return(0);
    }

    return(led_config[offset]);
}

unsigned char IlovbeeB87Controller::GetModeSpeed(unsigned char mode_value)
{
    unsigned int offset = ILOVBEE_B87_MODE_CONFIG_BASE + (mode_value * 2) + 1;

    if(offset >= ILOVBEE_B87_LED_CONFIG_SIZE)
    {
        return(0);
    }

    return((led_config[offset] >> 4) & 0x0F);
}

RGBColor IlovbeeB87Controller::GetModeColor(unsigned char mode_value)
{
    unsigned int offset = mode_value * ILOVBEE_B87_RGB_COLORS_PER_MODE * 3;

    if((offset + 2) >= ILOVBEE_B87_RGB_TABLE_SIZE)
    {
        return(0);
    }

    return(ToRGBColor(rgb_table[offset], rgb_table[offset + 1], rgb_table[offset + 2]));
}

bool IlovbeeB87Controller::ReadG5Data(unsigned char command, unsigned char* payload, unsigned int payload_size)
{
    if(!SendG5Command(command, nullptr, payload_size))
    {
        return(false);
    }

    unsigned char report[ILOVBEE_B87_REPORT_SIZE] = { 0 };
    report[0] = ILOVBEE_B87_REPORT_ID;

    int result = hid_get_feature_report(dev, report, sizeof(report));

    if(result < (int)(ILOVBEE_B87_HEADER_SIZE + payload_size))
    {
        LOG_ERROR("[%s] Short response to G5 command 0x%02X: received %d bytes", name.c_str(), command, result);
        return(false);
    }

    if((report[0] != ILOVBEE_B87_REPORT_ID) || (report[1] != command))
    {
        LOG_ERROR("[%s] Unexpected G5 response to command 0x%02X: report 0x%02X command 0x%02X", name.c_str(), command, report[0], report[1]);
        return(false);
    }

    memcpy(payload, &report[ILOVBEE_B87_HEADER_SIZE], payload_size);
    return(true);
}

bool IlovbeeB87Controller::SendG5Command(unsigned char command, const unsigned char* payload, unsigned int payload_size)
{
    unsigned char report[ILOVBEE_B87_REPORT_SIZE] = { 0 };

    report[0] = ILOVBEE_B87_REPORT_ID;
    report[1] = command;
    report[4] = 0x01;
    report[6] = (unsigned char)(payload_size & 0xFF);
    report[7] = (unsigned char)(payload_size >> 8);

    if(payload != nullptr)
    {
        memcpy(&report[ILOVBEE_B87_HEADER_SIZE], payload, payload_size);
    }

    if(hid_send_feature_report(dev, report, sizeof(report)) != (int)sizeof(report))
    {
        LOG_ERROR("[%s] Failed to send G5 command 0x%02X: %ls", name.c_str(), command, hid_error(dev));
        return(false);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    return(true);
}

bool IlovbeeB87Controller::ReadLightingState()
{
    unsigned char new_led_config[ILOVBEE_B87_LED_CONFIG_SIZE];
    unsigned char new_rgb_table[ILOVBEE_B87_RGB_TABLE_SIZE];

    if(!ReadG5Data(ILOVBEE_B87_READ_LED_CONFIG_COMMAND, new_led_config, sizeof(new_led_config)))
    {
        return(false);
    }

    if((new_led_config[ILOVBEE_B87_CONFIG_SIGNATURE_INDEX] != 0x5A)
    || (new_led_config[ILOVBEE_B87_CONFIG_SIGNATURE_INDEX + 1] != 0xA5))
    {
        LOG_ERROR("[%s] Invalid B87 lighting configuration signature", name.c_str());
        return(false);
    }

    if(!ReadG5Data(ILOVBEE_B87_READ_RGB_TABLE_COMMAND, new_rgb_table, sizeof(new_rgb_table)))
    {
        return(false);
    }

    memcpy(led_config, new_led_config, sizeof(led_config));
    memcpy(rgb_table, new_rgb_table, sizeof(rgb_table));
    return(true);
}

bool IlovbeeB87Controller::SetMode(unsigned char mode_value, unsigned int brightness, bool update_speed, unsigned int speed, bool update_color, RGBColor color)
{
    if((mode_value > ILOVBEE_B87_MODE_BLOSSOMING)
    || (mode_value == ILOVBEE_B87_MODE_HIDDEN_SCAN)
    || (brightness > ILOVBEE_B87_BRIGHTNESS_MAX)
    || (update_speed && ((speed < ILOVBEE_B87_SPEED_MIN) || (speed > ILOVBEE_B87_SPEED_MAX))))
    {
        LOG_ERROR("[%s] Refusing invalid lighting mode parameters", name.c_str());
        return(false);
    }

    if(!ReadLightingState())
    {
        LOG_ERROR("[%s] Refusing to write without a valid current lighting configuration", name.c_str());
        return(false);
    }

    led_config[ILOVBEE_B87_CUSTOM_MODE_INDEX]  = 0x00;
    led_config[ILOVBEE_B87_CURRENT_MODE_INDEX] = mode_value;

    if(mode_value != ILOVBEE_B87_MODE_OFF)
    {
        unsigned int mode_offset = ILOVBEE_B87_MODE_CONFIG_BASE + (mode_value * 2);
        led_config[mode_offset] = (unsigned char)brightness;

        if(update_speed)
        {
            led_config[mode_offset + 1] = (unsigned char)((led_config[mode_offset + 1] & 0x0F) | (speed << 4));
        }
    }

    if(update_color)
    {
        unsigned int color_offset = mode_value * ILOVBEE_B87_RGB_COLORS_PER_MODE * 3;

        for(unsigned int color_idx = 0; color_idx < ILOVBEE_B87_RGB_COLORS_PER_MODE; color_idx++)
        {
            rgb_table[color_offset + (color_idx * 3)]     = RGBGetRValue(color);
            rgb_table[color_offset + (color_idx * 3) + 1] = RGBGetGValue(color);
            rgb_table[color_offset + (color_idx * 3) + 2] = RGBGetBValue(color);
        }
    }

    unsigned char expected_led_config[ILOVBEE_B87_LED_CONFIG_SIZE];
    unsigned char expected_rgb_table[ILOVBEE_B87_RGB_TABLE_SIZE];
    memcpy(expected_led_config, led_config, sizeof(expected_led_config));

    if(update_color)
    {
        memcpy(expected_rgb_table, rgb_table, sizeof(expected_rgb_table));
    }

    if(!SendG5Command(ILOVBEE_B87_WRITE_LED_CONFIG_COMMAND, led_config, sizeof(led_config))
    || (update_color && !SendG5Command(ILOVBEE_B87_WRITE_RGB_TABLE_COMMAND, rgb_table, sizeof(rgb_table))))
    {
        return(false);
    }

    return(VerifyLightingState(expected_led_config, update_color ? expected_rgb_table : nullptr));
}

bool IlovbeeB87Controller::VerifyLightingState(const unsigned char* expected_led_config, const unsigned char* expected_rgb_table)
{
    if(!ReadLightingState())
    {
        return(false);
    }

    if(memcmp(led_config, expected_led_config, sizeof(led_config)) != 0)
    {
        LOG_ERROR("[%s] B87 lighting configuration did not verify after write", name.c_str());
        return(false);
    }

    if((expected_rgb_table != nullptr) && (memcmp(rgb_table, expected_rgb_table, sizeof(rgb_table)) != 0))
    {
        LOG_ERROR("[%s] B87 RGB table did not verify after write", name.c_str());
        return(false);
    }

    return(true);
}

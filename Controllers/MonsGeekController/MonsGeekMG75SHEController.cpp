/*---------------------------------------------------------*\
| MonsGeekMG75SHEController.cpp                             |
|                                                           |
|   Driver for the MonsGeek MG75S HE keyboard               |
|                                                           |
|   Sergey Tsai (mustang651)                 14 Sep 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>
#include "LogManager.h"
#include "MonsGeekMG75SHEController.h"
#include "StringUtils.h"

#define MONSGEEK_MG75S_HE_STATIC_PACKET_DELAY_MS        30
#define MONSGEEK_MG75S_HE_STATIC_COMMIT_DELAY_MS        1000
#define MONSGEEK_MG75S_HE_MODE_COMMIT_DELAY_MS          1000
#define MONSGEEK_MG75S_HE_SIMPLE_COMMAND_DELAY_MS       20

MonsGeekMG75SHEController::MonsGeekMG75SHEController(hid_device* device, const char* path)
{
    this->device = device;
    location     = path;
}

MonsGeekMG75SHEController::~MonsGeekMG75SHEController()
{
    hid_close(device);
}

void MonsGeekMG75SHEController::ApplyChecksum(unsigned char* payload, size_t checksum_index)
{
    unsigned int checksum = 0;

    for(size_t index = 0; index < checksum_index; index++)
    {
        checksum += payload[index];
    }

    payload[checksum_index] = 0xFF - (checksum & 0xFF);
}

std::string MonsGeekMG75SHEController::GetDeviceName()
{
    return("MonsGeek MG75S HE");
}

std::string MonsGeekMG75SHEController::GetSerial()
{
    wchar_t serial_string[128] = { 0 };

    if(hid_get_serial_number_string(device, serial_string, 128) != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string MonsGeekMG75SHEController::GetLocation()
{
    return("HID: " + location);
}

bool MonsGeekMG75SHEController::SendFeatureReport(const unsigned char* payload)
{
    unsigned char report[MONSGEEK_MG75S_HE_REPORT_SIZE] = { 0 };

    std::memcpy(&report[1], payload, MONSGEEK_MG75S_HE_PACKET_SIZE);

    int result = hid_send_feature_report(device, report, sizeof(report));

    if(result < 0)
    {
        LOG_ERROR("[MonsGeek MG75S HE] Failed to send feature report: %ls", hid_error(device));
        return(false);
    }

    return(true);
}

bool MonsGeekMG75SHEController::SetMode
    (
    unsigned char   mode,
    unsigned char   speed,
    unsigned char   brightness,
    unsigned char   option,
    RGBColor        color
    )
{
    unsigned char payload[MONSGEEK_MG75S_HE_PACKET_SIZE] = { 0 };

    payload[0] = 0x07;
    payload[1] = mode;
    payload[2] = MONSGEEK_MG75S_HE_SPEED_MAX - std::min(speed, (unsigned char)MONSGEEK_MG75S_HE_SPEED_MAX);
    payload[3] = std::min(brightness, (unsigned char)MONSGEEK_MG75S_HE_BRIGHTNESS_MAX);
    payload[4] = option;

    if(mode == MONSGEEK_MG75S_HE_MODE_USER_PICTURE)
    {
        payload[5] = 0;
        payload[6] = 200;
        payload[7] = 200;
    }
    else
    {
        payload[5] = RGBGetRValue(color);
        payload[6] = RGBGetGValue(color);
        payload[7] = RGBGetBValue(color);

        /*-------------------------------------------------*\
        | The firmware reserves pure white and uses this    |
        | near-white value for user-selected white.         |
        \*-------------------------------------------------*/
        if(payload[5] == 0xFF && payload[6] == 0xFF && payload[7] == 0xFF)
        {
            payload[5] = 0xFA;
            payload[6] = 0xFD;
            payload[7] = 0xFA;
        }
    }

    ApplyChecksum(payload, 8);

    if(!SendFeatureReport(payload))
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Match the OEM settling interval before sending |
    | picture data or another lighting command.             |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(std::chrono::milliseconds(MONSGEEK_MG75S_HE_MODE_COMMIT_DELAY_MS));
    return(true);
}

bool MonsGeekMG75SHEController::SetStatic
    (
    const RGBColor*         colors,
    const unsigned char*    hardware_indices,
    size_t                  count
    )
{
    unsigned char matrix_data[MONSGEEK_MG75S_HE_MATRIX_DATA_SIZE] = { 0 };
    size_t led_count = std::min(count, (size_t)MONSGEEK_MG75S_HE_LED_COUNT);

    for(size_t led_index = 0; led_index < led_count; led_index++)
    {
        size_t matrix_offset = 3 * hardware_indices[led_index];

        matrix_data[matrix_offset]     = RGBGetRValue(colors[led_index]);
        matrix_data[matrix_offset + 1] = RGBGetGValue(colors[led_index]);
        matrix_data[matrix_offset + 2] = RGBGetBValue(colors[led_index]);
    }

    for(unsigned char packet_index = 0; packet_index < 7; packet_index++)
    {
        unsigned char payload[MONSGEEK_MG75S_HE_PACKET_SIZE] = { 0 };
        size_t source_offset = packet_index * 56;
        size_t bytes_left    = sizeof(matrix_data) - source_offset;
        size_t bytes_to_copy = std::min(bytes_left, (size_t)56);

        payload[0] = 0x0C;
        payload[1] = 0;
        payload[2] = 0x80;
        payload[3] = 0x01;
        payload[4] = packet_index;

        std::memcpy(&payload[8], &matrix_data[source_offset], bytes_to_copy);
        ApplyChecksum(payload, 7);

        /*-------------------------------------------------*\
        | A picture upload is one stateful seven-packet     |
        | transaction. Keep its packet cadence close to the |
        | OEM driver so it completes within the device's    |
        | transaction window.                               |
        \*-------------------------------------------------*/
        std::this_thread::sleep_for(std::chrono::milliseconds(MONSGEEK_MG75S_HE_STATIC_PACKET_DELAY_MS));

        if(!SendFeatureReport(payload))
        {
            return(false);
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(MONSGEEK_MG75S_HE_STATIC_COMMIT_DELAY_MS));
    return(true);
}

bool MonsGeekMG75SHEController::SetStaticLED(unsigned char hardware_index, RGBColor color)
{
    unsigned char payload[MONSGEEK_MG75S_HE_PACKET_SIZE] = { 0 };

    payload[0]  = 0x14;
    payload[1]  = 0;
    payload[2]  = hardware_index;
    payload[8]  = RGBGetRValue(color);
    payload[9]  = RGBGetGValue(color);
    payload[10] = RGBGetBValue(color);

    ApplyChecksum(payload, 7);
    std::this_thread::sleep_for(std::chrono::milliseconds(MONSGEEK_MG75S_HE_SIMPLE_COMMAND_DELAY_MS));

    if(!SendFeatureReport(payload))
    {
        return(false);
    }

    return(true);
}

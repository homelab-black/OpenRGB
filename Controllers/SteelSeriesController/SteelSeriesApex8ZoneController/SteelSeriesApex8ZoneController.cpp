/*---------------------------------------------------------*\
| SteelSeriesApex8ZoneController.cpp                        |
|                                                           |
|   Driver for SteelSeries Apex 8 Zone                      |
|                                                           |
|   Chris M (Dr_No)                             23 Feb 2022 |
|   Paul K. Gerke                               27 Oct 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "SteelSeriesApex8ZoneController.h"
#include "LogManager.h"

SteelSeriesApex8ZoneController::SteelSeriesApex8ZoneController(hid_device* dev_handle, const char* path, std::string dev_name) : SteelSeriesApex3Controller(dev_handle, path, dev_name), gamesense(GameSenseLost, this)
{
    direct_brightness = STEELSERIES_8Z_BRIGHTNESS_MAX;
}

SteelSeriesApex8ZoneController::~SteelSeriesApex8ZoneController()
{

}

uint8_t SteelSeriesApex8ZoneController::GetLedCount()
{
    return STEELSERIES_8Z_LED_COUNT;
}

uint8_t SteelSeriesApex8ZoneController::GetMaxBrightness()
{
    return STEELSERIES_8Z_BRIGHTNESS_MAX;
}

bool SteelSeriesApex8ZoneController::SupportsRainbowWave()
{
    return true;
}

bool SteelSeriesApex8ZoneController::SupportsSave()
{
    return false;
}

void SteelSeriesApex8ZoneController::Save()
{
    /*---------------------------------------------------------*\
    | This device does not yet support saving                   |
    \*---------------------------------------------------------*/
}

void SteelSeriesApex8ZoneController::SetBrightness(uint8_t brightness)
{
    uint8_t buffer[STEELSERIES_8Z_WRITE_PACKET_SIZE]    = { 0x00, 0x23, brightness };

    hid_write(dev, buffer, STEELSERIES_8Z_WRITE_PACKET_SIZE);

    current_brightness = brightness;
}

uint8_t SteelSeriesApex8ZoneController::GetBrightness()
{
    uint8_t buffer[STEELSERIES_8Z_WRITE_PACKET_SIZE]    = { 0x00, 0xA3 };

    hid_write(dev, buffer, STEELSERIES_8Z_WRITE_PACKET_SIZE);

    int result = hid_read_timeout(dev, buffer, STEELSERIES_8Z_WRITE_PACKET_SIZE, STEELSERIES_APEX3_HID_TIMEOUT);
    if (result > 1 && buffer[0x00] == 0xA3)
    {
        return(buffer[0x01]);
    }

    return(STEELSERIES_8Z_BRIGHTNESS_MAX);
}

void SteelSeriesApex8ZoneController::SetColor(std::vector<RGBColor> colors, uint8_t mode, uint8_t brightness)
{
    /*-----------------------------------------------------*\
    | Direct mode (0): if SteelSeries Engine is running it  |
    | overwrites HID writes, so route through GameSense.    |
    | Engine ignores the hardware brightness command, so    |
    | brightness is applied to the colors instead.          |
    \*-----------------------------------------------------*/
    if(mode == 0)
    {
        {
            std::lock_guard<std::mutex> lock(write_mutex);

            direct_colors       = colors;
            direct_brightness   = brightness;
        }

        std::vector<RGBColor> scaled(colors.size());
        unsigned int          level = (brightness > STEELSERIES_8Z_BRIGHTNESS_MAX) ? STEELSERIES_8Z_BRIGHTNESS_MAX : brightness;

        for(unsigned int i = 0; i < colors.size(); i++)
        {
            scaled[i] = ToRGBColor(RGBGetRValue(colors[i]) * level / STEELSERIES_8Z_BRIGHTNESS_MAX,
                                   RGBGetGValue(colors[i]) * level / STEELSERIES_8Z_BRIGHTNESS_MAX,
                                   RGBGetBValue(colors[i]) * level / STEELSERIES_8Z_BRIGHTNESS_MAX);
        }

        if(gamesense.SendColors(scaled))
        {
            return;
        }
    }
    else
    {
        gamesense.Stop();
    }

    WriteColors(colors, mode, brightness);
}

void SteelSeriesApex8ZoneController::GameSenseLost(void* this_ptr)
{
    SteelSeriesApex8ZoneController* this_obj = (SteelSeriesApex8ZoneController*)this_ptr;

    std::vector<RGBColor> colors;
    uint8_t               brightness;

    {
        std::lock_guard<std::mutex> lock(this_obj->write_mutex);

        colors      = this_obj->direct_colors;
        brightness  = this_obj->direct_brightness;
    }

    this_obj->WriteColors(colors, 0, brightness);
}

void SteelSeriesApex8ZoneController::WriteColors(std::vector<RGBColor> colors, uint8_t mode, uint8_t brightness)
{
    /*-----------------------------------------------------*\
    | Called from both the device update path and the       |
    | GameSense keepalive thread                            |
    \*-----------------------------------------------------*/
    std::lock_guard<std::mutex> lock(write_mutex);

    uint8_t buffer[STEELSERIES_8Z_WRITE_PACKET_SIZE]    = { 0x00, 0x21, 0xFF };

    buffer[1]              += mode;

    for(unsigned int i = 0; i < colors.size(); i++)
    {
        uint8_t index       = i * 3;

        buffer[index + 3]   = RGBGetRValue(colors[i]);;
        buffer[index + 4]   = RGBGetGValue(colors[i]);;
        buffer[index + 5]   = RGBGetBValue(colors[i]);;
    }

    hid_write(dev, buffer, STEELSERIES_8Z_WRITE_PACKET_SIZE);

    if(current_brightness != brightness)
    {
        SetBrightness(brightness);
    }
}

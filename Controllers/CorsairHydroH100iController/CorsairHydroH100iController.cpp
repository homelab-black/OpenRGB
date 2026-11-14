/*---------------------------------------------------------*\
| CorsairHydroH100iController.cpp                           |
|                                                           |
|   Driver for Corsair H100i Hydro Series cooler            |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          20 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "CorsairHydroH100iController.h"

static void AppendHIDByte(unsigned char* buffer, unsigned int& index, unsigned char value)
{
    index++;
    buffer[index] = value;
}

CorsairHydroH100iController::CorsairHydroH100iController(hid_device* dev_handle, const char* dev_path, std::string dev_name)
{
    dev         = dev_handle;
    location    = dev_path;
    name        = dev_name;
}

CorsairHydroH100iController::~CorsairHydroH100iController()
{
    if(dev)
    {
        hid_close(dev);
    }
}

std::string CorsairHydroH100iController::GetLocation()
{
    return("HID: " + location);
}

std::string CorsairHydroH100iController::GetNameString()
{
    return(name);
}

void CorsairHydroH100iController::SetFixed(std::vector<RGBColor> & colors)
{
    SendColors(colors);
}

void CorsairHydroH100iController::SendHIDReport(unsigned char* buffer, unsigned int length)
{
    hid_write(dev, buffer, length);
}

void CorsairHydroH100iController::SendColors
    (
        std::vector<RGBColor>& colors
    )
{
    /*-----------------------------------------------------*\
    | Build 64-byte command buffer following the Coolit     |
    | protocol for setting LED colors in Static mode        |
    \*-----------------------------------------------------*/
    unsigned char buf[65];
    memset(buf, 0, sizeof(buf));

    unsigned int idx = 1;
    unsigned char cmd_id = CORSAIR_COOLIT_CMD_ID_BASE;

    /*-----------------------------------------------------*\
    | Select current LED = 0                                |
    \*-----------------------------------------------------*/
    AppendHIDByte(buf, idx, cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    AppendHIDByte(buf, idx, CORSAIR_COOLIT_WRITE_ONE_BYTE);
    AppendHIDByte(buf, idx, CORSAIR_COOLIT_REG_LED_SELECT_CURRENT);
    AppendHIDByte(buf, idx, 0x00);

    /*-----------------------------------------------------*\
    | Set mode = StaticColor                                |
    \*-----------------------------------------------------*/
    AppendHIDByte(buf, idx, cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    AppendHIDByte(buf, idx, CORSAIR_COOLIT_WRITE_ONE_BYTE);
    AppendHIDByte(buf, idx, CORSAIR_COOLIT_REG_LED_MODE);
    AppendHIDByte(buf, idx, CORSAIR_COOLIT_LED_MODE_STATIC_COLOR);

    /*-----------------------------------------------------*\
    | Send RGB triplet as LED_CycleColors (repeated 4x)     |
    \*-----------------------------------------------------*/
    if(!colors.empty())
    {
        RGBColor color   = colors[0];
        unsigned char r  = RGBGetRValue(color);
        unsigned char g  = RGBGetGValue(color);
        unsigned char b  = RGBGetBValue(color);

        AppendHIDByte(buf, idx, cmd_id);
        cmd_id = (cmd_id + 1) & 0xFF;
        AppendHIDByte(buf, idx, CORSAIR_COOLIT_WRITE_THREE_BYTES);
        AppendHIDByte(buf, idx, CORSAIR_COOLIT_REG_LED_CYCLE_COLORS);
        /*-----------------------------------------------------*\
        | count/length used by OpenCorsairLink                  |
        \*-----------------------------------------------------*/
        AppendHIDByte(buf, idx, 0x0C);

        for(int i = 0; i < 4; i++)
        {
            AppendHIDByte(buf, idx, r);
            AppendHIDByte(buf, idx, g);
            AppendHIDByte(buf, idx, b);
        }
    }

    /*-----------------------------------------------------*\
    | Set length at buf[0]                                  |
    \*-----------------------------------------------------*/
    buf[1] = (idx - 1) & 0xFF;

    SendHIDReport(buf, sizeof(buf));
}

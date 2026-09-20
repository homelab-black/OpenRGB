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
    return("USB: " + location);
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
    | Helper function to append bytes to buffer             |
    \*-----------------------------------------------------*/
    auto app = [&](unsigned char val)
    {
        idx++;
        buf[idx] = val & 0xFF;
    };

    /*-----------------------------------------------------*\
    | Select current LED = 0                               |
    \*-----------------------------------------------------*/
    app(cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    app(CORSAIR_COOLIT_WRITE_ONE_BYTE);
    app(CORSAIR_COOLIT_REG_LED_SELECT_CURRENT);
    app(0x00);

    /*-----------------------------------------------------*\
    | Set mode = StaticColor                               |
    \*-----------------------------------------------------*/
    app(cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    app(CORSAIR_COOLIT_WRITE_ONE_BYTE);
    app(CORSAIR_COOLIT_REG_LED_MODE);
    app(CORSAIR_COOLIT_LED_MODE_STATIC_COLOR);

    /*-----------------------------------------------------*\
    | Send RGB triplet as LED_CycleColors (repeated 4x)    |
    \*-----------------------------------------------------*/
    if(!colors.empty())
    {
        RGBColor color   = colors[0];
        unsigned char r  = RGBGetRValue(color);
        unsigned char g  = RGBGetGValue(color);
        unsigned char b  = RGBGetBValue(color);

        app(cmd_id);
        cmd_id = (cmd_id + 1) & 0xFF;
        app(CORSAIR_COOLIT_WRITE_THREE_BYTES);
        app(CORSAIR_COOLIT_REG_LED_CYCLE_COLORS);
        app(0x0C);  /* count/length used by OpenCorsairLink */

        for(int i = 0; i < 4; i++)
        {
            app(r);
            app(g);
            app(b);
        }
    }

    /*-----------------------------------------------------*\
    | Set length at buf[0]                                 |
    \*-----------------------------------------------------*/
    buf[1] = (idx - 1) & 0xFF;

    SendHIDReport(buf, sizeof(buf));
}

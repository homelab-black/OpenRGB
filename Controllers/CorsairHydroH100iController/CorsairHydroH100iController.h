/*---------------------------------------------------------*\
| CorsairHydroH100iController.h                             |
|                                                           |
|   Driver for Corsair H100i Hydro Series cooler            |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          20 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*-----------------------------------------------------*\
| Corsair H100/Link Commands                           |
\*-----------------------------------------------------*/
enum
{
    CORSAIR_COOLIT_CMD_ID_BASE              = 0x81,
    CORSAIR_COOLIT_WRITE_ONE_BYTE           = 0x06,
    CORSAIR_COOLIT_WRITE_THREE_BYTES        = 0x0A,
};

/*-----------------------------------------------------*\
| Corsair H100i Register Addresses                     |
\*-----------------------------------------------------*/
enum
{
    CORSAIR_COOLIT_REG_LED_SELECT_CURRENT   = 0x04,
    CORSAIR_COOLIT_REG_LED_MODE             = 0x06,
    CORSAIR_COOLIT_REG_LED_CYCLE_COLORS     = 0x0B,
};

/*-----------------------------------------------------*\
| Corsair H100i LED Modes                              |
\*-----------------------------------------------------*/
enum
{
    CORSAIR_COOLIT_LED_MODE_STATIC_COLOR    = 0x00,
};

class CorsairHydroH100iController
{
public:
    CorsairHydroH100iController(hid_device* dev_handle, const char* dev_path, std::string dev_name);
    ~CorsairHydroH100iController();

    std::string     GetLocation();
    std::string     GetNameString();

    void            SetFixed
                        (
                            std::vector<RGBColor> & colors
                        );

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;

    void            SendColors
                        (
                            std::vector<RGBColor> & colors
                        );

    void            SendHIDReport
                        (
                            unsigned char *         buffer,
                            unsigned int            length
                        );

};

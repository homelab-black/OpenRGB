/*---------------------------------------------------------*\
| RGBController_MSIKeyboard1603.h                           |
|                                                           |
|   RGBController for MSI Mystic Light MS-1603 keyboards    |
|                                                           |
|   hayatboj                                  18 April 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "MSIKeyboard1603Controller.h"
#include "RGBController.h"

enum MSI_1603_MODE
{
    MSI_1603_MODE_OFF        = 0,
    MSI_1603_MODE_STATIC     = 1,
    MSI_1603_MODE_BREATHING  = 2,
    MSI_1603_MODE_COLOR_CYCLE= 3,
    MSI_1603_MODE_WAVE       = 4,
    MSI_1603_MODE_DANCE      = 5,
    MSI_1603_MODE_WHEEL      = 6,
    MSI_1603_MODE_BOUNCY_WALL= 7,
};

class RGBController_MSIKeyboard1603 : public RGBController
{
public:
    static constexpr unsigned int MatrixWidth  = 13;
    static constexpr unsigned int MatrixHeight = 2;

    RGBController_MSIKeyboard1603(MSIKeyboard1603Controller* controller_ptr);
    ~RGBController_MSIKeyboard1603();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);
    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    void        SetupModes();
    MSI1603Color ColorFromRGB(RGBColor color) const;

    MSIKeyboard1603Controller* controller;
    std::array<unsigned int, MatrixWidth * MatrixHeight> zone_matrix_map_data;
};

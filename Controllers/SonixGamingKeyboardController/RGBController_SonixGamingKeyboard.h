/*---------------------------------------------------------*\
| RGBController_SonixGamingKeyboard.h                       |
|                                                           |
|   RGBController for Sonix based keyboards using the       |
|   "Gaming Force Z" software (CZC.Gaming Revenant)         |
|                                                           |
|   Roman Ondráček                              05 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SonixGamingKeyboardController.h"

class RGBController_SonixGamingKeyboard : public RGBController
{
public:
    RGBController_SonixGamingKeyboard(SonixGamingKeyboardController* controller_ptr);
    ~RGBController_SonixGamingKeyboard();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SonixGamingKeyboardController*  controller;

    void        SendLighting();
};

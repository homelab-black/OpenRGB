/*---------------------------------------------------------*\
| RGBController_SinowealthKeyboard16.h                      |
|                                                           |
|   Definitions and types for Sinowealth Keyboard with      |
|   PID:0016 (made spefically for ZUOYA X51)                |
|                                                           |
|   Dmitri Kalinichenko (Dima-Kal)              23 Jun 2021 |
|   Zagorodnikov Aleksey (glooom)               26 Jul 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SinowealthKeyboard16Controller.h"

class RGBController_SinowealthKeyboard16 : public RGBController
{
public:
    RGBController_SinowealthKeyboard16(SinowealthKeyboard16Controller* controller_ptr, std::function<void()> shutdown_callback);
    ~RGBController_SinowealthKeyboard16();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SinowealthKeyboard16Controller* controller;
    std::function<void()>           shutdown_callback;

    mode getModeItem(unsigned int mode_id);
};

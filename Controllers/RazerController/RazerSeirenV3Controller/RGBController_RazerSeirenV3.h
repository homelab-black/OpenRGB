/*---------------------------------------------------------*\
| RGBController_RazerSeirenV3.h                             |
|                                                           |
|   RGBController for Razer Seiren V3 Chroma                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "RazerSeirenV3Controller.h"

enum
{
    RAZER_SEIREN_V3_MODE_DIRECT,
    RAZER_SEIREN_V3_MODE_OFF,
    RAZER_SEIREN_V3_MODE_STATIC,
    RAZER_SEIREN_V3_MODE_BREATHING,
    RAZER_SEIREN_V3_MODE_SPECTRUM_CYCLE,
    RAZER_SEIREN_V3_MODE_AUDIO_REACTIVE,
};

class RGBController_RazerSeirenV3 : public RGBController
{
public:
    RGBController_RazerSeirenV3(RazerSeirenV3Controller* controller_ptr);
    ~RGBController_RazerSeirenV3();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    RazerSeirenV3Controller*  controller;
};

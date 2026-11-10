/*---------------------------------------------------------*\
| RGBController_AlienwareAW2725QF.h                         |
|                                                           |
|   RGBController for the Alienware AW2725QF monitor        |
|                                                           |
|   Ferréol DUBOIS COLI (Fefe_du_973)           23 Jan 2025 |
|   Jingcheng Tang (at1289)                     08 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "AlienwareAW2725QFController.h"
#include "RGBController.h"

class RGBController_AlienwareAW2725QF : public RGBController
{
public:
    explicit RGBController_AlienwareAW2725QF(AlienwareAW2725QFController* controller_ptr);
    ~RGBController_AlienwareAW2725QF();

    void        SetupZones();
    void        SetupModes();

    void        UpdateMode() override;

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceUpdateZoneMode(int zone);

private:
    AlienwareAW2725QFController* controller;
    const mode& GetEffectiveMode(int zone) const;
    void        SendMode(unsigned char led_mask, const mode& selected_mode);
};

/*---------------------------------------------------------*\
| RGBController_EdifierHaloPixelbarAmbient.h                |
|                                                           |
|   RGBController for Edifier Halo PixelBar Ambient Light   |
|                                                           |
|   Bartholomew Ho <qiangqiang101@hotmail.com>  20 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <memory>
#include "RGBController.h"
#include "EdifierHaloController.h"

class RGBController_EdifierHaloPixelbarAmbient : public RGBController
{
public:
    RGBController_EdifierHaloPixelbarAmbient(std::shared_ptr<EdifierHaloController> controller_ptr);
    ~RGBController_EdifierHaloPixelbarAmbient();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    std::shared_ptr<EdifierHaloController> controller;
};

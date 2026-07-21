/*---------------------------------------------------------*\
| RGBController_IlovbeeB87.h                               |
|                                                           |
|   RGBController for ilovbee B87 keyboard                  |
|                                                           |
|   w568w                                       18 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "IlovbeeB87Controller.h"
#include "RGBController.h"

class RGBController_IlovbeeB87 : public RGBController
{
public:
    RGBController_IlovbeeB87(IlovbeeB87Controller* controller_ptr);
    ~RGBController_IlovbeeB87();

    void SetupZones();

    void DeviceUpdateMode();

private:
    IlovbeeB87Controller* controller;
};

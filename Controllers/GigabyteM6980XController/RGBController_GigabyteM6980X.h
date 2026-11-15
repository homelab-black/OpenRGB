/*---------------------------------------------------------*\
| RGBController_GigabyteM6980X.h                            |
|                                                           |
|   RGBController for the Gigabyte M6980X mouse             |
|                                                           |
|   Tom Rehm (Tom-8192)                         09 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "GigabyteM6980XController.h"

class RGBController_GigabyteM6980X : public RGBController
{
public:
    RGBController_GigabyteM6980X(GigabyteM6980XController* controller_ptr);
    ~RGBController_GigabyteM6980X();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    GigabyteM6980XController* controller;
};

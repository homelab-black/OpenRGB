/*---------------------------------------------------------*\
| RGBController_AMDRadeonRX7900XTX.h                        |
|                                                           |
|   RGBController for AMD Radeon RX 7900 XTX reference RGB  |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AMDRadeonRX7900XTXController.h"

class RGBController_AMDRadeonRX7900XTX : public RGBController
{
public:
    RGBController_AMDRadeonRX7900XTX(AMDRadeonRX7900XTXController* controller_ptr);
    ~RGBController_AMDRadeonRX7900XTX();

    void SetupZones();
    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    AMDRadeonRX7900XTXController* controller;
};

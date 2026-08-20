/*---------------------------------------------------------*\
| RGBController_AsusLegacyACPI_Windows.h                    |
|                                                           |
|   RGBController for legacy ASUS motherboard ACPI          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "AsusLegacyACPIController_Windows.h"
#include "RGBController.h"

class RGBController_AsusLegacyACPI_Windows : public RGBController
{
public:
    RGBController_AsusLegacyACPI_Windows(AsusLegacyACPIController_Windows *controller_ptr);
    ~RGBController_AsusLegacyACPI_Windows();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    RGBColor ApplyBrightness(RGBColor color) const;
    RGBColor GetActiveColor() const;
    AsusLegacyACPIController_Windows *controller;
};

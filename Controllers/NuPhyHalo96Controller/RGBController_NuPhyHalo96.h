/*---------------------------------------------------------*\
| RGBController_NuPhyHalo96.h                               |
|                                                           |
|   OpenRGB Controller for NuPhy Halo96 V2 keyboard.        |
|   Implements the OpenRGB RGBController API.               |
|                                                           |
|   Amir Yelenov (GreamDesu)                    22 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/
#ifndef RGBCONTROLLER_NUPHYHALO96_H
#define RGBCONTROLLER_NUPHYHALO96_H

#include "RGBController.h"          // Base class
#include "NuPhyHalo96Controller.h"  // Low-level controller
#include <vector>
#include <string>
#include <chrono> // For std::chrono::steady_clock

class RGBController_NuPhyHalo96 : public ::RGBController
{
public:
    RGBController_NuPhyHalo96(NuPhyHalo96Controller* controller_ptr);
    ~RGBController_NuPhyHalo96() override;

    // Functions to be implemented from RGBControllerInterface (and RGBController)
    void SetupZones() override;
    void ResizeZone(int zone, int new_size) override; // Though likely fixed size for this device

    void DeviceUpdateLEDs() override;
    void UpdateZoneLEDs(int zone) override;     // Matched base: int zone
    void UpdateSingleLED(int led) override;   // Matched base: int led

    void DeviceUpdateMode() override;         
    void SetCustomMode() override;            

    // DeviceSaveMode() is not pure virtual in base, so override if custom save needed.
    // void DeviceSaveMode() override; 

private:
    NuPhyHalo96Controller* controller; // Pointer to the low-level HID controller
};

#endif // RGBCONTROLLER_NUPHYHALO96_H

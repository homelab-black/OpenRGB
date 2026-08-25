/*---------------------------------------------------------*\
| RGBController_CosmicByteFireflyKeyboard.h                 |
|                                                           |
|   RGBcontroller for Cosmic Byte Firefly TKL Keyboard      |
|                                                           |
|                                                           |
|   Akankshya Das                               25 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CosmicByteFireflyKeyboardController.h"

class RGBcontroller_CosmicByteFireflyKeyboard : public RGBController
{
public:
    RGBcontroller_CosmicByteFireflyKeyboard(CosmicByteFireflyKeyboardController* controller_ptr);
    ~RGBcontroller_CosmicByteFireflyKeyboard();

    void SetupZones();
    void DeviceUpdateLEDs() override;
    void DeviceUpdateZoneLEDs(int zone) override;
    void DeviceUpdateSingleLED(int led) override;
    void DeviceUpdateMode() override;

private:
    CosmicByteFireflyKeyboardController* controller;
};
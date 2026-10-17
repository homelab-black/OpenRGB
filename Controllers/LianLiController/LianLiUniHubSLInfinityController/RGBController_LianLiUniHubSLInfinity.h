/*---------------------------------------------------------*\
| RGBController_LianLiUniHubSLInfinity.h                    |
|                                                           |
|   RGBController for Lian Li SL Infinity Uni Hub           |
|                                                           |
|   Pau Verdeguer                               13 May 2026 |
|   Simon McKenna                               21 Oct 2023 |
|   Will Kennedy                                17 Jan 2023 |
|   Oliver P                                    26 Apr 2022 |
|   Credit to Luca Lovisa for original work.                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include "LianLiUniHubSLInfinityController.h"
#include "RGBController.h"

class RGBController_LianLiUniHubSLInfinity : public RGBController
{
public:
    RGBController_LianLiUniHubSLInfinity(LianLiUniHubSLInfinityController* controller_ptr);
    ~RGBController_LianLiUniHubSLInfinity();

    void SetupZones();

    void DeviceConfigureZone(int zone_idx);

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();
    void DeviceUpdateZoneMode(int zone);

private:
    const mode& GetEffectiveModeForZone(int zone_idx) const;
    const mode* GetRingMode(unsigned int zone_idx);
    const mode* GetMergedMode();
    void        SendFanQuantities();
    void        GetPortModes(unsigned int port, const mode** inner_mode, const mode** outer_mode, bool* outer_follows);
    bool        UpdateRing(unsigned int zone_idx, const mode& ring_mode);
    bool        UpdatePort(unsigned int port);
    bool        NeedsFullUpdate();
    void        UpdateAllPorts();
    bool        UpdatePerLEDZone(unsigned int zone_idx);

    LianLiUniHubSLInfinityController* controller;

    /*-----------------------------------------------------*\
    | Set until the first update, since the hub state is    |
    | unknown, and after a zone is resized.                 |
    \*-----------------------------------------------------*/
    bool                    needs_full_update;

    /*-----------------------------------------------------*\
    | The zone functions are called from the caller's       |
    | thread and the others from the device thread, so they |
    | can run at the same time.                             |
    \*-----------------------------------------------------*/
    std::mutex              update_mutex;
};

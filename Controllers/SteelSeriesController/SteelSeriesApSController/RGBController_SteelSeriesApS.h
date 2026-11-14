/*-----------------------------------------*\
|  RGBController_SteelSeriesApSALC.h        |
|                                           |
|  Generic RGB Interface for SteelSeries    |
|  ApS ALC LED Strip                        |
|                                           |
|  Morgan Guimard (morg) 1/25/2024          |
\*-----------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SteelSeriesApSController.h"
#include "SteelSeriesApSDevices.h"

class RGBController_SteelSeriesApS : public RGBController
{
public:
    RGBController_SteelSeriesApS(SteelSeriesApSController* controller_ptr, unsigned short pid, std::string full_name);
    ~RGBController_SteelSeriesApS();

    void SetupZones();
    void ResizeZone(int zone, int new_size);
    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    SteelSeriesApSController*       controller;
    uint16_t                        pid;
    steelseries_aps_klc_device      device;

    void SetupKLCZone(const steelseries_aps_klc_device&);
    void SetupALCZone();
};

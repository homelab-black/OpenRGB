#pragma once

#include "RGBController.h"
#include "AulaF2067Controller.h"

class RGBController_AulaF2067 : public RGBController
{
public:
    RGBController_AulaF2067(AulaF2067Controller* controller_ptr);
    ~RGBController_AulaF2067();

    void SetupZones();

    void ResizeZone(int zone, int new_size);
    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    AulaF2067Controller* controller;
};

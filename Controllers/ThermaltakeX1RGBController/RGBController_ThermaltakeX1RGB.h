#pragma once

#include "RGBController.h"
#include "ThermaltakeX1RGBController.h"

class RGBController_ThermaltakeX1RGB : public RGBController
{
public:
    RGBController_ThermaltakeX1RGB(ThermaltakeX1RGBController* controller_ptr);
    ~RGBController_ThermaltakeX1RGB();
    void SetupZones();
    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    ThermaltakeX1RGBController* controller;
};

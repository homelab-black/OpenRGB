#pragma once
#include "RGBController.h"
#include "DarkProjectKD87AController.h"

class RGBController_KD87A : public RGBController
{
public:
    RGBController_KD87A(DarkProjectKD87AController* controller_ptr);
    ~RGBController_KD87A();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);
    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);
    void        DeviceUpdateMode() { /* Только прямой режим */ }

private:
    DarkProjectKD87AController* controller;
};

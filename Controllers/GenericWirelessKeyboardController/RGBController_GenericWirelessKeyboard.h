#pragma once

#include "RGBController.h"
#include "RGBController_Dummy.h"
#include "GenericWirelessKeyboardController.h"

class RGBController_GenericWirelessKeyboard : public RGBController_Dummy
{
public:
    RGBController_GenericWirelessKeyboard(GenericWirelessKeyboardController *controller);
    ~RGBController_GenericWirelessKeyboard();

protected:
    void SetupZones() override;
    void UpdateLEDs() override;
    void SetCustomMode() override;

    void ResizeZone(int zone, int new_size) override;
    void DeviceUpdateLEDs() override;
    void UpdateZoneLEDs(int zone) override;
    void UpdateSingleLED(int led) override;
    void DeviceUpdateMode() override;

    void KeepaliveThreadFunction();

private:
    GenericWirelessKeyboardController *controller;
    std::atomic<bool> keepalive_thread_run;
    std::thread *keepalive_thread;
    std::chrono::time_point<std::chrono::steady_clock> last_update_time;
};

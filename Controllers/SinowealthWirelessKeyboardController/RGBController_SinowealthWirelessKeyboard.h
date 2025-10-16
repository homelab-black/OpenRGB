#pragma once

#include "SinowealthWirelessKeyboardController.h"
#include "RGBController.h"

class RGBController_SinowealthWirelessKeyboard : public RGBController
{
  public:
    RGBController_SinowealthWirelessKeyboard(SinowealthWirelessKeyboardController* controller);
    ~RGBController_SinowealthWirelessKeyboard();

  protected:
    virtual void SetupZones() override = 0;
    virtual void UpdateLEDs() override;
    void         SetCustomMode() override;

    void         ResizeZone(int zone, int new_size) override;
    virtual void DeviceUpdateLEDs() override;
    virtual void UpdateZoneLEDs(int zone) override;
    virtual void UpdateSingleLED(int led) override;
    virtual void DeviceUpdateMode() override;

    virtual void SendColor() = 0;

  private:
    void KeepaliveThreadFunction();

    SinowealthWirelessKeyboardController*              controller;
    std::atomic<bool>                                  keepalive_thread_run;
    std::thread*                                       keepalive_thread;
    std::chrono::time_point<std::chrono::steady_clock> last_update_time;
    std::atomic<bool>                                  color_changed;
};

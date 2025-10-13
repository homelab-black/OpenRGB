#pragma once

#include "GenericWirelessKeyboardController.h"
#include "RGBController.h"
#include "RGBController_Dummy.h"
#include <unordered_map>

class RGBController_GenericWirelessKeyboard : public RGBController_Dummy
{
  public:
    RGBController_GenericWirelessKeyboard(
        GenericWirelessKeyboardController *controller);
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
    RGBColor QuantizeColor(RGBColor color, unsigned char bits_to_discard);

  private:
    GenericWirelessKeyboardController                       *controller;
    std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map;
    std::mutex                                               led_color_map_lock;
    std::atomic<bool>                                        keepalive_thread_run;
    std::thread                                             *keepalive_thread;
    std::chrono::time_point<std::chrono::steady_clock>       last_update_time;
    std::atomic<bool>                                        color_changed;
};

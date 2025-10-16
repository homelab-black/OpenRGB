#pragma once

#include "RGBController.h"
#include "RGBController_SinowealthWirelessKeyboard.h"
#include "SinowealthWirelessKeyboardController.h"
#include <unordered_map>

class RGBController_SinowealthWirelessKeyboardFullColor : public RGBController_SinowealthWirelessKeyboard
{
  public:
    RGBController_SinowealthWirelessKeyboardFullColor(SinowealthWirelessKeyboardController* controller, unsigned char model_id);

  protected:
    void SetupZones() override;
    void UpdateLEDs() override;
    void SendColor() override;

    RGBColor QuantizeColor(RGBColor color, unsigned char bits_to_discard);

  private:
    SinowealthWirelessKeyboardController*                    controller;
    unsigned char                                            device_id;
    std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map;
    std::mutex                                               led_color_map_lock;
    std::atomic<bool>                                        keepalive_thread_run;
    std::thread*                                             keepalive_thread;
    std::chrono::time_point<std::chrono::steady_clock>       last_update_time;
    std::atomic<bool>                                        color_changed;
};

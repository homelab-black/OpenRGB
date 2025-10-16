#pragma once

#include "RGBController.h"
#include "RGBController_SinowealthWirelessKeyboard.h"
#include "SinowealthWirelessKeyboardController.h"
#include <unordered_map>

class RGBController_SinowealthWirelessKeyboardSolid : public RGBController_SinowealthWirelessKeyboard
{
  public:
    RGBController_SinowealthWirelessKeyboardSolid(SinowealthWirelessKeyboardController* controller);

  protected:
    void SetupZones() override;
    void SendColor() override;

  private:
    SinowealthWirelessKeyboardController*                    controller;
    std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map;
    std::mutex                                               led_color_map_lock;
    std::atomic<bool>                                        keepalive_thread_run;
    std::thread*                                             keepalive_thread;
    std::chrono::time_point<std::chrono::steady_clock>       last_update_time;
    std::atomic<bool>                                        color_changed;
};

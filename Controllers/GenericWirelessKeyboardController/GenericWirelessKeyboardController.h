#pragma once

#include "RGBController.h"
#include <hidapi.h>
#include <string>
#include <unordered_map>
#include <vector>

class GenericWirelessKeyboardController
{
  public:
    GenericWirelessKeyboardController(hid_device *dev_handle, const std::string &path, const std::string &name);
    ~GenericWirelessKeyboardController();

    std::vector<std::vector<unsigned char>> SplitChunks(const std::vector<unsigned char> &data);
    void                                    SendSolidColor(const RGBColor &color);
    void                                    SendLEDColors(std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map);
    void                                    SetLEDColors(std::unordered_map<RGBColor, std::vector<unsigned char>> led_color_map);
    std::string                             GetName();
    std::string                             GetLocation();

  private:
    hid_device *dev;
    std::string location;
    std::string name;
    std::mutex  lock;

    uint8_t CalculateCrc(const unsigned char *report, size_t len);
};

#pragma once

#include <hidapi.h>
#include <string>
#include <vector>
#include "RGBController.h"

class GenericWirelessKeyboardController
{
public:
    GenericWirelessKeyboardController(hid_device* dev_handle, const std::string& path, const std::string& name);
    ~GenericWirelessKeyboardController();

    std::vector<std::array<unsigned char, 14>> SplitChunks(const std::vector<unsigned char>& data);
    void SetSolidColor(const RGBColor& color);
    void SetLEDColors(std::vector<led>& leds, std::vector<RGBColor>& colors);
    std::string GetName();
    std::string GetLocation();

private:
    hid_device* dev;
    std::string location;
    std::string name;

    uint8_t CalculateCrc(const unsigned char *report);
};

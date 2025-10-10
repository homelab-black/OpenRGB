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

    void SendColors(const std::vector<RGBColor>& colors);
    std::string GetName();
    std::string GetLocation();

private:
    hid_device* dev;
    std::string location;
    std::string name;

    uint8_t CalculateCrc(const unsigned char *report);
};

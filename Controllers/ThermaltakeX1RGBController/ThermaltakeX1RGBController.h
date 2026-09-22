#pragma once

#include <hidapi.h>
#include <string>
#include <vector>
#include "RGBController.h"

#define THERMALTAKE_X1_LED_COUNT 110

class ThermaltakeX1RGBController
{
public:
    ThermaltakeX1RGBController(hid_device* dev_handle, const char* path);
    ~ThermaltakeX1RGBController();

    std::string GetDeviceLocation();
    std::string GetSerialString();
    void SetStatic(RGBColor color);
    void SetLEDs(std::vector<RGBColor> colors);

private:
    hid_device* dev;
    std::string location;
    void Initialize();
    void DrainResponses();
    bool Write(const unsigned char packet[64]);
};

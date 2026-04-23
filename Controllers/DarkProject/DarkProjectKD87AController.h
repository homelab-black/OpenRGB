#pragma once

#include <string>
#include <vector>

#include "RGBController.h"
#include "hidapi.h"

class DarkProjectKD87AController
{
public:
    DarkProjectKD87AController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~DarkProjectKD87AController();

    std::string GetLocation();
    std::string GetName();
    std::string GetSerial();

    void SetLedsDirect(std::vector<RGBColor> colors);

private:
    hid_device*  dev;
    std::string  location;
    std::string  name;
};

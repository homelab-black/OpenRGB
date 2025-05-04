#pragma once

#include <hidapi.h>
#include <vector>
#include <string>

class AlienwareMonitorController
{
public:
AlienwareMonitorController(hid_device* dev_handle, const char* path);
    ~AlienwareMonitorController();

    std::string GetLocation();
    std::string GetSerialString();
    void SendColor(unsigned char led_id, unsigned char r, unsigned char g, unsigned char b);

private:
    hid_device* dev;
    std::string location;

    void Initialize();
};

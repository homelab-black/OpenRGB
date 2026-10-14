/*------------------------------------------*\
|  SinowealthKeyboardController.h            |
|                                            |
|  Definitions and types for Sinowealth      |
|  Keyboard, Hopefully generic, this was     |
|  made spefically for FL eSports F11 KB     |
|                                            |
|  Dmitri Kalinichenko (Dima-Kal) 23/06/2021 |
\*-----------------------------------------=*/

#pragma once

#include "RGBController.h"
#include <vector>
#include <hidapi.h>

class SinowealthKeyboardController
{
public:
    SinowealthKeyboardController(hid_device* dev_cmd_handle, hid_device* dev_data_handle, const char *_path, const std::string &dev_name); //RGB, Command, path
    ~SinowealthKeyboardController();

    unsigned int    GetLEDCount();
    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerialString();
    unsigned char   GetCurrentMode();

    void            SetLEDColor(RGBColor* color_buf);
    void            SetStaticColor(RGBColor* color_buf);
    void            SetMode(unsigned char mode, unsigned char brightness, unsigned char speed, unsigned char color_mode);
    void            GetProfile();
    void            ReadFirmwareInfo();
    void            SetLEDsDirect(std::vector<RGBColor> colors);
private:
    hid_device*     dev_cmd;
    hid_device*     dev_data;
    device_type     type;
    unsigned int    led_count;
    unsigned char   current_mode;
    unsigned char   current_speed;
    std::string     location;
    std::string     name;
};

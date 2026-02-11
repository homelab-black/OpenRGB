/*---------------------------------------------------------*\
| LEDStripController.h                                      |
|                                                           |
|   Driver for serial LED strips                            |
|                                                           |
|   Adam Honse (calcprogrammer1@gmail.com)      Dec 11 2016 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include "RGBController.h"
#include "i2c_smbus.h"
#include "serial_port.h"
#include "net_port.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifndef TRUE
#define TRUE true
#define FALSE false
#endif

#ifndef WIN32
#define LPSTR           char *
#define strtok_s        strtok_r
#endif


#define LEDSTRIP_CFG_KEY "LEDStripDevices"

enum class LedProtocol
{
    KEYBOARD_VISUALIZER,
    ADALIGHT,
    TPM2,
    BASIC_I2C
};

NLOHMANN_JSON_SERIALIZE_ENUM(LedProtocol, {
    {LedProtocol::KEYBOARD_VISUALIZER, "keyboard_visualizer"},
    {LedProtocol::ADALIGHT, "adalight"},
    {LedProtocol::TPM2, "tpm2"},
    {LedProtocol::BASIC_I2C, "basic_i2c"}
});

struct LEDStripDevice
{
    std::string     name = "LED Strip";
    std::string     port;
    unsigned int    baud        = 0;
    unsigned int    num_leds    = 0;
    LedProtocol    protocol = LedProtocol::KEYBOARD_VISUALIZER;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(LEDStripDevice,
    name,
    port,
    baud,
    num_leds,
    protocol
);

class LEDStripController
{
public:
    LEDStripController(std::string dev_name);
    ~LEDStripController();

    void        Initialize(char* ledstring, LedProtocol proto);

    void        InitializeI2C(char* i2cname);
    void        InitializeSerial(char* portname, int baud);
    void        InitializeUDP(char* clientname, char* port);

    char*       GetLEDString();
    std::string GetLocation();
    std::string GetName();

    void        SetLEDs(std::vector<RGBColor> colors);

    void        SetLEDsKeyboardVisualizer(std::vector<RGBColor> colors);
    void        SetLEDsAdalight(std::vector<RGBColor> colors);
    void        SetLEDsTPM2(std::vector<RGBColor> colors);
    void        SetLEDsBasicI2C(std::vector<RGBColor> colors);

    int num_leds;

private:
    int baud_rate;

    char led_string[1024];
    std::string port_name;
    std::string client_name;
    std::string name;
    serial_port *serialport;
    net_port *udpport;
    i2c_smbus_interface *i2cport;
    unsigned char i2c_addr;
    LedProtocol protocol;
};

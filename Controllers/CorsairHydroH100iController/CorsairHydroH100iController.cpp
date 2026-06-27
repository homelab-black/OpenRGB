/*---------------------------------------------------------*\
| CorsairHydroH100iController.cpp                           |
|                                                           |
|   Driver for Corsair H100i Hydro Series cooler            |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          27 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <iomanip>
#include <sstream>
#include "CorsairHydroH100iController.h"

CorsairHydroH100iController::CorsairHydroH100iController(libusb_device_handle* dev_handle, std::string dev_name)
{
    dev         = dev_handle;
    name        = dev_name;

    /*-----------------------------------------------------*\
    | Fill in location string with USB ID                   |
    \*-----------------------------------------------------*/
    libusb_device_descriptor descriptor;
    libusb_get_device_descriptor(libusb_get_device(dev_handle), &descriptor);

    std::stringstream location_stream;
    location_stream << std::hex << std::setfill('0') << std::setw(4) << descriptor.idVendor
                    << ":" << std::hex << std::setfill('0') << std::setw(4) << descriptor.idProduct;
    location = location_stream.str();
}

CorsairHydroH100iController::~CorsairHydroH100iController()
{
    if(dev)
    {
        libusb_close(dev);
    }
}

std::string CorsairHydroH100iController::GetLocation()
{
    return("USB: " + location);
}

std::string CorsairHydroH100iController::GetNameString()
{
    return(name);
}

void CorsairHydroH100iController::SetFixed(std::vector<RGBColor> & colors)
{
    SendColors(colors);
}

int CorsairHydroH100iController::GetHIDInterfaceNumber()
{
    /*-----------------------------------------------------*\
    | Find the HID interface number (bInterfaceClass == 3)  |
    \*-----------------------------------------------------*/
    libusb_config_descriptor* config_desc = NULL;
    libusb_device* dev_device = libusb_get_device(dev);

    if(libusb_get_active_config_descriptor(dev_device, &config_desc) == 0)
    {
        for(int i = 0; i < (int)config_desc->bNumInterfaces; i++)
        {
            const libusb_interface* intf = &config_desc->interface[i];
            if(intf->altsetting && intf->altsetting->bInterfaceClass == 3)
            {
                int intf_num = intf->altsetting->bInterfaceNumber;
                libusb_free_config_descriptor(config_desc);
                return intf_num;
            }
        }
        libusb_free_config_descriptor(config_desc);
    }

    return -1;
}

void CorsairHydroH100iController::SendHIDReport(unsigned char* buffer, unsigned int length)
{
    int intf_num = GetHIDInterfaceNumber();
    if(intf_num < 0)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Send using SET_REPORT (HID Output)                    |
    | ctrl_transfer(request_type, request, value, index,    |
    |               data, length, timeout)                  |
    | 0x21 = Device-to-Host, Class, Interface               |
    | 0x09 = SET_REPORT                                     |
    | (2 << 8) | 0 = Output Report ID 0                     |
    \*-----------------------------------------------------*/
    libusb_control_transfer(dev, 0x21, 0x09, (2 << 8) | 0, intf_num, buffer, length, 1000);
}

void CorsairHydroH100iController::SendColors
    (
        std::vector<RGBColor>& colors
    )
{
    /*-----------------------------------------------------*\
    | Build 64-byte command buffer following the Coolit     |
    | protocol for setting LED colors in Static mode        |
    \*-----------------------------------------------------*/
    unsigned char buf[64];
    memset(buf, 0, sizeof(buf));

    unsigned int idx = 0;
    unsigned char cmd_id = CORSAIR_COOLIT_CMD_ID_BASE;

    /*-----------------------------------------------------*\
    | Helper function to append bytes to buffer             |
    \*-----------------------------------------------------*/
    auto app = [&](unsigned char val)
    {
        idx++;
        buf[idx] = val & 0xFF;
    };

    /*-----------------------------------------------------*\
    | Select current LED = 0                               |
    \*-----------------------------------------------------*/
    app(cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    app(CORSAIR_COOLIT_WRITE_ONE_BYTE);
    app(CORSAIR_COOLIT_REG_LED_SELECT_CURRENT);
    app(0x00);

    /*-----------------------------------------------------*\
    | Set mode = StaticColor                               |
    \*-----------------------------------------------------*/
    app(cmd_id);
    cmd_id = (cmd_id + 1) & 0xFF;
    app(CORSAIR_COOLIT_WRITE_ONE_BYTE);
    app(CORSAIR_COOLIT_REG_LED_MODE);
    app(CORSAIR_COOLIT_LED_MODE_STATIC_COLOR);

    /*-----------------------------------------------------*\
    | Send RGB triplet as LED_CycleColors (repeated 4x)    |
    \*-----------------------------------------------------*/
    if(!colors.empty())
    {
        RGBColor color   = colors[0];
        unsigned char r  = RGBGetRValue(color);
        unsigned char g  = RGBGetGValue(color);
        unsigned char b  = RGBGetBValue(color);

        app(cmd_id);
        cmd_id = (cmd_id + 1) & 0xFF;
        app(CORSAIR_COOLIT_WRITE_THREE_BYTES);
        app(CORSAIR_COOLIT_REG_LED_CYCLE_COLORS);
        app(0x0C);  /* count/length used by OpenCorsairLink */

        for(int i = 0; i < 4; i++)
        {
            app(r);
            app(g);
            app(b);
        }
    }

    /*-----------------------------------------------------*\
    | Set length at buf[0]                                 |
    \*-----------------------------------------------------*/
    buf[0] = idx & 0xFF;

    SendHIDReport(buf, sizeof(buf));
}

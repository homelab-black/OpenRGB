/*---------------------------------------------------------*\
| ASUSLampArrayController.cpp                               |
|                                                           |
|   Driver for ASUS Vivobook LampArray Keyboard (ITE5570)   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ASUSLampArrayController.h"
#include <fstream>

ASUSLampArrayController::ASUSLampArrayController(hid_device* dev_handle, const char* path, unsigned short pid)
{
    dev        = dev_handle;
    location   = path;
    product_id = pid;

    /*-----------------------------------------------------*\
    | Determine Report IDs based on the PID                 |
    \*-----------------------------------------------------*/
    if(pid == 0x19B6)
    {
        firmware_report_id = 0x0B;
        color_report_id    = 0x05;
    }
    else if(pid == 0x5570)
    {
        firmware_report_id = 0x46;
        color_report_id    = 0x45;
    }
    else
    {
        // Fallback for ASUS Vivobook LampArray
        firmware_report_id = 0x46;
        color_report_id    = 0x45;
    }

    /*-----------------------------------------------------*\
    | Initialize device in Host Mode (disable firmware RGB) |
    \*-----------------------------------------------------*/
    SetHostMode(true);
}

ASUSLampArrayController::~ASUSLampArrayController()
{
    hid_close(dev);
}

std::string ASUSLampArrayController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ASUSLampArrayController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    std::string return_string(return_wstring.begin(), return_wstring.end());

    return(return_string);
}

void ASUSLampArrayController::SetHostMode(bool host_mode)
{
    /*-----------------------------------------------------*\
    | Set firmware mode. Host mode = 0x00, Firmware = 0x01  |
    \*-----------------------------------------------------*/
    unsigned char buf[2];
    buf[0] = firmware_report_id;
    buf[1] = host_mode ? 0x00 : 0x01;

    hid_send_feature_report(dev, buf, sizeof(buf));
}

void ASUSLampArrayController::SetHardwareRainbow(bool enable)
{
    /*-----------------------------------------------------*\
    | Firmware mode gives control back to the EC            |
    \*-----------------------------------------------------*/
    SetHostMode(!enable);

    /*-----------------------------------------------------*\
    | On 0x19B6 (S5406 / S5506 series), toggle ASUS WMI     |
    | OEM Rainbow generator                                 |
    \*-----------------------------------------------------*/
    if(product_id == 0x19B6)
    {
        std::ofstream method_id("/sys/kernel/debug/asus-nb-wmi/method_id");
        std::ofstream dev_id("/sys/kernel/debug/asus-nb-wmi/dev_id");
        std::ofstream ctrl_param("/sys/kernel/debug/asus-nb-wmi/ctrl_param");

        if(method_id.is_open() && dev_id.is_open() && ctrl_param.is_open())
        {
            method_id << "0x00000001";
            dev_id << "0x0005002f";
            ctrl_param << (enable ? "0x00000000" : "0x00000001");

            method_id.close();
            dev_id.close();
            ctrl_param.close();

            std::ifstream devs("/sys/kernel/debug/asus-nb-wmi/devs");
            if(devs.is_open())
            {
                std::string temp;
                devs >> temp;
                devs.close();
            }
        }
    }
}

void ASUSLampArrayController::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char brightness)
{
    /*-----------------------------------------------------*\
    | Build payload for static color                        |
    | 0: Report ID                                          |
    | 1: 0x01 (Constant)                                    |
    | 2-5: 0x00 (Padding)                                   |
    | 6: R                                                  |
    | 7: G                                                  |
    | 8: B                                                  |
    | 9: Brightness (0-255)                                 |
    \*-----------------------------------------------------*/
    unsigned char buf[10];
    buf[0] = color_report_id;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = r;
    buf[7] = g;
    buf[8] = b;
    buf[9] = brightness;

    hid_send_feature_report(dev, buf, sizeof(buf));
}

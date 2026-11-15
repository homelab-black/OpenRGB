/*---------------------------------------------------------*\
| RazerNariController.cpp                                   |
|                                                           |
|   Driver for Razer Nari devices                           |
|                                                           |
|   Greg Sandstrom (superstrom)                 22 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "RazerNariController.h"
#include "RazerDevices.h"

using namespace std::chrono_literals;

RazerNariController::RazerNariController(hid_device* dev_handle, const char* path, unsigned short pid, std::string dev_name)
{
    dev               = dev_handle;
    dev_pid           = pid;
    location          = path;
    name              = dev_name;

    /*-----------------------------------------------------------------*\
    | Loop through all known devices to look for a name match           |
    \*-----------------------------------------------------------------*/
    for(unsigned int i = 0; i < RAZER_NUM_DEVICES; i++)
    {
        if(device_list[i]->pid == dev_pid)
        {
            /*---------------------------------------------------------*\
            | Set device ID                                             |
            \*---------------------------------------------------------*/
            device_index = i;
        }
    }

    /*-----------------------------------------------------------------*\
    | Determine matrix type for device                                  |
    \*-----------------------------------------------------------------*/
    matrix_type = device_list[device_index]->matrix_type;
}

RazerNariController::~RazerNariController()
{
    hid_close(dev);
}

std::string RazerNariController::GetName()
{
    return(name);
}

unsigned int RazerNariController::GetDeviceIndex()
{
    return(device_index);
}

device_type RazerNariController::GetDeviceType()
{
    return(device_list[device_index]->type);
}

std::string RazerNariController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string RazerNariController::GetFirmwareString()
{
    return(razer_get_firmware());
}

std::string RazerNariController::GetSerialString()
{
    return(razer_get_serial());
}

unsigned char RazerNariController::GetMaxBrightness()
{
    /*-----------------------------------------------------*\
    | Max brightness for most devices is 0xFF (255)         |
    |   Add PIDs only for devices that use 0x64 (100)       |
    |   or any another arbitrary value                      |
    \*-----------------------------------------------------*/
    unsigned char max_brightness = 255;

    return(max_brightness);
}

void RazerNariController::SetDirect(RGBColor* colors)
{
    razer_nari_request_report report = razer_nari_create_report();

    // how to get the number of leds?
    unsigned int led_count = device_list[device_index]->cols;

    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    // ff0a00ff0412f10572ffffff00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    report.report_id = RAZER_NARI_REPORT_ID;
    report.hdr1 = 0x0a;
    report.hdr2 = 0x00;
    report.hdr3 = 0xff;
    report.hdr4 = 0x04;
    report.hdr5 = 0x12;
    report.hdr6 = 0xf1;
    report.hdr7 = 0x05;
    report.command_id = RAZER_NARI_CMD_LIGHTING_SET_COLOR;

    for(unsigned int led_idx = 0; led_idx < led_count; led_idx++)
    {
        report.arguments[1 + (led_idx * 3)] = RGBGetRValue(colors[led_idx]);
        report.arguments[2 + (led_idx * 3)] = RGBGetGValue(colors[led_idx]);
        report.arguments[3 + (led_idx * 3)] = RGBGetBValue(colors[led_idx]);
    }

    razer_usb_send(&report);
}

void RazerNariController::SetBrightness(unsigned char brightness)
{
    razer_nari_request_report report = razer_nari_create_report();

    //  0 1 2 3 4 5 6 7 8 9
    // ff0a00ff0412f10371ff000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    report.report_id = RAZER_NARI_REPORT_ID;
    report.hdr1 = 0x0a;
    report.hdr2 = 0x00;
    report.hdr3 = 0xff;
    report.hdr4 = 0x04;
    report.hdr5 = 0x12;
    report.hdr6 = 0xf1;
    report.hdr7 = 0x03;
    report.command_id = RAZER_NARI_CMD_LIGHTING_SET_BRIGHTNESS;

    report.arguments[0] = brightness;

    razer_usb_send(&report);
}

razer_nari_request_report RazerNariController::razer_nari_create_report()
{
    razer_nari_request_report new_report;

    /*---------------------------------------------------------*\
    | Zero out the new report                                   |
    \*---------------------------------------------------------*/
    memset(&new_report, 0, sizeof(razer_nari_request_report));

    new_report.report_id = RAZER_NARI_REPORT_ID;

    return new_report;
}

std::string RazerNariController::razer_get_firmware()
{
    std::string                       firmware_string;
    struct razer_nari_request_report  report                  = razer_nari_create_report();
    struct razer_nari_response_report response_report;

    // SET_REPORT OUT FF0A00FD0407F1050205
    // GET_REPORT IN  FF1B05FE07041F1402303130362C636C69656E742C323031382D31   ff 1b 05 fe 07 04 1f 14 02 0  1  0  6  ,  c  l  i  e  n  t  ,  2  0  1  8  -  1 

    //  0 1 2 3 4 5 6 7 8 9 0 1
    // ff0a00fd0407f1050205...
    report.report_id = RAZER_NARI_REPORT_ID;
    report.hdr1 = 0x0a;
    report.hdr2 = 0x00;
    report.hdr3 = 0xfd;
    report.hdr4 = 0x04;
    report.hdr5 = 0x07;
    report.hdr6 = 0xf1;
    report.hdr7 = 0x05;
    report.command_id = 0x02;
    report.arguments[0] = 0x05;

    std::this_thread::sleep_for(1ms);
    razer_usb_send(&report);
    std::this_thread::sleep_for(1ms);
    razer_usb_receive(&response_report);

    // Receive back 64-byte response with text starting at byte-9
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    // FF1B05FE07041F1402303130362C636C69656E742C323031382D31...
    //                    0 1 0 6 , c l i e n t , 2 0 1 8 - 1

    if(response_report.report_id == 0xFF)
    {
        firmware_string.assign((const char*)&response_report.arguments[8], 18);
    }

    for(size_t i = 0; i < 22; i++)
    {
        if(firmware_string[i] < 30 || firmware_string[i] > 126)
        {
            firmware_string[i] = ' ';
        }
    }

    return firmware_string;
}

std::string RazerNariController::razer_get_serial()
{
    std::string                       serial_string;
    struct razer_nari_request_report  report0 = razer_nari_create_report();
    struct razer_nari_request_report  report1 = razer_nari_create_report();
    struct razer_nari_request_report  report2 = razer_nari_create_report();

    struct razer_nari_response_report response_report;

    // SET_REPORT OUT FF0A00FF0400000C002EC4007F00060C010
    // SET_REPORT OUT FF0A00FF0400000284
    // SET_REPORT OUT FF0A00FD040000058160C010
    // GET_REPORT IN  FF1B05FE00040F148160C0484E32303439443039303031343236  ff 1b 05 fe 00 04 0f 14 81 60 c0 H  N  2  0  4  9  D  0  9  0  0  1  4  2  6 

    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7
    // ff0a00ff0400000c002ec4007f00060c010...
    report0.report_id = RAZER_NARI_REPORT_ID;
    report0.hdr1 = 0x0a;
    report0.hdr2 = 0x00;
    report0.hdr3 = 0xff;
    report0.hdr4 = 0x04;
    report0.hdr5 = 0x00;
    report0.hdr6 = 0x00;
    report0.hdr7 = 0x0c;
    report0.command_id = 0x00;
    report0.arguments[0] = 0x2e;
    report0.arguments[1] = 0xc4;
    report0.arguments[2] = 0x00;
    report0.arguments[3] = 0x7f;
    report0.arguments[4] = 0x00;
    report0.arguments[5] = 0x00;
    report0.arguments[6] = 0x60;
    report0.arguments[7] = 0xc0;
    report0.arguments[8] = 0x10;

    //  0 1 2 3 4 5 6 7 8 9 0 1
    // ff0a00ff0400000284...
    report1.report_id = RAZER_NARI_REPORT_ID;
    report1.hdr1 = 0x0a;
    report1.hdr2 = 0x00;
    report1.hdr3 = 0xff;
    report1.hdr4 = 0x04;
    report1.hdr5 = 0x00;
    report1.hdr6 = 0x00;
    report1.hdr7 = 0x02;
    report1.command_id = 0x84;

    //  0 1 2 3 4 5 6 7 8 9 0 1
    // ff0a00fd040000058160c010...
    report2.report_id = RAZER_NARI_REPORT_ID;
    report2.hdr1 = 0x0a;
    report2.hdr2 = 0x00;
    report2.hdr3 = 0xfd;
    report2.hdr4 = 0x04;
    report2.hdr5 = 0x00;
    report2.hdr6 = 0x00;
    report2.hdr7 = 0x05;
    report2.command_id = 0x81;
    report2.arguments[0] = 0x60;
    report2.arguments[1] = 0xc0;
    report2.arguments[2] = 0x10;

    std::this_thread::sleep_for(1ms);
    razer_usb_send(&report0);
    std::this_thread::sleep_for(1ms);
    razer_usb_send(&report1);
    std::this_thread::sleep_for(1ms);
    razer_usb_send(&report2);
    std::this_thread::sleep_for(1ms);
    razer_usb_receive(&response_report);

    // Receive back 64-byte response with serial number starting at byte-12
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    // ff1b05fe00040f148160c0484e32303439443039303031343236
    //                        H N 2 0 4 9 D 0 9 0 0 1 4 2 6

    if(response_report.report_id == 0xFF)
    {
        serial_string.assign((const char*)&response_report.arguments[10], 15);
    }

    return serial_string;
}

/*---------------------------------------------------------------------------------*\
| USB transfer functions                                                            |
\*---------------------------------------------------------------------------------*/

int RazerNariController::razer_usb_receive(razer_nari_response_report* report)
{
    return hid_read(dev, (unsigned char*)report, sizeof(*report));
}

int RazerNariController::razer_usb_send(razer_nari_request_report* report)
{
    return hid_write(dev, (unsigned char*)report, sizeof(*report));
}

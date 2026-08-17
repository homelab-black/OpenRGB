/*---------------------------------------------------------*\
| RazerSeirenV3Controller.cpp                                |
|                                                           |
|   Driver for Razer Seiren V3 Chroma (64-byte report)      |
|                                                           |
|   Reverse-engineered 2026-08-17 on real hardware          |
|   (PID 0x056F, no Synapse installed)                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "RazerSeirenV3Controller.h"
#include "RazerController.h"
#include "RazerDevices.h"

using namespace std::chrono_literals;

RazerSeirenV3Controller::RazerSeirenV3Controller(hid_device* dev_handle, const char* path, unsigned short pid, std::string dev_name)
{
    dev               = dev_handle;
    dev_pid           = pid;
    location          = path;
    name              = dev_name;

    /*-----------------------------------------------------*\
    | Loop through all known devices to look for a name     |
    | match                                                 |
    \*-----------------------------------------------------*/
    for(unsigned int i = 0; i < RAZER_NUM_DEVICES; i++)
    {
        if(device_list[i]->pid == dev_pid)
        {
            device_index = i;
        }
    }

    /*-----------------------------------------------------*\
    | Seiren V3 Chroma uses report id 0x07 and              |
    | transaction id 0x0A                                    |
    \*-----------------------------------------------------*/
    report_index        = 0x07;
    response_index      = 0x07;
    dev_transaction_id  = 0x0A;
    dev_led_id          = RAZER_LED_ID_BACKLIGHT;
}

RazerSeirenV3Controller::~RazerSeirenV3Controller()
{
    hid_close(dev);
}

std::string RazerSeirenV3Controller::GetName()
{
    return(name);
}

unsigned int RazerSeirenV3Controller::GetDeviceIndex()
{
    return(device_index);
}

device_type RazerSeirenV3Controller::GetDeviceType()
{
    return(device_list[device_index]->type);
}

std::string RazerSeirenV3Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string RazerSeirenV3Controller::GetFirmwareString()
{
    return(razer_get_firmware());
}

std::string RazerSeirenV3Controller::GetSerialString()
{
    return(razer_get_serial());
}

unsigned char RazerSeirenV3Controller::GetMaxBrightness()
{
    return(255);
}

/*---------------------------------------------------------*\
| Direct mode: push a 16-LED custom frame continuously.     |
| The device reverts to dark unless frame data is          |
| re-sent; the custom-mode command (0x08) must be sent      |
| once, not per cycle (re-sending it causes a visible       |
| flash).                                                  |
\*---------------------------------------------------------*/
void RazerSeirenV3Controller::SetDirect(RGBColor* colors)
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x03, (5 + (3 * device_list[device_index]->cols)));

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0;
    report.arguments[3]             = 0;
    report.arguments[4]             = device_list[device_index]->cols - 1;

    for(unsigned int led_idx = 0; led_idx < device_list[device_index]->cols; led_idx++)
    {
        report.arguments[5 + (led_idx * 3)] = RGBGetRValue(colors[led_idx]);
        report.arguments[6 + (led_idx * 3)] = RGBGetGValue(colors[led_idx]);
        report.arguments[7 + (led_idx * 3)] = RGBGetBValue(colors[led_idx]);
    }

    razer_usb_send(&report);
}

void RazerSeirenV3Controller::SetModeOff()
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x06);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x00;

    razer_usb_send(&report);
}

void RazerSeirenV3Controller::SetModeStatic(unsigned char red, unsigned char grn, unsigned char blu)
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x09);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x01;
    report.arguments[3]             = 0x00;
    report.arguments[4]             = 0x00;
    report.arguments[5]             = 0x01;
    report.arguments[6]             = red;
    report.arguments[7]             = grn;
    report.arguments[8]             = blu;

    razer_usb_send(&report);
}

void RazerSeirenV3Controller::SetModeBreathingOneColor(unsigned char red, unsigned char grn, unsigned char blu)
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x09);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x02;
    report.arguments[3]             = 0x01;
    report.arguments[4]             = 0x00;
    report.arguments[5]             = 0x01;
    report.arguments[6]             = red;
    report.arguments[7]             = grn;
    report.arguments[8]             = blu;

    razer_usb_send(&report);
}

void RazerSeirenV3Controller::SetModeBreathingTwoColors(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char r2, unsigned char g2, unsigned char b2)
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x0C);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x02;
    report.arguments[3]             = 0x02;
    report.arguments[4]             = 0x00;
    report.arguments[5]             = 0x02;
    report.arguments[6]             = r1;
    report.arguments[7]             = g1;
    report.arguments[8]             = b1;
    report.arguments[9]             = r2;
    report.arguments[10]            = g2;
    report.arguments[11]            = b2;

    razer_usb_send(&report);
}

void RazerSeirenV3Controller::SetModeSpectrumCycle()
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x06);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x03;

    razer_usb_send(&report);
}

/*---------------------------------------------------------*\
| Effect 0x04 on the Seiren is AUDIO-REACTIVE rainbow,      |
| not wave (classic protocol maps 0x04 to wave; this mic    |
| remaps it). Verified on hardware: reacts to mic input.    |
\*---------------------------------------------------------*/
void RazerSeirenV3Controller::SetModeAudioReactive()
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x0F, 0x02, 0x06);

    report.arguments[0]             = RAZER_STORAGE_NO_SAVE;
    report.arguments[1]             = dev_led_id;
    report.arguments[2]             = 0x04;
    report.arguments[3]             = 0x01;
    report.arguments[4]             = 0x28;

    razer_usb_send(&report);
}

unsigned char RazerSeirenV3Controller::razer_seiren_v3_calculate_crc(razer_seiren_v3_report* report)
{
    /*-----------------------------------------------------*\
    | CRC = XOR of bytes 2..61 (payload bytes 1..60,        |
    | includes transaction id, excludes status and crc).    |
    | Verified against the device's own response CRC.       |
    \*-----------------------------------------------------*/
    unsigned char   crc         = 0;
    unsigned char*  report_ptr  = (unsigned char*)report;

    for(unsigned int i = 2; i < 62; i++)
    {
        crc ^= report_ptr[i];
    }

    return crc;
}

razer_seiren_v3_report RazerSeirenV3Controller::razer_seiren_v3_create_report(unsigned char command_class, unsigned char command_id, unsigned char data_size)
{
    razer_seiren_v3_report new_report;

    memset(&new_report, 0, sizeof(razer_seiren_v3_report));

    new_report.report_id            = report_index;
    new_report.status               = 0x00;
    new_report.transaction_id       = dev_transaction_id;
    new_report.remaining_packets    = 0x00;
    new_report.protocol_type        = 0x00;
    new_report.data_size            = data_size;
    new_report.command_class        = command_class;
    new_report.command_id           = command_id;

    return new_report;
}

razer_seiren_v3_report RazerSeirenV3Controller::razer_seiren_v3_create_response()
{
    razer_seiren_v3_report new_report;

    memset(&new_report, 0, sizeof(razer_seiren_v3_report));

    new_report.report_id            = response_index;
    new_report.status               = 0x00;
    new_report.transaction_id       = dev_transaction_id;
    new_report.remaining_packets    = 0x00;
    new_report.protocol_type        = 0x00;
    new_report.command_class        = 0x00;
    new_report.command_id           = 0x00;
    new_report.data_size            = 0x00;

    return new_report;
}

void RazerSeirenV3Controller::razer_set_device_mode(unsigned char mode)
{
    struct razer_seiren_v3_report   report          = razer_seiren_v3_create_report(0x00, RAZER_COMMAND_ID_SET_DEVICE_MODE, 0x02);

    report.arguments[0]             = 0x00;
    report.arguments[1]            = mode;

    razer_usb_send(&report);
}

std::string RazerSeirenV3Controller::razer_get_firmware()
{
    std::string                     firmware_string         = "";
    struct razer_seiren_v3_report   report                  = razer_seiren_v3_create_report(0x00, RAZER_COMMAND_ID_GET_FIRMWARE_VERSION, 0x02);
    struct razer_seiren_v3_report   response_report         = razer_seiren_v3_create_response();

    std::this_thread::sleep_for(2ms);
    razer_usb_send(&report);
    std::this_thread::sleep_for(5ms);
    razer_usb_receive(&response_report);

    firmware_string = "v" + std::to_string(response_report.arguments[0]) + "." + std::to_string(response_report.arguments[1]);

    return firmware_string;
}

std::string RazerSeirenV3Controller::razer_get_serial()
{
    char                            serial_string[64]       = "";
    struct razer_seiren_v3_report   report                  = razer_seiren_v3_create_report(0x00, RAZER_COMMAND_ID_GET_SERIAL_STRING, 0x16);
    struct razer_seiren_v3_report   response_report         = razer_seiren_v3_create_response();

    std::this_thread::sleep_for(2ms);
    razer_usb_send(&report);
    std::this_thread::sleep_for(5ms);
    razer_usb_receive(&response_report);

    memcpy(&serial_string[0], &response_report.arguments[0], 22);
    serial_string[22] = '\0';

    for(size_t i = 0; i < 22; i++)
    {
        if(serial_string[i] < 30 || serial_string[i] > 126)
        {
            serial_string[i] = ' ';
        }
    }

    std::string ret_string = serial_string;
    return ret_string;
}

int RazerSeirenV3Controller::razer_usb_receive(razer_seiren_v3_report* report)
{
    return hid_get_feature_report(dev, (unsigned char*)report, sizeof(*report));
}

int RazerSeirenV3Controller::razer_usb_send(razer_seiren_v3_report* report)
{
    report->crc = razer_seiren_v3_calculate_crc(report);

    return hid_send_feature_report(dev, (unsigned char*)report, sizeof(*report));
}

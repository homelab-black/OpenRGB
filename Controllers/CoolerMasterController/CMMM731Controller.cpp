/*-------------------------------------------------------------------*\
| CMMM731Controller.cpp                                               |
|                                                                     |
|   Driver for the Cooler Master MM731 Mouse                          |
|                                                                     |
|   Chris M (Dr_No)                              1 Jan 2022           |
|                                                                     |
|   This file is part of the OpenRGB project                          |
|   SPDX-License-Identifier: GPL-2.0-only                             |
\*-------------------------------------------------------------------*/

#include <cstring>
#include "CMMM731Controller.h"
#include "StringUtils.h"

CMMM731Controller::CMMM731Controller(hid_device* dev_handle, char *_path)
{
    dev                 = dev_handle;
    location            = _path;
    current_speed       = CM_MM731_SPEED_NORMAL;

    /*---------------------------------------------------------*\
    | Get device name from HID manufacturer and product strings |
    \*---------------------------------------------------------*/
    wchar_t name_string[HID_MAX_STR];

    hid_get_manufacturer_string(dev, name_string, HID_MAX_STR);
    device_name = StringUtils::wstring_to_string(name_string);

    hid_get_product_string(dev, name_string, HID_MAX_STR);
    device_name.append(" ").append(StringUtils::wstring_to_string(name_string));

    hid_get_indexed_string(dev, 2, name_string, HID_MAX_STR);
    serial = StringUtils::wstring_to_string(name_string);

    SendInitPacket();
}

CMMM731Controller::~CMMM731Controller()
{
    hid_close(dev);
}

/*void CMMM731Controller::GetColourStatus()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x52, 0x2B };

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);

    current_brightness      = buffer[CM_MM731_BRIGHTNESS_BYTE - 1];
    current_red             = buffer[CM_MM731_RED_BYTE - 1];
    current_green           = buffer[CM_MM731_GREEN_BYTE - 1];
    current_blue            = buffer[CM_MM731_BLUE_BYTE - 1];
}*/

/*void CMMM731Controller::GetCustomStatus()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x52, 0xA8 };
    int read_size                           = CM_MM731_PACKET_SIZE - 1;
    int result                              = 0;

    hid_write(dev, buffer, CM_MM711_PACKET_SIZE);
    do
    {
        result = hid_read_timeout(dev, buffer, read_size, CM_MM731_INTERRUPT_TIMEOUT);
    }while(buffer[1] != 0xA8 && result == read_size);

    if(result == read_size)
    {
        wheel_colour            = ToRGBColor(buffer[4], buffer[5], buffer[6]);
        logo_colour             = ToRGBColor(buffer[7], buffer[8], buffer[9]);
    }
}*/

std::string CMMM731Controller::GetDeviceName()
{
    return device_name;
}

std::string CMMM731Controller::GetSerial()
{
    return(serial);
}

std::string CMMM731Controller::GetLocation()
{
    return("HID: " + location);
}

uint8_t CMMM731Controller::GetMode()
{
    return(current_mode);
}

uint8_t CMMM731Controller::GetLedRed()
{
    return(current_red);
}

uint8_t CMMM731Controller::GetLedGreen()
{
    return(current_green);
}

uint8_t CMMM731Controller::GetLedBlue()
{
    return(current_blue);
}

uint8_t CMMM731Controller::GetLedSpeed()
{
    return(current_speed);
}

uint8_t CMMM731Controller::GetBrightness()
{
    return(current_brightness);
}

bool CMMM731Controller::GetRandomColours()
{
    return(bool_random);
}

void CMMM731Controller::SetMode(uint8_t mode, uint8_t speed, uint8_t brightness, RGBColor colour, bool random_colours)
{
    bool needs_update       = !( (current_mode == mode) && (current_speed == speed) && (current_brightness == brightness) && (ToRGBColor(current_red, current_green, current_blue) == colour));

    if (needs_update)
    {
        current_mode        = mode;
        current_speed       = speed;
        current_brightness  = brightness;
        current_red         = RGBGetRValue(colour);
        current_green       = RGBGetGValue(colour);
        current_blue        = RGBGetBValue(colour);
        bool_random         = random_colours;
        SendUpdate();
    }
}

void  CMMM731Controller::SetLedsDirect(RGBColor logo_colour)
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x4C, 0x81, 0x04, CM_MM731_MODE_STATIC, CM_MM731_BRIGHTNESS_MAX, CM_MM731_BRIGHTNESS_MAX, CM_MM731_SPEED_NORMAL };

    buffer[CM_MM731_RED_BYTE]               = RGBGetRValue(logo_colour);
    buffer[CM_MM731_GREEN_BYTE]             = RGBGetGValue(logo_colour);
    buffer[CM_MM731_BLUE_BYTE]              = RGBGetBValue(logo_colour);
    buffer[CM_MM731_NFI_2]                  = CM_MM731_BRIGHTNESS_MAX;

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);
}

void CMMM731Controller::GetStatus()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x52, 0x28 };

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);

    current_mode                            = buffer[CM_MM731_MODE_BYTE - 1];
}

void CMMM731Controller::SendUpdate()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x4C, 0x81, 0x04 };

    buffer[CM_MM731_MODE_BYTE]              = current_mode;
    buffer[CM_MM731_NFI_1]                  = CM_MM731_BRIGHTNESS_MAX;
    buffer[CM_MM731_BRIGHTNESS_BYTE]        = current_brightness;
    buffer[CM_MM731_SPEED_BYTE]             = current_speed;
    buffer[CM_MM731_RED_BYTE]               = current_red;
    buffer[CM_MM731_GREEN_BYTE]             = current_green;
    buffer[CM_MM731_BLUE_BYTE]              = current_blue;
    buffer[CM_MM731_NFI_2]                  = CM_MM731_BRIGHTNESS_MAX;

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);

    SendApplyPacket();
}

void CMMM731Controller::SendApplyPacket()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x4C, 0x81, 0x08, current_mode };

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);
}

void CMMM731Controller::SendInitPacket()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x44, 0x81, 0x02 };

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);

}

void CMMM731Controller::SendSavePacket()
{
    uint8_t buffer[CM_MM731_PACKET_SIZE]    = { 0x00, 0x54, 0x81, 0x01 };

    hid_write(dev, buffer, CM_MM731_PACKET_SIZE);
    hid_read_timeout(dev, buffer, CM_MM731_PACKET_SIZE, CM_MM731_INTERRUPT_TIMEOUT);
}


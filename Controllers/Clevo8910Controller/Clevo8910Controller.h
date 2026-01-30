
/*---------------------------------------------------------*\
|   Clevo8910Controller.h				    |
|                                                           |
|   Driver for Clevo Color per key keyboard                 |
|                                                           |
|   EzyUser     9 Jan 2026                                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"
#include "Clevo8910Keys.h"

#define CLEVO8910_CLASS_BYTE                 0xCC
#define CLEVO8910_DIRECT_CMD                 0x01
#define CLEVO8910_DIRECT_BRIGHTNESS_MIN      0
#define CLEVO8910_DIRECT_BRIGHTNESS_MAX      10
#define CLEVO8910_DIRECT_BRIGHTNESS_DEFAULT  10
#define CLEVO8910_DIRECT_SPEED_MIN           0
#define CLEVO8910_DIRECT_SPEED_MAX           2
#define CLEVO8910_DIRECT_SPEED_DEFAULT       1
#define CLEVO8910_BRT_SPD_CMD                0x09
#define CLEVO8910_OFF_CMD                    0x0C
#define CLEVO8910_BIOS_CMD		     0x20

enum
{
    CLEVO8910_COMPLEX_MODE         = (1<<8),
    CLEVO8910_MODE_DIRECT          = CLEVO8910_COMPLEX_MODE+1,
    CLEVO8910_MODE_RIPPLE          = CLEVO8910_COMPLEX_MODE+7,
    CLEVO8910_MODE_FADE            = CLEVO8910_COMPLEX_MODE+8,
    CLEVO8910_MODE_BREATHE         = CLEVO8910_COMPLEX_MODE+10,
    CLEVO8910_MODE_FLASHING        = CLEVO8910_COMPLEX_MODE+11,
};

enum
{
    CLEVO8910_TYPE_BLINK           = 1,
    CLEVO8910_TYPE_CYCLE           = 2,
    CLEVO8910_TYPE_STOP            = 3,
    CLEVO8910_TYPE_WAVE            = 4,
    CLEVO8910_TYPE_NOOP            = 5,
    CLEVO8910_TYPE_RANDOM          = 9,
    CLEVO8910_TYPE_SCAN            = 10,
    CLEVO8910_TYPE_SNAKE           = 11,
    CLEVO8910_TYPE_OFF             = 12
};

class Clevo8910Controller
{
    public:
        Clevo8910Controller(hid_device* dev_handle, const hid_device_info& info, const unsigned short pid, std::string dev_name);
        ~Clevo8910Controller();

        std::string 	GetDeviceLocation();
        std::string 	GetDeviceName();
        uint16_t    	GetUSBPID();
        void        	TurnOff();
	void	    	SaveBios(bool save);
	void	    	SetParams(uint8_t brightness, uint8_t speed);
        void	    	SetSimpleMode(uint8_t effect, uint8_t bright, uint8_t speed);
	void	    	SetMode(uint8_t effect, uint8_t brightness, uint8_t speed, uint8_t key, RGBColor color );
	void	    	SetColors(uint8_t mode, std::vector<led> array_leds, bool rnd_flag, std::vector<RGBColor> array_colors);
	void	    	SetSingleLED(uint8_t mode, uint8_t key, RGBColor color);

    private:
        hid_device*	dev;
        std::string	location;
        std::string	name;
        //std::string	serial_number;
        uint16_t   	usb_pid;
};

typedef	struct
	{
	    uint8_t	header;
	    uint8_t	mode;
	    uint8_t	param;
	    uint8_t	red;
	    uint8_t	greem;
	    uint8_t	blue;
	    uint8_t	end;
	} report_buf_t;
#define CLEVO8910_REPORT_SIZE	(7)

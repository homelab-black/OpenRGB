/*---------------------------------------------------------*\
| YunziiController.cpp                                      |
|                                                           |
|   Driver for Yunzii keyboards                             |
|                                                           |
|   Kosta A (kostaarvanitis)                    23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "YunziiController.h"

YunziiController::YunziiController(hid_device* dev_handle, hid_device_info* dev_info, std::string dev_name) :
    RoyuanController(dev_handle, dev_info, dev_name)
{
}

unsigned char YunziiController::Crc(const unsigned char data[], unsigned int start, unsigned int end)
{
    unsigned int crc = 0;

    for(unsigned int i = start; i < end; ++i)
    {
        crc += data[i];
    }

    crc &= 0xFF;
    crc = (255 - crc);

    return (crc & 0xFF);
}

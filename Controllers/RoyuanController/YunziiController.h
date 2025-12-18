/*---------------------------------------------------------*\
| YunziiController.h                                        |
|                                                           |
|   Driver for Yunzii keyboards                             |
|                                                           |
|   Kosta A (kostaarvanitis)                    23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RoyuanController.h"

class YunziiController : public RoyuanController
{
public:
    YunziiController(hid_device* dev_handle, hid_device_info* dev_info, std::string dev_name);

protected:
    virtual unsigned char Crc(const unsigned char data[], unsigned int start, unsigned int end);
};
/*---------------------------------------------------------*\
| RGBController_YunziiController.h                          |
|                                                           |
|   Driver for Yunzii keyboards                             |
|                                                           |
|   Kosta A (kostaarvanitis)                    23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "YunziiController.h"
#include "RGBController_RoyuanController.h"

class RGBController_YunziiController : public RGBController_RoyuanController
{
public:
    RGBController_YunziiController(YunziiController* controller_ptr);
};

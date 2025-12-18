/*---------------------------------------------------------*\
| RGBController_YunziiController.cpp                        |
|                                                           |
|   Driver for Yunzii keyboards                             |
|                                                           |
|   Kosta A (kostaarvanitis)                    23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_YunziiController.h"

/**------------------------------------------------------------------*\
    @name Yunzii YZ75
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectYunziiControllers
    @comment
    The Yunzii keyboards use a similar if not same micro controller
    as epomaker and other brands. For the few subtle differences, we 
    subclass the Royuan controllers to accommodate these specific 
    changes, otherwise they remain functionally equivalent.
\*-------------------------------------------------------------------*/

RGBController_YunziiController::RGBController_YunziiController(YunziiController* controller_ptr) :
    RGBController_RoyuanController(controller_ptr)
{
    for(int mode_index = 0; mode_index < modes.size(); mode_index++)
    {
        // Invert speed values
        unsigned int tmp = modes[mode_index].speed_min;
        modes[mode_index].speed_min = modes[mode_index].speed_max;
        modes[mode_index].speed_max = tmp;
        modes[mode_index].speed = modes[mode_index].speed_max;
    }
}

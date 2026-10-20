/*---------------------------------------------------------*\
| RGBController_ColorfulMBoard.h                           |
|                                                           |
|   RGBController for Colorful USB motherboard lighting     |
|                                                           |
|   OpenRGB contributors                       08 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "ColorfulMBoardController.h"
#include "RGBController.h"

/*---------------------------------------------------------*\
| Mode values                                               |
\*---------------------------------------------------------*/
#define COLORFUL_MBOARD_MODE_DIRECT         0
#define COLORFUL_MBOARD_MODE_BREATHING      1
#define COLORFUL_MBOARD_MODE_COLOR_CYCLE    2
#define COLORFUL_MBOARD_MODE_RAINBOW        3
#define COLORFUL_MBOARD_MODE_OFF            4

/*---------------------------------------------------------*\
| Speed range exposed for native modes                      |
\*---------------------------------------------------------*/
#define COLORFUL_MBOARD_MODE_SPEED_MIN      1
#define COLORFUL_MBOARD_MODE_SPEED_MAX      5

class RGBController_ColorfulMBoard : public RGBController
{
public:
    RGBController_ColorfulMBoard(ColorfulMBoardController* controller_ptr);
    ~RGBController_ColorfulMBoard();

    void SetupZones();

    void DeviceConfigureZone(int zone_idx);
    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    ColorfulMBoardController* controller;
};

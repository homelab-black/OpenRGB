
/*---------------------------------------------------------*\
 |   RGBController_Clevo8910.h				   |
 |                                                         |
 |   Driver for Clevo Color per key keyboard               |
 |                                                         |
 |   EzyUser     9 Jan 2026                                |
 |                                                         |
 |   This file is part of the OpenRGB project              |
 |   SPDX-License-Identifier: GPL-2.0-or-later             |
\*---------------------------------------------------------*/

/*
 *
 *    RGBController for Clevo RGB colour per key keyboard
 *    
 *    Ezy  9 Jan 202r65
 *                                                            |
 *    This file is part of the OpenRGB project
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "RGBController.h"
#include "Clevo8910Controller.h"


class RGBController_Clevo8910 : public RGBController
{
public:
    RGBController_Clevo8910(Clevo8910Controller* controller_ptr);
    ~RGBController_Clevo8910();

    void SetupZones();
    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();
    void DeviceSaveMode();


private:
    Clevo8910Controller* controller;
};

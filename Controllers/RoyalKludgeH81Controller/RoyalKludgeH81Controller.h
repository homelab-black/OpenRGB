/*---------------------------------------------------------*\
| RoyalKludgeH81Controller.h                                |
|                                                           |
|   Driver for Royal Kludge RK-H81 keyboard                 |
|                                                           |
|   eclecticBelletrist                          04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

/*-----------------------------------------------------*\
| Royal Kludge / BY Tech vendor ID                       |
\*-----------------------------------------------------*/
#define ROYALKLUDGE_VID             0x258A

/*-----------------------------------------------------*\
| keyboard product IDs                                   |
\*-----------------------------------------------------*/
#define ROYALKLUDGE_H81_PID         0x0171

/*-----------------------------------------------------*\
| number of per-key LED slots reserved in the protocol   |
| buffer.  only the first RKH81_LED_COUNT slots address  |
| a physical LED on the H81, see PROTOCOL_DOSSIER.md for |
| what the rest of the buffer is                         |
\*-----------------------------------------------------*/
#define RKH81_LED_COUNT             80

/*-----------------------------------------------------*\
| SET_FEATURE report size on report ID 6                 |
\*-----------------------------------------------------*/
#define RKH81_REPORT_SIZE           520

class RoyalKludgeH81Controller
{
public:
    RoyalKludgeH81Controller(hid_device* dev_handle, char* path);
    ~RoyalKludgeH81Controller();

    RoyalKludgeH81Controller(const RoyalKludgeH81Controller&)            = delete;
    RoyalKludgeH81Controller& operator=(const RoyalKludgeH81Controller&) = delete;

    std::string     GetLocation();
    std::string     GetSerialString();
    const std::string& GetPath();

    bool            SetLEDsDirect(std::vector<RGBColor> colors);
    bool            SetLightsOff();

private:
    hid_device*     dev;
    std::string     location;
};

/*-----------------------------------------------------*\
| detector-side claim table lives in                     |
| RoyalKludgeH81ControllerDetect.cpp; the RGBController   |
| calls this on teardown so a replug of the same hidraw   |
| path is not permanently locked out                      |
\*-----------------------------------------------------*/
void RKH81UnclaimPath(const std::string& path);

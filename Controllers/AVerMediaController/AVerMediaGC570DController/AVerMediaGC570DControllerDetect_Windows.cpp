/*---------------------------------------------------------*\
| AVerMediaGC570DControllerDetect_Windows.cpp               |
|                                                           |
|   Detector for AVerMedia Live Gamer DUO (GC570D)          |
|                                                           |
|   Niaz (derneuere)                             24 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "LogManager.h"
#include "AVerMediaGC570DController_Windows.h"
#include "RGBController_AVerMediaGC570D_Windows.h"

DetectedControllers DetectAVerMediaGC570DControllers()
{
    DetectedControllers detected_controllers;

    AVerMediaGC570DController* controller = new AVerMediaGC570DController();

    if(controller->Initialize())
    {
        RGBController_AVerMediaGC570D* rgb_controller = new RGBController_AVerMediaGC570D(controller);

        detected_controllers.push_back(rgb_controller);
    }
    else
    {
        LOG_DEBUG("[AVerMedia Live Gamer DUO] Device initialization failed");

        delete controller;
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("AVerMedia Live Gamer DUO", DetectAVerMediaGC570DControllers);

/*---------------------------------------------------------*\
| SkydimoSerialControllerDetect.cpp                        |
|                                                           |
|   Detector for Skydimo serial devices                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <set>
#include <vector>
#include "Detector.h"
#include "RGBController_SkydimoSerial.h"
#include "SkydimoSerialController.h"
#include "find_usb_serial_port.h"

#define SKYDIMO_SERIAL_VID 0x1A86
#define SKYDIMO_SERIAL_PID 0x7523

void DetectSkydimoSerialControllers()
{
    std::vector<std::string*> ports = find_usb_serial_port(SKYDIMO_SERIAL_VID, SKYDIMO_SERIAL_PID);
    std::set<std::string> seen_ports;

    for(unsigned int port_idx = 0; port_idx < ports.size(); port_idx++)
    {
        std::string port = *ports[port_idx];

        if(seen_ports.insert(port).second)
        {
            SkydimoSerialController* controller = new SkydimoSerialController(port);

            if(controller->IsPresent())
            {
                RGBController_SkydimoSerial* rgb_controller = new RGBController_SkydimoSerial(controller);
                ResourceManager::get()->RegisterRGBController(rgb_controller);
            }
            else
            {
                delete controller;
            }
        }

        delete ports[port_idx];
    }
}

REGISTER_DETECTOR("Skydimo Serial", DetectSkydimoSerialControllers);
/*---------------------------------------------------------------------------------------------------------*\
| Entries for dynamic UDEV rules                                                                            |
|                                                                                                           |
| DUMMY_DEVICE_DETECTOR("Skydimo Serial", DetectSkydimoSerialControllers, 0x1A86, 0x7523 )                  |
\*---------------------------------------------------------------------------------------------------------*/

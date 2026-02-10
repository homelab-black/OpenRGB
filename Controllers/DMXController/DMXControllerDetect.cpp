/*---------------------------------------------------------*\
| DMXControllerDetect.cpp                                   |
|                                                           |
|   Detector for DMX devices                                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <fcntl.h>
#include <string>
#include <vector>
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController_DMX.h"
#include "SettingsManager.h"

DetectedControllers DetectDMXControllers()
{
    DetectedControllers detected_controllers;
    std::vector<std::vector<DMXDevice>> device_lists;

    /*-----------------------------------------------------*\
    | Get DMX settings from settings manager                |
    \*-----------------------------------------------------*/
    std::optional<DMXSettings> cfg_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettings<DMXSettings>(DMX_CFG_KEY);
    DMXSettings dmx_settings = cfg_settings.value_or(DMXSettings {});

    /*-----------------------------------------------------*\
    | If the DMX settings contains devices, process         |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < dmx_settings.devices.size(); device_idx++)
    {
        DMXDevice dev = dmx_settings.devices[device_idx];

        /*---------------------------------------------*\
        | Determine whether to create a new list or add |
        | this device to an existing list.  A device is |
        | added to an existing list if both devices     |
        | share one or more universes for the same      |
        | output destination                            |
        \*---------------------------------------------*/
        bool device_added_to_existing_list = false;

        /*---------------------------------------------*\
        | Track grouping for all controllers.           |
        \*---------------------------------------------*/
        for(unsigned int list_idx = 0; list_idx < device_lists.size(); list_idx++)
        {
            for(unsigned int device_idx = 0; device_idx < device_lists[list_idx].size(); device_idx++)
            {
                /*-------------------------------------*\
                | Check if the port used by this new    |
                | device is the same as in the existing |
                | device.  If so, add the new device to |
                | the existing list.                    |
                \*-------------------------------------*/
                if(1)
                {
                    device_lists[list_idx].push_back(dev);
                    device_added_to_existing_list = true;
                    break;
                }
            }

            if(device_added_to_existing_list)
            {
                break;
            }
        }

        /*---------------------------------------------------------*\
        | If the device did not overlap with existing devices,      |
        | create a new list for it                                  |
        \*---------------------------------------------------------*/
        if(!device_added_to_existing_list)
        {
            std::vector<DMXDevice> new_list;

            new_list.push_back(dev);

            device_lists.push_back(new_list);
        }
    }

    for(unsigned int list_idx = 0; list_idx < device_lists.size(); list_idx++)
    {
        RGBController_DMX* rgb_controller;
        rgb_controller = new RGBController_DMX(device_lists[list_idx]);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("DMX", DetectDMXControllers);

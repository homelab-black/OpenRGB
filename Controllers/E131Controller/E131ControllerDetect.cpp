/*---------------------------------------------------------*\
| E131ControllerDetect.cpp                                  |
|                                                           |
|   Detector for E1.31 devices                              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <fcntl.h>
#include <string>
#include <vector>
#include "DetectionManager.h"
#include "ResourceManager.h"
#include "RGBController.h"
#include "RGBController_E131.h"
#include "SettingsManager.h"

DetectedControllers DetectE131Controllers()
{
    DetectedControllers detected_controllers;
    std::vector<std::vector<E131Device>> device_lists;

    /*-----------------------------------------------------*\
    | Get E1.31 settings from settings manager              |
    \*-----------------------------------------------------*/
    E131Settings e131_settings = ResourceManager::get()->GetSettingsManager()->GetControllerSettingsOrDefault<E131Settings>(E131_SETTINGS_KEY);

    /*-----------------------------------------------------*\
    | If the E1.31 settings contains devices, process       |
    \*-----------------------------------------------------*/
    for(unsigned int device_idx = 0; device_idx < e131_settings.devices.size(); device_idx++)
    {
        E131Device dev = e131_settings.devices[device_idx];

        /*---------------------------------------------*\
        | Determine whether to create a new list or add |
        | this device to an existing list.  A device is |
        | added to an existing list if both devices     |
        | share one or more universes for the same      |
        |output destination                             |
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
                | Determine if there is any overlap     |
                | between this device and any existing  |
                | device list                           |
                | Offset the end by two - one because   |
                | the range is 1-512 rather than 0-511, |
                | and one because the start channel is  |
                | included in the first set of 3        |
                | channels.                             |
                \*-------------------------------------*/
                unsigned int dev_start  = dev.start_universe;
                unsigned int list_start = device_lists[list_idx][device_idx].start_universe;
                unsigned int dev_end    = dev.start_universe + ((dev.start_channel + (3 * dev.num_leds) - 2) / 512);
                unsigned int list_end   = device_lists[list_idx][device_idx].start_universe + ((device_lists[list_idx][device_idx].start_channel + (3 * device_lists[list_idx][device_idx].num_leds) - 2) / 512);
                std::string  dev_ip     = dev.ip;
                std::string  list_ip    = device_lists[list_idx][device_idx].ip;

                bool overlap = dev_ip == list_ip && !(dev_end < list_start || list_end < dev_start);

                /*-------------------------------------*\
                | Check if any universes used by this   |
                | new device exist in the existing      |
                | device.  If so, add the new device to |
                | the existing list.                    |
                \*-------------------------------------*/
                if(overlap)
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

        /*---------------------------------------------*\
        | If the device did not overlap with existing   |
        | devices, create a new list for it             |
        \*---------------------------------------------*/
        if(!device_added_to_existing_list)
        {
            std::vector<E131Device> new_list;

            new_list.push_back(dev);

            device_lists.push_back(new_list);
        }
    }

    for(unsigned int list_idx = 0; list_idx < device_lists.size(); list_idx++)
    {
        RGBController_E131* rgb_controller;
        rgb_controller = new RGBController_E131(device_lists[list_idx]);

        detected_controllers.push_back(rgb_controller);
    }

    return(detected_controllers);
}

REGISTER_DETECTOR("E1.31", DetectE131Controllers);

/*---------------------------------------------------------*\
| CreativeControllerDetect.cpp                              |
|                                                           |
|   Detector for Creative devices                           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "CreativeSoundBlasterXG6Controller.h"
#include "RGBController_CreativeSoundBlasterXG6.h"
#include "CreativePebbleXController.h"
#include "RGBController_CreativePebbleX.h"
#include "Detector.h"

/*-----------------------------------------------------*\
| Creative vendor ID                                    |
\*-----------------------------------------------------*/
#define CREATIVE_VID                            0x041E
/*-----------------------------------------------------*\
| SoundCards                                            |
\*-----------------------------------------------------*/
#define CREATIVE_SOUNDBLASTERX_G6_PID           0x3256
/*-----------------------------------------------------*\
| Speakers                                              |
\*-----------------------------------------------------*/
#define CREATIVE_PEBBLE_X_PID                   0x3299

void DetectCreativeDevice(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        CreativeSoundBlasterXG6Controller*     controller     = new CreativeSoundBlasterXG6Controller(dev, info->path, name);
        RGBController_CreativeSoundBlasterXG6* rgb_controller = new RGBController_CreativeSoundBlasterXG6(controller);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

void DetectCreativePebbleXDevice(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);

    if(dev)
    {
        CreativePebbleXController*     controller     = new CreativePebbleXController(dev, info->path);
        RGBController_CreativePebbleX* rgb_controller = new RGBController_CreativePebbleX(controller);
        rgb_controller->name = name;
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------------------*\
| Sound Cards                                                                                                                                       |
\*-------------------------------------------------------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("Creative SoundBlasterX G6",               DetectCreativeDevice,         CREATIVE_VID,     CREATIVE_SOUNDBLASTERX_G6_PID, 4);

/*-------------------------------------------------------------------------------------------------------------------------------------------------*\
| Speakers                                                                                                                                          |
\*-------------------------------------------------------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Creative Pebble X", DetectCreativePebbleXDevice, CREATIVE_VID, CREATIVE_PEBBLE_X_PID, 0, 0xFF01, 1);

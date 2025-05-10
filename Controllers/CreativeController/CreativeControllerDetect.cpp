/*---------------------------------------------------------*\
| CreativeControllerDetect.cpp                              |
|                                                           |
|   Detector for Creative devices                           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/


#include <hidapi.h>
#include "CreativeSoundBlasterXG6Controller.h"
#include "RGBController_CreativeSoundBlasterXG6.h"
#include "Detector.h"
#include "LogManager.h"


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
        CreativeSoundBlasterXG6Controller*     controller     = new CreativeSoundBlasterXG6Controller(dev, info->path);
        RGBController_CreativeSoundBlasterXG6* rgb_controller = new RGBController_CreativeSoundBlasterXG6(controller);
        rgb_controller->name = name;
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}


void DetectCreativePebbleDevice(hid_device_info* info, const std::string& name)
{

    hid_device* dev = hid_open_path(info->path);
    // LOG_DEBUG("[%s] at address 0x%02X invalid. Expected 0xAB [0x*] but received:%s", 'GIGABYTEGPU_CONTROLLER_NAME2', address, text.c_str());
    LOG_DEBUG("BEFORE IFELSE STATAEMNET");

    if(dev)
    {
        LOG_DEBUG("MADE IT HERE DEV IS TRUE");
        CreativePebbleXController*     controller     = new CreativePebbleXController(dev, info->path);
        RGBController_CreativePebbleX* rgb_controller = new RGBController_CreativePebbleX(controller);
        rgb_controller->name = name;
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
    else
    {
        LOG_DEBUG("'DIDNT MAKE IT HERE DEV IS FALSE'");
    }
}



/*-------------------------------------------------------------------------------------------------------------------------------------------------*\
| Sound Cards                                                                                                                                       |
\*-------------------------------------------------------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("Creative SoundBlasterX G6",               DetectCreativeDevice,         CREATIVE_VID,     CREATIVE_SOUNDBLASTERX_G6_PID, 4);

/*-------------------------------------------------------------------------------------------------------------------------------------------------*\
| Speakers                                                                                                                                          |
\*-------------------------------------------------------------------------------------------------------------------------------------------------*/
REGISTER_HID_DETECTOR_I("Creative Pebble X", DetectCreativePebbleDevice, CREATIVE_VID, CREATIVE_PEBBLE_X_PID, 3);

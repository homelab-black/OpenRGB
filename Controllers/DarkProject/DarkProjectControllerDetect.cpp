/*---------------------------------------------------------*\
| DarkProjectKD87ADetect.cpp                                |
|                                                           |
|   Detector for Dark Project KD87A Skeleton keyboard       |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "RGBController_KD87A.h"
#include "DarkProjectKD87AController.h"

#define DARKPROJECT_KD87A_VID 0x1A2C
#define DARKPROJECT_KD87A_PID 0x1514

void DetectDarkProjectKD87AControllers(hid_device_info* info, const std::string& name)
{
    hid_device* dev = hid_open_path(info->path);
    if (dev)
    {
        DarkProjectKD87AController* controller = new DarkProjectKD87AController(dev, info->path, name);
        RGBController_KD87A* rgb_controller = new RGBController_KD87A(controller);
        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

REGISTER_HID_DETECTOR("Dark Project KD87A Skeleton", DetectDarkProjectKD87AControllers, DARKPROJECT_KD87A_VID, DARKPROJECT_KD87A_PID);

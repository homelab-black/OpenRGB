/*---------------------------------------------------------*\
| SamsungSSDControllerDetect_Linux.cpp                      |
|                                                           |
|   Detector for Samsung SSD heatsink LED (Linux)           |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <unistd.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController_SamsungSSD_Windows_Linux.h"
#include "SamsungSSDController_Windows_Linux.h"

#define NVME_CLASS_PATH                 "/sys/class/nvme"

static std::string ReadSysfsString(const std::string& path)
{
    std::ifstream   file(path);
    std::string     value;

    std::getline(file, value);

    value.erase(value.find_last_not_of(' ') + 1);
    value.erase(0, value.find_first_not_of(' '));

    return(value);
}

DetectedControllers DetectSamsungSSDControllers()
{
    DetectedControllers detected_controllers;
    DIR*                nvme_dir = opendir(NVME_CLASS_PATH);

    if(nvme_dir == NULL)
    {
        return(detected_controllers);
    }

    struct dirent* entry;

    while((entry = readdir(nvme_dir)) != NULL)
    {
        /*-------------------------------------------------*\
        | Use only controller nodes such as nvme0, not      |
        | namespace nodes such as nvme0n1                   |
        \*-------------------------------------------------*/
        std::string name = entry->d_name;

        if(name.compare(0, 4, "nvme") != 0 || name.size() == 4 || name.find_first_not_of("0123456789", 4) != std::string::npos)
        {
            continue;
        }

        std::string sysfs_path  = std::string(NVME_CLASS_PATH) + "/" + name;
        std::string model       = ReadSysfsString(sysfs_path + "/model");

        LOG_DEBUG("[Samsung SSD] Probing %s, model: %s", name.c_str(), model.c_str());

        if(model.compare(0, strlen(SAMSUNG_SSD_990_PRO_HEATSINK), SAMSUNG_SSD_990_PRO_HEATSINK) != 0)
        {
            continue;
        }

        std::string dev_path    = "/dev/" + name;
        int         dev_handle  = open(dev_path.c_str(), O_RDWR);

        if(dev_handle < 0)
        {
            LOG_WARNING("[Samsung SSD] Cannot open %s (%s), root rights are necessary", dev_path.c_str(), strerror(errno));
            continue;
        }

        std::string serial      = ReadSysfsString(sysfs_path + "/serial");
        std::string firmware    = ReadSysfsString(sysfs_path + "/firmware_rev");

        SamsungSSDController*     controller     = new SamsungSSDController(dev_handle, dev_path, model, serial, firmware);
        RGBController_SamsungSSD* rgb_controller = new RGBController_SamsungSSD(controller);

        detected_controllers.push_back(rgb_controller);
    }

    closedir(nvme_dir);

    return(detected_controllers);
}

REGISTER_DETECTOR("Samsung SSD", DetectSamsungSSDControllers);

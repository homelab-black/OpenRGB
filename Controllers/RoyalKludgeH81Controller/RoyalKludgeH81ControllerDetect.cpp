/*---------------------------------------------------------*\
| RoyalKludgeH81ControllerDetect.cpp                        |
|                                                           |
|   Detector for Royal Kludge RK-H81 keyboard               |
|                                                           |
|   eclecticBelletrist                          04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <mutex>
#include <set>
#include <string>
#include <hidapi.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "RoyalKludgeH81Controller.h"
#include "RGBController_RoyalKludgeH81.h"

/*---------------------------------------------------------------*\
| interface 1 exposes three top-level HID collections (report IDs |
| 3, 5, 6) sharing one usage_page/usage, so hidapi enumerates this |
| detector three times for the same path.  claim the path so only |
| the first call builds a controller.  guarded by a mutex since   |
| a manual Rescan and a hotplug arrival/removal can land on       |
| different threads                                                |
\*---------------------------------------------------------------*/
static std::set<std::string> rkh81_claimed_paths;
static std::mutex            rkh81_claimed_paths_mutex;

static bool RKH81ClaimPath(const std::string& path)
{
    std::lock_guard<std::mutex> lock(rkh81_claimed_paths_mutex);
    return(rkh81_claimed_paths.insert(path).second);
}

void RKH81UnclaimPath(const std::string& path)
{
    std::lock_guard<std::mutex> lock(rkh81_claimed_paths_mutex);
    rkh81_claimed_paths.erase(path);
}

static void RKH81PreDetectionReset()
{
    std::lock_guard<std::mutex> lock(rkh81_claimed_paths_mutex);
    rkh81_claimed_paths.clear();
}
REGISTER_PRE_DETECTION_HOOK(RKH81PreDetectionReset);

/*---------------------------------------------------------------*\
| VID 0x258A is shared by unrelated vendors/chipsets (see          |
| SinowealthControllerDetect.cpp).  restricted to interface 1,     |
| usage page/usage of the H81's lighting endpoint                  |
\*---------------------------------------------------------------*/
DetectedControllers DetectRoyalKludgeH81Controllers(hid_device_info* info, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    hid_device*         dev;

    if(!RKH81ClaimPath(info->path))
    {
        return(detected_controllers);
    }

    dev = hid_open_path(info->path);

    if(!dev)
    {
        LOG_DEBUG("[Royal Kludge H81] hid_open_path failed for %s", info->path);
        return(detected_controllers);
    }

    RoyalKludgeH81Controller* controller = new RoyalKludgeH81Controller(dev, info->path);

    // probe with an all-black direct frame before trusting the device
    std::vector<RGBColor> probe_colors(RKH81_LED_COUNT, 0x00000000);

    if(controller->SetLEDsDirect(probe_colors))
    {
        try
        {
            RGBController_RoyalKludgeH81* rgb_controller = new RGBController_RoyalKludgeH81(controller);
            detected_controllers.push_back(rgb_controller);

            LOG_INFO("[Royal Kludge H81] detected at %s", info->path);
        }
        catch(...)
        {
            LOG_WARNING("[Royal Kludge H81] failed to build RGBController for %s", info->path);
            delete controller;
            RKH81UnclaimPath(info->path);
        }
    }
    else
    {
        LOG_DEBUG("[Royal Kludge H81] probe frame rejected by %s", info->path);
        delete controller;
        RKH81UnclaimPath(info->path);
    }

    return(detected_controllers);
}

REGISTER_HID_DETECTOR_IPU("Royal Kludge H81", DetectRoyalKludgeH81Controllers, ROYALKLUDGE_VID, ROYALKLUDGE_H81_PID, 1, 0xFF00, 1);

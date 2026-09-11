/*---------------------------------------------------------*\
| OnixLumiGPUControllerDetect.cpp                           |
|                                                           |
|   Detector for ONIX LUMI Intel Arc B580                   |
|                                                           |
|   Jan Dvořák                                  11 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include <vector>

#include "DetectionManager.h"
#include "LogManager.h"
#include "OnixLumiGPUController.h"
#include "RGBController_OnixLumiGPU.h"

#define ONIX_LUMI_DETECTOR_NAME "ONIX LUMI Intel Arc B580"
#define ONIX_LUMI_I2C_ADDRESS   0x28

#ifdef __linux__

#include <cstdio>
#include <fstream>

#include "filesystem.h"

namespace fs = filesystem;

static bool ReadHexFile(const fs::path& path, unsigned int& value)
{
    std::ifstream file(path);

    if(!file.is_open())
    {
        return(false);
    }

    std::string text;

    if(!(file >> text))
    {
        return(false);
    }

    try
    {
        value = std::stoul(text, nullptr, 0);
    }
    catch(...)
    {
        return(false);
    }

    return(true);
}

static bool IsOnixB580Ancestor(fs::path path)
{
    while(!path.empty())
    {
        unsigned int vendor;
        unsigned int device;
        unsigned int subsystem_vendor;
        unsigned int subsystem_device;

        if(ReadHexFile(path / "vendor", vendor) &&
           ReadHexFile(path / "device", device) &&
           ReadHexFile(path / "subsystem_vendor", subsystem_vendor) &&
           ReadHexFile(path / "subsystem_device", subsystem_device))
        {
            if(vendor           == 0x8086 &&
               device           == 0xE20B &&
               subsystem_vendor == 0x207E &&
               subsystem_device == 0xA002)
            {
                return(true);
            }
        }

        fs::path parent = path.parent_path();

        if(parent == path)
        {
            break;
        }

        path = parent;
    }

    return(false);
}

static bool IsOnixLumiBus(i2c_smbus_interface* bus)
{
    std::string device_name(bus->info.device_name);

    /*
     * Ignore GPU GMBUS/DDC/AUX adapters.  LUMI is attached to
     * the dedicated DesignWare I2C controller exposed by xe.
     */
    if(device_name.find("DesignWare I2C adapter") == std::string::npos)
    {
        return(false);
    }

    fs::path adapter =
        fs::path("/sys/bus/i2c/devices") /
        ("i2c-" + std::to_string(bus->info.bus_id));

    std::error_code error;
    fs::path real_adapter = fs::canonical(adapter, error);

    if(error)
    {
        return(false);
    }

    if(!IsOnixB580Ancestor(real_adapter))
    {
        return(false);
    }

    /*
     * The xe driver exposes the LUMI/board MCU as an I2C
     * client at address 0x28.  Requiring the sysfs client
     * avoids probing arbitrary DesignWare adapters.
     */
    char client_name[32];
    snprintf(
        client_name,
        sizeof(client_name),
        "%d-%04x",
        bus->info.bus_id,
        ONIX_LUMI_I2C_ADDRESS
    );

    fs::path client =
        fs::path("/sys/bus/i2c/devices") /
        client_name;

    return(fs::exists(client));
}

#endif /* __linux__ */

DetectedControllers DetectOnixLumiGPUControllers(
    std::vector<i2c_smbus_interface*>& buses)
{
    DetectedControllers detected_controllers;

#ifdef __linux__

    for(i2c_smbus_interface* bus : buses)
    {
        if(!IsOnixLumiBus(bus))
        {
            continue;
        }

        LOG_DEBUG(
            "[%s] Found controller on I2C bus %d at address 0x%02X",
            ONIX_LUMI_DETECTOR_NAME,
            bus->info.bus_id,
            ONIX_LUMI_I2C_ADDRESS
        );

        OnixLumiGPUController* controller =
            new OnixLumiGPUController(
                bus,
                ONIX_LUMI_I2C_ADDRESS,
                ONIX_LUMI_DETECTOR_NAME
            );

        RGBController_OnixLumiGPU* rgb_controller =
            new RGBController_OnixLumiGPU(controller);

        detected_controllers.push_back(rgb_controller);
    }

#else

    (void)buses;

#endif

    return(detected_controllers);
}

REGISTER_I2C_DETECTOR(
    ONIX_LUMI_DETECTOR_NAME,
    DetectOnixLumiGPUControllers
);

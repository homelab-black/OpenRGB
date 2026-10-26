/*---------------------------------------------------------*\
| AMDRadeonRX7900XTXControllerDetect_Linux.cpp              |
|                                                           |
|   Detector for AMD Radeon RX 7900 XTX reference RGB       |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "LogManager.h"
#include "pci_ids.h"
#include "RGBController_AMDRadeonRX7900XTX.h"

DetectedControllers DetectAMDRadeonRX7900XTXControllers(i2c_smbus_interface* bus, unsigned char /*address*/, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;

    if(!AMDRadeonRX7900XTXController::IsSupportedBus(bus))
    {
        return detected_controllers;
    }

    AMDRadeonRX7900XTXController* controller = new AMDRadeonRX7900XTXController(bus);

    if(controller->Detect())
    {
        LOG_INFO("[AMD RX 7900 XTX] Reference RGB controller identified on %s at 0x5A", bus->info.device_name);
        detected_controllers.push_back(new RGBController_AMDRadeonRX7900XTX(controller));
    }
    else
    {
        LOG_WARNING("[AMD RX 7900 XTX] RGB identification failed on %s at 0x5A; no lighting writes sent", bus->info.device_name);
        delete controller;
    }

    return detected_controllers;
}

/*---------------------------------------------------------*\
| Windows' regular ADL I2C backend is not the SMU I2C API   |
| used by this controller. Detection is Linux-only.         |
\*---------------------------------------------------------*/
REGISTER_I2C_PCI_DETECTOR("AMD Radeon RX 7900 XTX Reference", DetectAMDRadeonRX7900XTXControllers, AMD_GPU_VEN, AMD_NAVI31_DEV, AMD_GPU_VEN, AMD_RX7900XTX_REFERENCE_SUB_DEV, AMD_RX7900XTX_RGB_ADDR);

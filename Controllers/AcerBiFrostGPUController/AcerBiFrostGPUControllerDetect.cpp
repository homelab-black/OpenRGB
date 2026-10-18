/*---------------------------------------------------------*\
| AcerBiFrostGPUControllerDetect.cpp                        |
|                                                           |
|   Detector for Acer Predator BiFrost GPU                  |
|                                                           |
|   Baobaosheng                                 18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include "pci_ids.h"
#include "i2c_amd_gpu.h"
#include "AcerBiFrostGPUController.h"
#include "RGBController_AcerBiFrostGPU.h"

/*---------------------------------------------------------*\
| The MCU reports its lamp count in register 0x10. Only     |
| claim the device when it reports the 6 logo lamps         |
\*---------------------------------------------------------*/

DetectedControllers DetectAcerBiFrostGPUControllers(i2c_smbus_interface* bus, uint8_t i2c_addr, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(is_amd_gpu_i2c_bus(bus))
    {
        AcerBiFrostGPUController* controller = new AcerBiFrostGPUController(bus, i2c_addr, name);

        if(controller->GetLampCount() == ACER_BIFROST_NUM_LEDS)
        {
            RGBController_AcerBiFrostGPU* rgb_controller = new RGBController_AcerBiFrostGPU(controller);

            detected_controllers.push_back(rgb_controller);
        }
        else
        {
            delete controller;
        }
    }

    return(detected_controllers);
}

REGISTER_I2C_PCI_DETECTOR("Acer Predator BiFrost RX 9070 XT", DetectAcerBiFrostGPUControllers, AMD_GPU_VEN, AMD_NAVI48_DEV, ACER_SUB_VEN, ACER_BIFROST_RX9070XT_SUB_DEV, 0x68);

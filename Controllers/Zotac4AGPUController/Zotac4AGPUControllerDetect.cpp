/*---------------------------------------------------------*\
| Zotac4AGPUControllerDetect.cpp                            |
|                                                           |
|   Detector for Zotac 4A GPU                               |
|                                                           |
|   Peter Berendi                               28 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "DetectionManager.h"
#include <cstring>
#include "Zotac4AGPUController.h"
#include "RGBController_Zotac4AGPU.h"
#include "pci_ids.h"
#include "LogManager.h"

DetectedControllers DetectZotac4AGPUControllers(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& /*name*/)
{
    DetectedControllers detected_controllers;
    u8 data[I2C_SMBUS_BLOCK_MAX] = { 0 };
    s32 length = bus->i2c_smbus_read_i2c_block_data(i2c_addr, ZOTAC_4A_GPU_REG_RGB, sizeof(data), data);
    if(length != I2C_SMBUS_BLOCK_MAX)
    {
        LOG_ERROR("[Zotac4A] Full 32-byte identification read failed: %d", length);
        return(detected_controllers);
    }
    for(unsigned int i = 0; i < ZOTAC_4A_RGB_ID_LEN; i++)
    {
        bool valid = (i == 0) ? data[i] == 'N' :
                     (i == 4) ? (data[i] >= 'A' && data[i] <= 'Z') :
                     (i == 5) ? data[i] == '-' :
                     (data[i] >= '0' && data[i] <= '9');
        if(!valid)
        {
            LOG_ERROR("[Zotac4A] Identification response is invalid");
            return(detected_controllers);
        }
    }
    char firmware[ZOTAC_4A_RGB_ID_LEN + 1] = { 0 };
    memcpy(firmware, data, ZOTAC_4A_RGB_ID_LEN);
    Zotac4AGPUController* controller = new Zotac4AGPUController(bus, i2c_addr, firmware);
    RGBController_Zotac4AGPU* rgb_controller = new RGBController_Zotac4AGPU(controller);
    detected_controllers.push_back(rgb_controller);
    return(detected_controllers);
}

#ifdef _WIN32
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 5080 SOLID CORE", DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX5080_DEV, ZOTAC_SUB_VEN, ZOTAC_RTX5080_SOLID_CORE_SUB_DEV, 0x4A);
#endif

/*---------------------------------------------------------*\
| ZotacV3GPUControllerDetect.cpp                            |
|                                                           |
|   Detector for Zotac V3 GPU                               |
|                                                           |
|   Based on ZotacV2GPUControllerDetect                     |
|   Adapted for Zotac RTX 5070 SOLIC OC                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "ZotacV3GPUController.h"
#include "RGBController_ZotacV3GPU.h"
#include "i2c_smbus.h"
#include "pci_ids.h"
#include "LogManager.h"

/******************************************************************************************\
*                                                                                          *
*   DetectZotacV3GPUControllers                                                            *
*                                                                                          *
*       Detect ZOTAC 50 series RGB controllers on the enumerated I2C busses                *
*       at address 0x4A.                                                                   *
*                                                                                          *
*           bus - pointer to i2c_smbus_interface where RGB device is connected             *
*           dev - I2C address of RGB device                                                *
*                                                                                          *
\******************************************************************************************/

void DetectZotacV3GPUControllers(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name)
{
    u8  rdata_pkt[I2C_SMBUS_BLOCK_MAX]  = { 0x00 };

    if(bus->i2c_smbus_read_i2c_block_data(i2c_addr, ZOTAC_V3_GPU_REG_RGB, sizeof(rdata_pkt), rdata_pkt) >= 0)
    {
        ZotacV3GPUController*     controller     = new ZotacV3GPUController(bus, i2c_addr, name);
        RGBController_ZotacV3GPU* rgb_controller = new RGBController_ZotacV3GPU(controller);

        if(rgb_controller->config.numberOfZones > 0)
        {
            ResourceManager::get()->RegisterRGBController(rgb_controller);
        }
        else
        {
            LOG_ERROR("[%s] RGB controller not registered.", name.c_str());
        }
    }
}

REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 5070 Solid OC", DetectZotacV3GPUControllers, NVIDIA_VEN, NVIDIA_RTX5070_DEV, ZOTAC_SUB_VEN, ZOTAC_RTX5070_SOLID_SUB_DEV, 0x4A);

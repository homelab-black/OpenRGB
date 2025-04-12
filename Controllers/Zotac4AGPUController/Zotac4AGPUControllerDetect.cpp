/*---------------------------------------------------------*\
| ZotacV2GPUControllerDetect.cpp                            |
|                                                           |
|   Detector for Zotac V2 GPU                               |
|                                                           |
|   Peter Berendi                               28 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "Detector.h"
#include "Zotac4AGPUController.h"
#include "RGBController_Zotac4AGPU.h"
#include "pci_ids.h"
#include "LogManager.h"

/******************************************************************************************\
*                                                                                          *
*   DetectZotac4AGPUControllers                                                            *
*                                                                                          *
*       Detect ZOTAC 40xx series RGB controllers on the enumerated I2C busses              *
*       at address 0x4A.                                                                   *
*                                                                                          *
*           bus  - pointer to i2c_smbus_interface where RGB device is connected            *
*           dev  - I2C address of RGB device                                               *
*           name - ID string reported by the controller                                    *
*                                                                                          *
* Common function called from the OS specific detector                                     *
*                                                                                          *
\******************************************************************************************/

s32 DetectZotac4AGPUControllers(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name)
{
    bus->i2c_set_max_speed(10000);

    u8 rdata_pkt[I2C_SMBUS_BLOCK_MAX] = { 0x00 };
    s32 res = bus->i2c_smbus_read_i2c_block_data(i2c_addr, ZOTAC_4A_GPU_REG_RGB, sizeof(rdata_pkt), rdata_pkt);
    LOG_TRACE("i2c_smbus_read_i2c_block_data(0x%02X) returned %d", i2c_addr, res);
    if((res < 0) || (res > I2C_SMBUS_BLOCK_MAX))
    {
        bus->i2c_set_max_speed(0); // restore default speed if not detected
        return -1;
    }

    char tempstr[I2C_SMBUS_BLOCK_MAX * 3 + 1];
    int i;
    char *p = tempstr;
    for(i = 0; i < res; i++)
    {
        snprintf(p, 4, " %02X", rdata_pkt[i]);
        p += 3;
    }
    LOG_DEBUG("data:%s", tempstr);

    memcpy(tempstr, rdata_pkt, ZOTAC_4A_RGB_ID_LEN);
    tempstr[ZOTAC_4A_RGB_ID_LEN] = 0;
    LOG_TRACE("Found RGB controller '%s' on '%s'", tempstr, name.c_str());
    Zotac4AGPUController* controller = new Zotac4AGPUController(bus, i2c_addr, tempstr);
    RGBController_Zotac4AGPU* rgb_controller = new RGBController_Zotac4AGPU(controller);
    ResourceManager::get()->RegisterRGBController(rgb_controller);
    return res;
}

REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 4060 Ti Twin Edge 8G",        DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX4060TI_DEV,       ZOTAC_SUB_VEN, ZOTAC_RTX4060TI_8G_TWIN_EDGE_SUB_DEV,  0x4A);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 4070 Twin Edge",              DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX4070_DEV,         ZOTAC_SUB_VEN, ZOTAC_RTX4070_TWIN_EDGE_SUB_DEV,       0x4A);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 4070 Super Trinity/Twin Edge",DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX4070S_DEV,        ZOTAC_SUB_VEN, ZOTAC_RTX4070S_TRINITY_SUB_DEV,      0x4A);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 4070 Ti Super Trinity",       DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX4070TIS_DEV,      ZOTAC_SUB_VEN, ZOTAC_RTX4070TIS_TRINITY_SUB_DEV,      0x4A);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 4080 Super Trinity",          DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX4080S_DEV,        ZOTAC_SUB_VEN, ZOTAC_RTX4080S_TRINITY_SUB_DEV,        0x4A);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 5070 Solid",                  DetectZotac4AGPUControllers, NVIDIA_VEN, NVIDIA_RTX5070_DEV,         ZOTAC_SUB_VEN, ZOTAC_RTX5070_SOLID_SUB_DEV,           0x4A);

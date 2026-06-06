/*---------------------------------------------------------*\
| ZotacBlackwellGPUControllerDetect.cpp                     |
|                                                           |
|   Detector for ZOTAC Blackwell (RTX 50 series) GPU        |
|                                                           |
|   Eder Sánchez                                27 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include <vector>
#include "Detector.h"
#include "ZotacBlackwellGPUController.h"
#include "RGBController_ZotacBlackwellGPU.h"
#include "i2c_smbus.h"
#include "pci_ids.h"

/******************************************************************************************\
*                                                                                          *
*   RegisterZotacBlackwellGPUController                                                    *
*                                                                                          *
*       Probe register 0x10; if the controller answers, register an RGBController with     *
*       the card's zone layout. The zone names are positional - each name's index is       *
*       written verbatim to the zone register (0x21).                                      *
*                                                                                          *
\******************************************************************************************/

static void RegisterZotacBlackwellGPUController(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name, const std::vector<std::string>& zone_names)
{
    s32 result = bus->i2c_smbus_read_byte_data(i2c_addr, 0x10);

    if(result >= 0)
    {
        ZotacBlackwellGPUController*     controller     = new ZotacBlackwellGPUController(bus, i2c_addr, name);
        RGBController_ZotacBlackwellGPU* rgb_controller = new RGBController_ZotacBlackwellGPU(controller, zone_names);

        ResourceManager::get()->RegisterRGBController(rgb_controller);
    }
}

void DetectZotacBlackwellGPUControllersPCI(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name)
{
    RegisterZotacBlackwellGPUController(bus, i2c_addr, name, { "Logo", "Side Bar", "Infinity Mirror" });
}

void DetectZotacBlackwellSolidGPUControllersPCI(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name)
{
    RegisterZotacBlackwellGPUController(bus, i2c_addr, name, { "ZOTAC Gaming", "Logo" });
}

REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 5080 AMP Extreme INFINITY",  DetectZotacBlackwellGPUControllersPCI, NVIDIA_VEN, NVIDIA_RTX5080_DEV, ZOTAC_SUB_VEN, ZOTAC_RTX5080_AMP_EXTREME_SUB_DEV, 0x4B);
REGISTER_I2C_PCI_DETECTOR("ZOTAC GAMING GeForce RTX 5090 SOLID OC",              DetectZotacBlackwellSolidGPUControllersPCI, NVIDIA_VEN, NVIDIA_RTX5090_DEV, ZOTAC_SUB_VEN, ZOTAC_RTX5090_SOLID_OC_SUB_DEV, 0x4B);

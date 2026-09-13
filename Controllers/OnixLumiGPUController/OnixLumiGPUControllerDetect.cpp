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

#include "DetectionManager.h"
#include "i2c_intel_gpu.h"
#include "pci_ids.h"
#include "OnixLumiGPUController.h"
#include "RGBController_OnixLumiGPU.h"

#define ONIX_LUMI_DETECTOR_NAME          "ONIX LUMI Intel Arc B580"
#define ONIX_LUMI_GPU_DEVICE             0xE20B
#define ONIX_LUMI_SUBSYSTEM_VENDOR       0x207E
#define ONIX_LUMI_SUBSYSTEM_DEVICE       0xA002
#define ONIX_LUMI_I2C_ADDRESS            0x28

DetectedControllers DetectOnixLumiGPUControllers(i2c_smbus_interface* bus, u8 i2c_addr, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(!is_intel_gpu_i2c_bus(bus))
    {
        return(detected_controllers);
    }

    OnixLumiGPUController* controller = new OnixLumiGPUController(bus, i2c_addr, name);
    RGBController_OnixLumiGPU* rgb_controller = new RGBController_OnixLumiGPU(controller);

    detected_controllers.push_back(rgb_controller);

    return(detected_controllers);
}

REGISTER_I2C_PCI_DETECTOR(ONIX_LUMI_DETECTOR_NAME, DetectOnixLumiGPUControllers, INTEL_VEN, ONIX_LUMI_GPU_DEVICE, ONIX_LUMI_SUBSYSTEM_VENDOR, ONIX_LUMI_SUBSYSTEM_DEVICE, ONIX_LUMI_I2C_ADDRESS);

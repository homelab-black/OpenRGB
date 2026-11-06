/*---------------------------------------------------------*\
| NuPhyHalo96Detect.cpp                                     |
|                                                           |
|   Detector for NuPhy Halo96 V2 keyboard.                  |
|                                                           |
|   Amir Yelenov (GreamDesu)                    22 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <hidapi.h>
#include "Detector.h"                       // For REGISTER_HID_DETECTOR_IPU and ResourceManager
#include "NuPhyHalo96Controller.h"          // For NuPhyHalo96Controller
#include "RGBController_NuPhyHalo96.h"      // For RGBController_NuPhyHalo96
#include "LogManager.h"                     // For logging

// Define VID, PID, Interface, Usage Page, and Usage for NuPhy Halo96 V2
#define NUPHY_HALO96_VID 0x19F5
#define NUPHY_HALO96_PID 0x3302
#define NUPHY_HALO96_INTERFACE 1
#define NUPHY_HALO96_USAGE_PAGE 0xFF60 
#define NUPHY_HALO96_USAGE_PAGE_ALT 0xFF00 
#define NUPHY_HALO96_USAGE 0x0061    

// Detector function that will be called by ResourceManager for matching HID devices
static void DetectNuPhyHalo96V2(hid_device_info* device_info, const std::string& name)
{
    LOG_INFO("NuPhyDetect: PROBE_ENTRY"); // Simplest possible log

    // --- Block for device_info checks and Details log (should be active) ---
    if (!device_info)
    {
        LOG_ERROR("NuPhyDetect (%s): Probe 2 - CRITICAL: device_info is NULL. Aborting detection for this call.", name.c_str());
        return;
    }

    LOG_INFO("NuPhyDetect (%s): Probe 3 - device_info is not NULL. Path pointer: %p", name.c_str(), device_info->path);

    if (!device_info->path)
    {
        LOG_ERROR("NuPhyDetect (%s): Probe 4 - CRITICAL: device_info->path is NULL. Aborting detection for this call.", name.c_str());
        return;
    }
    
    LOG_INFO("NuPhyDetect (%s): Probe 5 - device_info->path is not NULL. Path: '%s'", name.c_str(), device_info->path);
    LOG_INFO("NuPhyDetect (%s): Details - Interface: %d, Usage Page: 0x%04X, Usage: 0x%04X", 
             name.c_str(), device_info->interface_number, device_info->usage_page, device_info->usage);
    // --- End of device_info checks ---

    LOG_INFO("NuPhyDetect (%s): Probe 6 - Before filtering if statement.", name.c_str());

    // --- Filtering logic (should be active) ---
    if (!((device_info->usage_page == NUPHY_HALO96_USAGE_PAGE || device_info->usage_page == NUPHY_HALO96_USAGE_PAGE_ALT) &&
          device_info->usage == NUPHY_HALO96_USAGE))
    {
        LOG_INFO("NuPhyDetect (%s): Skipped. Path %s, interface %d, usage_page 0x%04X, usage 0x%04X does not match target UPs (0x%04X or 0x%04X) and U (0x%04X).",
                 name.c_str(), device_info->path, device_info->interface_number, device_info->usage_page, device_info->usage,
                 NUPHY_HALO96_USAGE_PAGE, NUPHY_HALO96_USAGE_PAGE_ALT, NUPHY_HALO96_USAGE);
        return;
    }

    LOG_INFO("NuPhyDetect (%s): Matched! Proceeding with open for path %s, interface %d, usage_page 0x%04X, usage 0x%04X",
             name.c_str(), device_info->path, device_info->interface_number, device_info->usage_page, device_info->usage);
    // --- End of filtering logic ---

    // --- hid_open_path block (now active) ---
    hid_device* hid_handle = hid_open_path(device_info->path);
    if (!hid_handle)
    {
        LOG_ERROR("NuPhyDetect: Failed to open HID device at path %s", device_info->path);
        return;
    }

    LOG_INFO("NuPhyDetect: Successfully opened HID device %s", name.c_str());
    // --- End of hid_open_path block ---

    // --- NuPhyHalo96Controller creation block (now active) ---
    NuPhyHalo96Controller* low_level_controller = nullptr;
    try
    {
        low_level_controller = new NuPhyHalo96Controller(hid_handle, *device_info);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("NuPhyDetect: Exception creating NuPhyHalo96Controller for %s: %s", name.c_str(), e.what());
        hid_close(hid_handle); // Close the handle if controller creation fails
        return;
    }
    catch (...)
    {
        LOG_ERROR("NuPhyDetect: Unknown exception creating NuPhyHalo96Controller for %s", name.c_str());
        hid_close(hid_handle); // Close the handle if controller creation fails
        return;
    }
    // --- End of NuPhyHalo96Controller creation block ---

    RGBController_NuPhyHalo96* rgb_controller = nullptr;
    try
    {
        rgb_controller = new RGBController_NuPhyHalo96(low_level_controller);
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("NuPhyDetect: Exception creating RGBController_NuPhyHalo96 for %s: %s", name.c_str(), e.what());
        delete low_level_controller; 
        return;
    }
    catch (...)
    {
        LOG_ERROR("NuPhyDetect: Unknown exception creating RGBController_NuPhyHalo96 for %s", name.c_str());
        delete low_level_controller; 
        return;
    }

    ResourceManager::get()->RegisterRGBController(rgb_controller);
    LOG_INFO("NuPhyDetect: Registration of %s SUCCESSFUL.", name.c_str());
}

// Register the detector function with ResourceManager using the macro.
// This creates a static HIDDeviceDetector object, whose constructor calls
// ResourceManager::get()->RegisterHIDDeviceDetector(...).

// Commenting out the previous specific registrations
// REGISTER_HID_DETECTOR_IPU("NuPhy Halo96 V2 (FF60)", DetectNuPhyHalo96V2, NUPHY_HALO96_VID, NUPHY_HALO96_PID, NUPHY_HALO96_INTERFACE, NUPHY_HALO96_USAGE_PAGE, NUPHY_HALO96_USAGE);
// REGISTER_HID_DETECTOR_IPU("NuPhy Halo96 V2 (FF00)", DetectNuPhyHalo96V2, NUPHY_HALO96_VID, NUPHY_HALO96_PID, NUPHY_HALO96_INTERFACE, NUPHY_HALO96_USAGE_PAGE_ALT, NUPHY_HALO96_USAGE);

// New broader registration: VID, PID, Interface. Will call DetectNuPhyHalo96V2 for ANY usage page/usage on this interface.
// Internal filtering in DetectNuPhyHalo96V2 will then select the correct one.
REGISTER_HID_DETECTOR_I("NuPhy Halo96 V2 (Interface Detect)", DetectNuPhyHalo96V2, NUPHY_HALO96_VID, NUPHY_HALO96_PID, NUPHY_HALO96_INTERFACE);


// If HID_USAGE_PAGE_ANY and HID_USAGE_ANY are available and preferred to catch both 0xFF60 and 0xFF00 under interface 1:
// #include "hidapi_shared.h" // Check if this brings in HID_USAGE_PAGE_ANY etc.
// REGISTER_HID_DETECTOR_I("NuPhy Halo96 V2 (AnyUP)", DetectNuPhyHalo96V2, NUPHY_HALO96_VID, NUPHY_HALO96_PID, NUPHY_HALO96_INTERFACE); 
// Then, inside DetectNuPhyHalo96V2, you would add:
// if (device_info->usage_page != 0xFF60 && device_info->usage_page != 0xFF00) { return; } 
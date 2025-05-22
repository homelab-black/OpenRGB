/*---------------------------------------------------------*\
| NuPhyHalo96Controller.h                                   |
|                                                           |
|   Controller for NuPhy Halo96 V2 keyboard.                |
|   Handles low-level HID communication.                    |
|                                                           |
|   Amir Yelenov (GreamDesu)                    22 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/
#pragma once
#include <hidapi.h>
#include "RGBController.h"
#include <string>
#include <vector>

// Forward declaration of hid_device_info if not included by hidapi.h
struct hid_device_info;

class NuPhyHalo96Controller {
public:
    // Constructor now takes an opened HID device handle and device info
    NuPhyHalo96Controller(hid_device* open_device_handle, const hid_device_info& device_info_ref);
    ~NuPhyHalo96Controller();

    // DetectDevice() and OpenDevice() are removed as detection is now handled by 
    // the static detector function and ResourceManager.
    // CloseDevice() functionality is handled by the destructor.

    void EnsureSoftwareModeActive();
    void EnableSignalRGBInternal();
    void DisableSignalRGB();
    void SetLEDs(const std::vector<RGBColor>& colors);
    void SetKeystrokeColor(unsigned int led_idx, unsigned char r, unsigned char g, unsigned char b);
    bool IsDeviceOpen();
    std::string GetDeviceLocation();
    std::string GetSerialString();

private:
    hid_device* dev;
    std::string device_path;
    static const int VENDOR_ID = 0x19F5;
    static const int PRODUCT_ID = 0x3302;
    // Add a member to store the hid_device_info if needed for serial/location strings later
    // Or ensure path and serial are extracted in constructor if info_ref is not stored.
    // hid_device_info hid_info_cache; // Example if you store the whole struct
};

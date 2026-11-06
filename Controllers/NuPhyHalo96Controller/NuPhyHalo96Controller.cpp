/*---------------------------------------------------------*\
| NuPhyHalo96Controller.cpp                                 |
|                                                           |
|   Controller for NuPhy Halo96 V2 keyboard.                |
|   Handles low-level HID communication.                    |
|                                                           |
|   Amir Yelenov (GreamDesu)                    22 May 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/
#include "NuPhyHalo96Controller.h"
#include <hidapi.h> // Ensure hid_device_info is known for the constructor
#include <thread>
#include <algorithm>
#include <vector>
#include <cstring> // For memset
#include "LogManager.h" // For logging, if needed
// RGBColor definition and macros like RGBGetRValue are in RGBController.h, which is included by NuPhyHalo96Controller.h

// Define report length constant
const int HID_REPORT_BUFFER_SIZE = 32;    // Size of buffer passed to hid_write, matching Python's explicit 32-byte packets starting with Report ID 0x00

// New constructor taking an already opened device handle and device info
NuPhyHalo96Controller::NuPhyHalo96Controller(hid_device* open_device_handle, const hid_device_info& device_info_ref)
    : dev(open_device_handle), device_path("")
{
    if (dev)
    {
        if (device_info_ref.path)
        {
            this->device_path = device_info_ref.path;
        }
        LOG_INFO("NuPhyHalo96Controller: Instance created. Path: %s", this->device_path.c_str());
    }
    else
    {
        LOG_ERROR("NuPhyHalo96Controller: Constructed with a null HID handle.");
    }
}

NuPhyHalo96Controller::~NuPhyHalo96Controller()
{
    LOG_INFO("NuPhyHalo96Controller: Destructor ~NuPhyHalo96Controller() - Entry. Path: %s", device_path.c_str());
    if (dev)
    {
        LOG_INFO("NuPhyHalo96Controller: ~NuPhyHalo96Controller() - Device is open, attempting to disable software mode and close. Path: %s", this->device_path.c_str());
        LOG_INFO("NuPhyHalo96Controller: ~NuPhyHalo96Controller() - Calling DisableSignalRGB(). Path: %s", this->device_path.c_str());
        DisableSignalRGB(); // Revert to hardware mode before closing
        LOG_INFO("NuPhyHalo96Controller: ~NuPhyHalo96Controller() - DisableSignalRGB() returned. Calling hid_close(). Path: %s", this->device_path.c_str());
        hid_close(dev);
        dev = nullptr;
        LOG_INFO("NuPhyHalo96Controller: ~NuPhyHalo96Controller() - Device released and closed. Path: %s", this->device_path.c_str());
    }
    else
    {
        LOG_WARNING("NuPhyHalo96Controller: ~NuPhyHalo96Controller() - Device was already null. Path: %s", device_path.c_str());
    }
    LOG_INFO("NuPhyHalo96Controller: Destructor ~NuPhyHalo96Controller() - Exit. Path: %s", device_path.c_str());
}

// Renamed from EnableSignalRGB
void NuPhyHalo96Controller::EnableSignalRGBInternal()
{
    if (!dev)
    {
        LOG_ERROR("NuPhyHalo96Controller: EnableSignalRGBInternal called but device is not open.");
        return;
    }
    unsigned char data[HID_REPORT_BUFFER_SIZE];
    std::memset(data, 0, sizeof(data));
    data[0] = 0x00; // Report ID
    data[1] = 0x25; // Enable SignalRGB/Direct control command

    LOG_INFO("NuPhyHalo96Controller: Sending EnableSignalRGBInternal (0x25). Path: %s", device_path.c_str());
    int bytes_written = hid_write(dev, data, sizeof(data));
    if (bytes_written == -1)
    {
        LOG_ERROR("NuPhyHalo96Controller: hid_write failed for EnableSignalRGBInternal. Error: %ls. Path: %s", hid_error(dev), device_path.c_str());
    }
    else
    {
        // LOG_INFO("NuPhyHalo96Controller: EnableSignalRGBInternal hid_write returned %d. Path: %s", bytes_written, device_path.c_str());
        if (bytes_written != HID_REPORT_BUFFER_SIZE && bytes_written != HID_REPORT_BUFFER_SIZE + 1)
        {
            LOG_WARNING("NuPhyHalo96Controller: EnableSignalRGBInternal - hid_write: Unexpected number of bytes: %d (expected %d or %d). Path: %s", bytes_written, HID_REPORT_BUFFER_SIZE, HID_REPORT_BUFFER_SIZE + 1, device_path.c_str());
        }
    }
}

void NuPhyHalo96Controller::EnsureSoftwareModeActive()
{
    // LOG_INFO("NuPhyHalo96Controller: EnsureSoftwareModeActive - Called. Path: %s", device_path.c_str());
    EnableSignalRGBInternal();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
    // LOG_INFO("NuPhyHalo96Controller: EnsureSoftwareModeActive - Finished delay. Path: %s", device_path.c_str());
}

void NuPhyHalo96Controller::DisableSignalRGB()
{
    if (!dev)
    {
        return;
    }
    unsigned char data[HID_REPORT_BUFFER_SIZE];
    std::memset(data, 0, sizeof(data));
    data[0] = 0x00; // Report ID
    data[1] = 0x26; // Disable SignalRGB/Direct control command
    
    LOG_INFO("NuPhyHalo96Controller: Sending DisableSignalRGB (0x26). Path: %s", device_path.c_str());
    int bytes_written = hid_write(dev, data, sizeof(data));
    if (bytes_written == -1)
    {
        LOG_ERROR("NuPhyHalo96Controller: hid_write failed for DisableSignalRGB. Error: %ls. Path: %s", hid_error(dev), device_path.c_str());
    }
    else
    {
        // LOG_INFO("NuPhyHalo96Controller: DisableSignalRGB hid_write returned %d. Path: %s", bytes_written, device_path.c_str());
        if (bytes_written != HID_REPORT_BUFFER_SIZE && bytes_written != HID_REPORT_BUFFER_SIZE + 1)
        {
            LOG_WARNING("NuPhyHalo96Controller: DisableSignalRGB - hid_write: Unexpected number of bytes: %d (expected %d or %d). Path: %s", bytes_written, HID_REPORT_BUFFER_SIZE, HID_REPORT_BUFFER_SIZE + 1, device_path.c_str());
        }
    }
}

void NuPhyHalo96Controller::SetLEDs(const std::vector<RGBColor>& colors)
{
    if (!dev) return;
    // LOG_INFO("NuPhyHalo96Controller: SetLEDs - Ensuring software mode active before sending data.");
    EnableSignalRGBInternal();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    const int MAX_LEDS_PER_PACKET = 9; 
    int num_leds_to_set = static_cast<int>(colors.size());

    for (int i = 0; i < num_leds_to_set; i += MAX_LEDS_PER_PACKET)
    {
        unsigned char data[HID_REPORT_BUFFER_SIZE];
        std::memset(data, 0, sizeof(data));
        
        data[0] = 0x00;      // Report ID
        data[1] = 0x24;      // Set LEDs command
        data[2] = static_cast<unsigned char>(i); // Start index
        
        int leds_in_this_packet = std::min(MAX_LEDS_PER_PACKET, num_leds_to_set - i);
        data[3] = static_cast<unsigned char>(leds_in_this_packet); // Count

        for (int j = 0; j < leds_in_this_packet; ++j)
        {
            if ((i + j) < num_leds_to_set)
            {
                const RGBColor& color = colors[i + j];
                data[4 + j * 3 + 0] = RGBGetRValue(color); // Data starts at data[4]
                data[4 + j * 3 + 1] = RGBGetGValue(color);
                data[4 + j * 3 + 2] = RGBGetBValue(color);
            }
        }
        
        int bytes_written = hid_write(dev, data, sizeof(data));
        if (bytes_written == -1)
        {
            LOG_ERROR("NuPhyHalo96Controller: SetLEDs - hid_write failed. Error: %ls. Path: %s", hid_error(dev), device_path.c_str());
            return; 
        }
        else if (bytes_written != HID_REPORT_BUFFER_SIZE && bytes_written != HID_REPORT_BUFFER_SIZE + 1)
        {
             LOG_WARNING("NuPhyHalo96Controller: SetLEDs - Unexpected number of bytes written: %d (expected %d or %d) for packet starting at LED %d. Path: %s", bytes_written, HID_REPORT_BUFFER_SIZE, HID_REPORT_BUFFER_SIZE + 1, i, device_path.c_str());
        }
        // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}

void NuPhyHalo96Controller::SetKeystrokeColor(unsigned int led_idx, unsigned char r, unsigned char g, unsigned char b)
{
    if (!dev) return;
    // LOG_INFO("NuPhyHalo96Controller: SetKeystrokeColor - Ensuring software mode active before sending data.");
    EnableSignalRGBInternal();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    unsigned char data[HID_REPORT_BUFFER_SIZE];
    std::memset(data, 0, sizeof(data));

    data[0] = 0x00; // Report ID
    data[1] = 0x24; // Set LEDs command
    data[2] = static_cast<unsigned char>(led_idx);
    data[3] = 1;    // Count of LEDs to set
    data[4] = r;
    data[5] = g;
    data[6] = b;

    int bytes_written = hid_write(dev, data, sizeof(data));
    if (bytes_written == -1)
    {
        LOG_ERROR("NuPhyHalo96Controller: SetKeystrokeColor - hid_write failed for LED %u. Error: %ls. Path: %s", led_idx, hid_error(dev), device_path.c_str());
    }
    else if (bytes_written != HID_REPORT_BUFFER_SIZE && bytes_written != HID_REPORT_BUFFER_SIZE + 1)
    {
        LOG_WARNING("NuPhyHalo96Controller: SetKeystrokeColor - Unexpected number of bytes written: %d (expected %d or %d) for LED %u. Path: %s", bytes_written, HID_REPORT_BUFFER_SIZE, HID_REPORT_BUFFER_SIZE + 1, led_idx, device_path.c_str());
    }
}

bool NuPhyHalo96Controller::IsDeviceOpen()
{
    return dev != nullptr;
}

std::string NuPhyHalo96Controller::GetDeviceLocation()
{
    if (dev && !device_path.empty()) 
    {
        return device_path; // Return the path used to open the device
    }
    // This fallback should ideally not be reached if constructor logic is robust
    // and dev is always valid when this is called by RGBController_NuPhyHalo96.
    return "USB (Path not available)"; 
}

std::string NuPhyHalo96Controller::GetSerialString()
{
    // hid_get_serial_number_string can be used if dev is open.
    if (dev)
    {
        wchar_t wstr[255] = {0}; // Initialize for safety
        if (hid_get_serial_number_string(dev, wstr, 254) == 0)
        {
            if (wstr[0] != L'\0')
            {
                // Basic wchar_t to std::string conversion (locale-dependent, mostly for simple ASCII-like serials)
                // For robust Unicode, a proper conversion library or Qt functions would be better.
                std::wstring ws(wstr);
                std::string str(ws.begin(), ws.end()); 
                // A better simple conversion for Windows if serial is likely plain chars:
                // char c_str[255];
                // size_t num_converted;
                // wcstombs_s(&num_converted, c_str, sizeof(c_str), wstr, _TRUNCATE);
                // if (num_converted > 0 && num_converted <= sizeof(c_str)) return std::string(c_str);

                // Using the potentially problematic simple conversion for now, as serial is "N/A" anyway.
                if (!str.empty() && str.length() < 250) return str; // Basic sanity check
            }
        }
    }
    return "N/A"; // Generic fallback
}



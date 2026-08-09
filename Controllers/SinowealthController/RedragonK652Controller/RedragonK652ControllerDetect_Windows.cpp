/*---------------------------------------------------------*\
| RedragonK652ControllerDetect_Windows.cpp                 |
|                                                           |
|   Detector for the wired Redragon K652 keyboard           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <array>
#include <cstring>
#include <cstdint>
#include <cwchar>
#include <exception>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <hidsdi.h>
#include <hidpi.h>

#include <hidapi.h>
#include <hidapi_winapi.h>

#include "DetectionManager.h"
#include "LogManager.h"
#include "RedragonK652Controller.h"
#include "RedragonK652DeviceRegistry.h"
#include "RGBController_RedragonK652.h"

/*
 * K652 lighting configuration is validated only over wired USB. The 2.4 GHz
 * receiver is 25A7:FA70, but the official software does not configure it;
 * do not alias it to this protocol without independent validation. Wired
 * settings persist when switched to wireless.
 */

#define REDRAGON_K652_VID                    0x258A
#define REDRAGON_K652_PID                    0x0049
#define REDRAGON_K652_RELEASE_NUMBER         0x0104
#define REDRAGON_K652_INTERFACE              1
#define REDRAGON_K652_USAGE_PAGE             0xFF00
#define REDRAGON_K652_USAGE                  0x0001
#define REDRAGON_K652_COMMAND_REPORT_ID       0x05
#define REDRAGON_K652_COMMAND_REPORT_SIZE     6
#define REDRAGON_K652_DATA_REPORT_ID          0x06

struct redragon_k652_topology_entry
{
    int             interface_number;
    unsigned short  usage_page;
    unsigned short  usage;
    unsigned int    count;
};

struct redragon_k652_hid_global_state
{
    std::uint32_t   report_size  = 0;
    std::uint32_t   report_count = 0;
    unsigned char   report_id    = 0;
};

struct redragon_k652_feature_layout
{
    std::array<std::size_t, 256> feature_bits    = {};
    std::array<bool, 256>        feature_present = {};
    bool                         uses_report_ids = false;
};

struct redragon_k652_hid_handle_deleter
{
    void operator()(hid_device* device) const
    {
        if(device)
        {
            hid_close(device);
        }
    }
};

struct redragon_k652_hid_info_deleter
{
    void operator()(hid_device_info* info) const
    {
        if(info)
        {
            hid_free_enumeration(info);
        }
    }
};

using redragon_k652_hid_handle = std::unique_ptr<hid_device, redragon_k652_hid_handle_deleter>;
using redragon_k652_hid_info   = std::unique_ptr<hid_device_info, redragon_k652_hid_info_deleter>;

static const std::array<redragon_k652_topology_entry, 6> redragon_k652_topology =
{{
    { 0, 0x0001, 0x0006, 1 },
    { 1, 0x0001, 0x0080, 1 },
    { 1, 0x000C, 0x0001, 1 },
    { 1, 0xFF00, 0x0001, 4 },
    { 1, 0x0001, 0x0006, 1 },
    { 1, 0x0001, 0x0002, 1 },
}};

static bool ReadRedragonK652HIDItemValue(const unsigned char* data, std::size_t data_size, std::uint32_t& value)
{
    if(data_size > sizeof(value))
    {
        return(false);
    }

    value = 0;

    for(std::size_t byte_idx = 0; byte_idx < data_size; byte_idx++)
    {
        value |= static_cast<std::uint32_t>(data[byte_idx]) << (byte_idx * 8);
    }

    return(true);
}

static bool ParseRedragonK652FeatureLayout(const unsigned char* descriptor, std::size_t descriptor_size, redragon_k652_feature_layout& layout)
{
    redragon_k652_hid_global_state              global_state;
    std::vector<redragon_k652_hid_global_state> global_stack;
    std::size_t                                 collection_depth = 0;
    std::size_t                                 descriptor_idx   = 0;

    if(!descriptor || descriptor_size == 0)
    {
        return(false);
    }

    while(descriptor_idx < descriptor_size)
    {
        const unsigned char prefix = descriptor[descriptor_idx++];

        /* Long items are not expected in the reconstructed K652 descriptors. */
        if(prefix == 0xFE)
        {
            return(false);
        }

        std::size_t item_size = prefix & 0x03;

        if(item_size == 3)
        {
            item_size = 4;
        }

        if(item_size > descriptor_size - descriptor_idx)
        {
            return(false);
        }

        const unsigned int item_type = (prefix >> 2) & 0x03;
        const unsigned int item_tag  = (prefix >> 4) & 0x0F;
        std::uint32_t      item_value;

        if(!ReadRedragonK652HIDItemValue(&descriptor[descriptor_idx], item_size, item_value))
        {
            return(false);
        }

        descriptor_idx += item_size;

        if(item_type == 0)
        {
            /* Main items */
            if(item_tag == 0x0A)
            {
                /* Collection */
                if(item_size == 0)
                {
                    return(false);
                }

                collection_depth++;
            }
            else if(item_tag == 0x0C)
            {
                /* End Collection */
                if(item_size != 0 || collection_depth == 0)
                {
                    return(false);
                }

                collection_depth--;
            }
            else if(item_tag == 0x0B)
            {
                /* Feature */
                if(item_size == 0 || collection_depth == 0 || global_state.report_size == 0 || global_state.report_count == 0)
                {
                    return(false);
                }

                const std::size_t report_size  = global_state.report_size;
                const std::size_t report_count = global_state.report_count;

                if(report_size > std::numeric_limits<std::size_t>::max() / report_count)
                {
                    return(false);
                }

                const std::size_t feature_bits = report_size * report_count;
                const unsigned int report_id    = global_state.report_id;

                if(layout.feature_bits[report_id] > std::numeric_limits<std::size_t>::max() - feature_bits)
                {
                    return(false);
                }

                layout.feature_bits[report_id] += feature_bits;
                layout.feature_present[report_id] = true;
            }
            else if(item_tag != 0x08 && item_tag != 0x09)
            {
                /* Only Input and Output are otherwise valid Main items here. */
                return(false);
            }
        }
        else if(item_type == 1)
        {
            /* Global items */
            if(item_tag == 0x07)
            {
                /* Report Size */
                if(item_size == 0)
                {
                    return(false);
                }

                global_state.report_size = item_value;
            }
            else if(item_tag == 0x08)
            {
                /* Report ID */
                if(item_size != 1 || item_value == 0 || item_value > 0xFF)
                {
                    return(false);
                }

                global_state.report_id = static_cast<unsigned char>(item_value);
                layout.uses_report_ids = true;
            }
            else if(item_tag == 0x09)
            {
                /* Report Count */
                if(item_size == 0)
                {
                    return(false);
                }

                global_state.report_count = item_value;
            }
            else if(item_tag == 0x0A)
            {
                /* Push */
                if(item_size != 0 || global_stack.size() >= descriptor_size)
                {
                    return(false);
                }

                global_stack.push_back(global_state);
            }
            else if(item_tag == 0x0B)
            {
                /* Pop */
                if(item_size != 0 || global_stack.empty())
                {
                    return(false);
                }

                global_state = global_stack.back();
                global_stack.pop_back();
            }
            else if(item_tag > 0x06)
            {
                /* Unknown or reserved Global item. */
                return(false);
            }
        }
        else if(item_type == 3)
        {
            /* Reserved item type. */
            return(false);
        }
        /* Local items do not affect report sizes. */
    }

    return(collection_depth == 0
        && global_stack.empty()
        && (!layout.uses_report_ids || !layout.feature_present[0]));
}

static bool GetRedragonK652FeatureReportTotalSize(const redragon_k652_feature_layout& layout, unsigned char report_id, std::size_t& report_size)
{
    if(!layout.uses_report_ids
    || report_id == 0
    || !layout.feature_present[report_id]
    || (layout.feature_bits[report_id] % 8) != 0)
    {
        return(false);
    }

    const std::size_t payload_size = layout.feature_bits[report_id] / 8;

    if(payload_size == std::numeric_limits<std::size_t>::max())
    {
        return(false);
    }

    report_size = payload_size + 1;
    return(true);
}

static bool GetRedragonK652MaximumFeatureReportSize(const redragon_k652_feature_layout& layout, std::size_t& maximum_size)
{
    if(layout.uses_report_ids && layout.feature_present[0])
    {
        return(false);
    }

    maximum_size = 0;

    const std::size_t first_report_id = layout.uses_report_ids ? 1 : 0;

    for(std::size_t report_id = first_report_id; report_id < layout.feature_present.size(); report_id++)
    {
        if(layout.feature_present[report_id])
        {
            const std::size_t report_bits = layout.feature_bits[report_id];

            if(report_bits > std::numeric_limits<std::size_t>::max() - 7)
            {
                return(false);
            }

            const std::size_t payload_size = (report_bits + 7) / 8;
            const std::size_t report_id_size = layout.uses_report_ids ? 1 : 0;

            if(payload_size > std::numeric_limits<std::size_t>::max() - report_id_size)
            {
                return(false);
            }

            const std::size_t report_size = payload_size + report_id_size;

            maximum_size = std::max(maximum_size, report_size);
        }
    }

    /* HIDP_CAPS reports zero when a collection has no Feature reports. */
    return(true);
}

static bool IsRedragonK652FeatureRole(const redragon_k652_feature_layout& layout, unsigned char report_id,
                                      std::size_t expected_size, std::size_t maximum_size)
{
    std::size_t report_size;

    return(GetRedragonK652FeatureReportTotalSize(layout, report_id, report_size)
        && report_size  == expected_size
        && maximum_size == expected_size);
}

static bool GetRedragonK652FeatureLayout(hid_device* device, redragon_k652_feature_layout& layout)
{
    std::array<unsigned char, HID_API_MAX_REPORT_DESCRIPTOR_SIZE> descriptor = {};

    layout = {};

    /*-----------------------------------------------------------------*\
    | On Windows hidapi reconstructs this descriptor from the cached    |
    | HID preparsed data.  This is passive and does not issue a Feature, |
    | Input, or Output report to the keyboard.                           |
    \*-----------------------------------------------------------------*/
    const int result = hid_get_report_descriptor(device, descriptor.data(), descriptor.size());

    return(result > 0
        && static_cast<std::size_t>(result) <= descriptor.size()
        && ParseRedragonK652FeatureLayout(descriptor.data(), static_cast<std::size_t>(result), layout));
}

static bool GetRedragonK652FeatureReportByteLength(const char* path, std::size_t& report_size)
{
    /*-----------------------------------------------------------------*\
    | Windows hidapi pads feature writes to HIDP_CAPS' native maximum.  |
    | Read that cached capability directly so a reconstructed descriptor |
    | can never make a longer write look safe.  No HID report is issued. |
    \*-----------------------------------------------------------------*/
    HANDLE device_handle = CreateFileA(path,
                                       0,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       nullptr,
                                       OPEN_EXISTING,
                                       0,
                                       nullptr);

    if(device_handle == INVALID_HANDLE_VALUE)
    {
        return(false);
    }

    PHIDP_PREPARSED_DATA preparsed_data = nullptr;
    HIDP_CAPS            capabilities   = {};
    bool                 valid          = false;

    if(HidD_GetPreparsedData(device_handle, &preparsed_data))
    {
        if(HidP_GetCaps(preparsed_data, &capabilities) == HIDP_STATUS_SUCCESS)
        {
            report_size = capabilities.FeatureReportByteLength;
            valid = true;
        }

        HidD_FreePreparsedData(preparsed_data);
    }

    CloseHandle(device_handle);
    return(valid);
}

static bool IsRedragonK652Identity(const hid_device_info* info)
{
    return(info
        && info->path
        && info->vendor_id      == REDRAGON_K652_VID
        && info->product_id     == REDRAGON_K652_PID
        && info->release_number == REDRAGON_K652_RELEASE_NUMBER
        && info->product_string
        && std::wcscmp(info->product_string, L"Gaming Keyboard") == 0
        && info->bus_type       == HID_API_BUS_USB);
}

static bool GetRedragonK652ContainerId(const char* path, GUID& container_id)
{
    redragon_k652_hid_handle device(hid_open_path(path));

    if(!device)
    {
        return(false);
    }

    const int result = hid_winapi_get_container_id(device.get(), &container_id);

    const GUID null_container_id = {};

    /* A null ID cannot safely correlate collections to one physical device. */
    return(result == 0 && !IsEqualGUID(container_id, null_container_id));
}

static bool ValidateRedragonK652Topology(hid_device_info* info, std::vector<std::string>& vendor_paths, GUID& container_id)
{
    if(!GetRedragonK652ContainerId(info->path, container_id))
    {
        return(false);
    }

    redragon_k652_hid_info devices(hid_enumerate(REDRAGON_K652_VID, REDRAGON_K652_PID));

    if(!devices)
    {
        return(false);
    }

    std::array<unsigned int, 6> counts = {};
    unsigned int collection_count = 0;
    bool valid = true;

    for(hid_device_info* candidate = devices.get(); candidate; candidate = candidate->next)
    {
        GUID candidate_container_id;

        /*-------------------------------------------------*\
        | Container IDs are obtained without issuing any    |
        | feature, output, or input reports.  If any sibling |
        | cannot be identified, fail closed rather than      |
        | accepting an incomplete topology.                  |
        \*-------------------------------------------------*/
        if(!candidate->path || !GetRedragonK652ContainerId(candidate->path, candidate_container_id))
        {
            valid = false;
            break;
        }

        if(!IsEqualGUID(container_id, candidate_container_id))
        {
            continue;
        }

        collection_count++;

        if(!IsRedragonK652Identity(candidate))
        {
            valid = false;
            break;
        }

        bool topology_entry_found = false;

        for(unsigned int entry_idx = 0; entry_idx < redragon_k652_topology.size(); entry_idx++)
        {
            const redragon_k652_topology_entry& entry = redragon_k652_topology[entry_idx];

            if(candidate->interface_number == entry.interface_number
            && candidate->usage_page       == entry.usage_page
            && candidate->usage            == entry.usage)
            {
                counts[entry_idx]++;
                topology_entry_found = true;

                if(candidate->interface_number == REDRAGON_K652_INTERFACE
                && candidate->usage_page       == REDRAGON_K652_USAGE_PAGE
                && candidate->usage            == REDRAGON_K652_USAGE)
                {
                    vendor_paths.push_back(candidate->path);
                }

                break;
            }
        }

        if(!topology_entry_found)
        {
            valid = false;
            break;
        }
    }

    if(!valid || collection_count != 9)
    {
        return(false);
    }

    for(unsigned int entry_idx = 0; entry_idx < redragon_k652_topology.size(); entry_idx++)
    {
        if(counts[entry_idx] != redragon_k652_topology[entry_idx].count)
        {
            return(false);
        }
    }

    std::sort(vendor_paths.begin(), vendor_paths.end());

    if(vendor_paths.size() != 4
    || std::adjacent_find(vendor_paths.begin(), vendor_paths.end()) != vendor_paths.end())
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Four vendor collections share the detector IPU.       |
    | Only the lexicographically first path may construct a  |
    | controller, preventing duplicate detection.            |
    \*-----------------------------------------------------*/
    return(vendor_paths.front() == info->path);
}

static bool RevalidateRedragonK652Collection(redragon_k652_hid_handle& handle, const std::string& expected_path,
                                             const GUID& expected_container_id)
{
    const hid_device_info* opened_info = hid_get_device_info(handle.get());
    GUID                   opened_container_id;
    const GUID             null_container_id = {};

    return(IsRedragonK652Identity(opened_info)
        && opened_info->interface_number == REDRAGON_K652_INTERFACE
        && opened_info->usage_page       == REDRAGON_K652_USAGE_PAGE
        && opened_info->usage            == REDRAGON_K652_USAGE
        && opened_info->path
        && std::strcmp(opened_info->path, expected_path.c_str()) == 0
        && hid_winapi_get_container_id(handle.get(), &opened_container_id) == 0
        && !IsEqualGUID(opened_container_id, null_container_id)
        && IsEqualGUID(opened_container_id, expected_container_id));
}

static DetectedControllers DetectRedragonK652Impl(hid_device_info* info, const std::string& name)
{
    DetectedControllers detected_controllers;

    if(!IsRedragonK652Identity(info)
    || info->interface_number != REDRAGON_K652_INTERFACE
    || info->usage_page       != REDRAGON_K652_USAGE_PAGE
    || info->usage            != REDRAGON_K652_USAGE)
    {
        return(detected_controllers);
    }

    std::vector<std::string> vendor_paths;
    GUID                     container_id;

    if(!ValidateRedragonK652Topology(info, vendor_paths, container_id))
    {
        LOG_DEBUG("[Redragon K652] Passive fingerprint rejected HID path %s", info->path);
        return(detected_controllers);
    }

    std::vector<redragon_k652_hid_handle> handles;
    handles.reserve(vendor_paths.size());

    for(const std::string& path : vendor_paths)
    {
        redragon_k652_hid_handle handle(hid_open_path(path.c_str()));

        if(!handle)
        {
            return(detected_controllers);
        }

        handles.push_back(std::move(handle));
    }

    std::size_t command_idx = handles.size();
    std::size_t data_idx    = handles.size();

    for(std::size_t handle_idx = 0; handle_idx < handles.size(); handle_idx++)
    {
        redragon_k652_feature_layout layout;
        std::size_t                  descriptor_maximum;
        std::size_t                  native_maximum;

        if(!GetRedragonK652FeatureLayout(handles[handle_idx].get(), layout)
        || !GetRedragonK652MaximumFeatureReportSize(layout, descriptor_maximum)
        || !GetRedragonK652FeatureReportByteLength(vendor_paths[handle_idx].c_str(), native_maximum)
        || descriptor_maximum != native_maximum)
        {
            return(detected_controllers);
        }

        if(native_maximum == REDRAGON_K652_COMMAND_REPORT_SIZE
        && IsRedragonK652FeatureRole(layout, REDRAGON_K652_COMMAND_REPORT_ID,
                                    REDRAGON_K652_COMMAND_REPORT_SIZE, descriptor_maximum))
        {
            if(command_idx != handles.size())
            {
                return(detected_controllers);
            }

            command_idx = handle_idx;
        }

        if(native_maximum == redragon_k652::REPORT_SIZE
        && IsRedragonK652FeatureRole(layout, REDRAGON_K652_DATA_REPORT_ID,
                                    redragon_k652::REPORT_SIZE, descriptor_maximum))
        {
            if(data_idx != handles.size())
            {
                return(detected_controllers);
            }

            data_idx = handle_idx;
        }
    }

    /*-----------------------------------------------------------------*\
    | The captures do not associate Windows COLxx paths with protocol    |
    | roles.  Report IDs plus descriptor and native HIDP_CAPS lengths do, |
    | so require exactly one distinct match for each role before I/O.     |
    \*-----------------------------------------------------------------*/
    if(command_idx == handles.size() || data_idx == handles.size() || command_idx == data_idx)
    {
        return(detected_controllers);
    }

    /*-----------------------------------------------------------------*\
    | Bind the passive fingerprint to the exact retained handles.  All  |
    | metadata and container values are cached/read-only; no HID report  |
    | is issued.  This closes a disconnect/path-reuse race before the    |
    | single active validation query.                                    |
    \*-----------------------------------------------------------------*/
    for(std::size_t handle_idx = 0; handle_idx < handles.size(); handle_idx++)
    {
        if(!RevalidateRedragonK652Collection(handles[handle_idx], vendor_paths[handle_idx], container_id))
        {
            return(detected_controllers);
        }
    }

    hid_device* command_handle = handles[command_idx].get();
    hid_device* data_handle    = handles[data_idx].get();

    /*-------------------------------------------------------------*\
    | Hotplug enumeration can present the same physical container   |
    | more than once.  Reserve it before the active query so only one|
    | callback can validate and register a controller instance.      |
    \*-------------------------------------------------------------*/
    RedragonK652DeviceClaim device_claim = RedragonK652DeviceRegistry::Claim(&container_id, sizeof(container_id));

    if(!device_claim)
    {
        return(detected_controllers);
    }

    /*-----------------------------------------------------*\
    | This is the only active validation write.  It is sent  |
    | exactly once, after the passive identity, topology, and  |
    | unique native report roles have matched, and is copied  |
    | byte-for-byte from the official Redragon capture.       |
    \*-----------------------------------------------------*/
    if(!RedragonK652Controller::ValidateConfiguration(command_handle, data_handle))
    {
        return(detected_controllers);
    }

    /* Transfer ownership only after each allocation succeeds. */
    std::unique_ptr<RedragonK652Controller> controller = std::make_unique<RedragonK652Controller>
        (command_handle, data_handle, info->path, name, std::move(device_claim));

    handles[command_idx].release();
    handles[data_idx].release();

    std::unique_ptr<RGBController> rgb_controller = std::make_unique<RGBController_RedragonK652>(controller.get());
    controller.release();

    detected_controllers.push_back(rgb_controller.get());
    rgb_controller.release();
    return(detected_controllers);
}

DetectedControllers DetectRedragonK652(hid_device_info* info, const std::string& name)
{
    try
    {
        return(DetectRedragonK652Impl(info, name));
    }
    catch(const std::exception& exception)
    {
        LOG_ERROR("[Redragon K652] Detector failed safely: %s", exception.what());
    }
    catch(...)
    {
        LOG_ERROR("[Redragon K652] Detector failed safely with an unknown exception");
    }

    return(DetectedControllers());
}

/*---------------------------------------------------------*\
| VID/PID 258A:0049 is reused by incompatible Redragon      |
| keyboards.  Never replace this exact detector with the    |
| disabled generic Sinowealth/FL Esports F11 detector.      |
\*---------------------------------------------------------*/
REGISTER_HID_DETECTOR_IPU("Redragon K652", DetectRedragonK652, REDRAGON_K652_VID, REDRAGON_K652_PID, REDRAGON_K652_INTERFACE, REDRAGON_K652_USAGE_PAGE, REDRAGON_K652_USAGE);

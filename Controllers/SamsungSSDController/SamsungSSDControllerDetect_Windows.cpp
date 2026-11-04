/*---------------------------------------------------------*\
| SamsungSSDControllerDetect_Windows.cpp                    |
|                                                           |
|   Detector for Samsung SSD heatsink LED (Windows)         |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include <vector>
#include <windows.h>
#include <winioctl.h>
#include "DetectionManager.h"
#include "LogManager.h"
#include "RGBController_SamsungSSD_Windows_Linux.h"
#include "SamsungSSDController_Windows_Linux.h"
#include "scsiapi.h"

/*---------------------------------------------------------*\
| Windows cuts the product name in the device path to 16    |
| characters, so the path only selects candidates           |
\*---------------------------------------------------------*/
#define SAMSUNG_SSD_PATH_VENDOR         "NVMe"
#define SAMSUNG_SSD_PATH_PRODUCT        "Samsung SSD 990"

static std::string DescriptorString(const std::vector<unsigned char>& buffer, DWORD offset)
{
    if(offset == 0 || offset >= buffer.size())
    {
        return("");
    }

    const char* start   = (const char*)buffer.data() + offset;
    size_t      length  = strnlen(start, buffer.size() - offset);
    std::string value(start, length);

    value.erase(value.find_last_not_of(' ') + 1);
    value.erase(0, value.find_first_not_of(' '));

    return(value);
}

/*---------------------------------------------------------*\
| Read the full model from the storage device descriptor.   |
| The query needs no read or write access to the disk.      |
\*---------------------------------------------------------*/
static bool QueryNVMeDevice(const char* path, std::string& model, std::string& serial, std::string& firmware)
{
    HANDLE query_handle = CreateFileA(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if(query_handle == INVALID_HANDLE_VALUE)
    {
        return(false);
    }

    STORAGE_PROPERTY_QUERY      query;
    std::vector<unsigned char>  buffer(1024);
    DWORD                       bytes_returned = 0;

    memset(&query, 0, sizeof(query));
    query.PropertyId    = StorageDeviceProperty;
    query.QueryType     = PropertyStandardQuery;

    BOOL result = DeviceIoControl(query_handle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), buffer.data(), (DWORD)buffer.size(), &bytes_returned, NULL);

    CloseHandle(query_handle);

    if(!result || bytes_returned < sizeof(STORAGE_DEVICE_DESCRIPTOR))
    {
        return(false);
    }

    buffer.resize(bytes_returned);

    STORAGE_DEVICE_DESCRIPTOR* descriptor = (STORAGE_DEVICE_DESCRIPTOR*)buffer.data();

    if(descriptor->BusType != BusTypeNvme)
    {
        return(false);
    }

    model       = DescriptorString(buffer, descriptor->ProductIdOffset);
    serial      = DescriptorString(buffer, descriptor->SerialNumberOffset);
    firmware    = DescriptorString(buffer, descriptor->ProductRevisionOffset);

    return(true);
}

DetectedControllers DetectSamsungSSDControllers()
{
    DetectedControllers detected_controllers;
    scsi_device_info*   devices = scsi_enumerate(SAMSUNG_SSD_PATH_VENDOR, SAMSUNG_SSD_PATH_PRODUCT);

    for(scsi_device_info* info = devices; info != NULL; info = info->next)
    {
        std::string model;
        std::string serial;
        std::string firmware;

        if(!QueryNVMeDevice(info->path, model, serial, firmware))
        {
            continue;
        }

        LOG_DEBUG("[Samsung SSD] Probing %s, model: %s", info->path, model.c_str());

        if(model.compare(0, strlen(SAMSUNG_SSD_990_PRO_HEATSINK), SAMSUNG_SSD_990_PRO_HEATSINK) != 0)
        {
            continue;
        }

        HANDLE dev_handle = CreateFileA(info->path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if(dev_handle == INVALID_HANDLE_VALUE)
        {
            LOG_WARNING("[Samsung SSD] Cannot open %s (error %lu), administrator rights are necessary", model.c_str(), GetLastError());
            continue;
        }

        SamsungSSDController*     controller     = new SamsungSSDController(dev_handle, info->path, model, serial, firmware);
        RGBController_SamsungSSD* rgb_controller = new RGBController_SamsungSSD(controller);

        detected_controllers.push_back(rgb_controller);
    }

    scsi_free_enumeration(devices);

    return(detected_controllers);
}

REGISTER_DETECTOR("Samsung SSD", DetectSamsungSSDControllers);

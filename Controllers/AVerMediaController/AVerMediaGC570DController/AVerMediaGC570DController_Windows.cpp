/*---------------------------------------------------------*\
| AVerMediaGC570DController_Windows.cpp                     |
|                                                           |
|   Driver for AVerMedia Live Gamer DUO (GC570D) (Windows)  |
|                                                           |
|   Niaz (derneuere)                             24 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cctype>
#include <cstring>
#include <string>
#include <windows.h>
#include <setupapi.h>

#include "AVerMediaGC570DController_Windows.h"
#include "LogManager.h"
#include "StringUtils.h"

#define IOCTL_KS_PROPERTY                   0x002F0003

#define KSPROPERTY_TYPE_GET                 0x00000001
#define KSPROPERTY_TYPE_SET                 0x00000002
#define KSPROPERTY_TYPE_BASICSUPPORT        0x00000200

#define KSPROPERTY_SUPPORT_GET              0x00000001
#define KSPROPERTY_SUPPORT_SET              0x00000002

#define AVERMEDIA_LED_PROPERTY_PATTERN      0
#define AVERMEDIA_LED_PROPERTY_COLOR        8

#define AVERMEDIA_KS_HEADER_SIZE            24
#define AVERMEDIA_KS_PAYLOAD_SIZE           8
#define AVERMEDIA_KS_BUFFER_SIZE            32

#define AVERMEDIA_GC570D_HARDWARE_ID        "ven_1461&dev_0054"
#define AVERMEDIA_GC570D_LOG_NAME           "AVerMedia Live Gamer DUO"

static const GUID KSCATEGORY_CAPTURE_GUID =
{
    0x65E8773D, 0x8F56, 0x11D0, { 0xA3, 0xB9, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96 }
};

static const GUID AVERMEDIA_LED_PROPERTY_SET =
{
    0x1F92A5BF, 0xFFBC, 0x4093, { 0x9C, 0x57, 0xBF, 0xC0, 0x23, 0x72, 0x89, 0xDF }
};

struct AVerMediaKSProperty
{
    GUID            Set;
    unsigned int    Id;
    unsigned int    Flags;
};

AVerMediaGC570DController::AVerMediaGC570DController()
{
    device_handle = INVALID_HANDLE_VALUE;
}

AVerMediaGC570DController::~AVerMediaGC570DController()
{
    if(device_handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(device_handle);
    }
}

bool AVerMediaGC570DController::Initialize()
{
    HDEVINFO device_info = SetupDiGetClassDevsW(&KSCATEGORY_CAPTURE_GUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if(device_info == INVALID_HANDLE_VALUE)
    {
        LOG_ERROR("[%s] SetupDiGetClassDevs failed: %lu", AVERMEDIA_GC570D_LOG_NAME, GetLastError());

        return(false);
    }

    LOG_DEBUG("[%s] Looking for capture filter matching %s", AVERMEDIA_GC570D_LOG_NAME, AVERMEDIA_GC570D_HARDWARE_ID);

    SP_DEVICE_INTERFACE_DATA interface_data;
    interface_data.cbSize   = sizeof(SP_DEVICE_INTERFACE_DATA);

    for(unsigned int interface_idx = 0; SetupDiEnumDeviceInterfaces(device_info, NULL, &KSCATEGORY_CAPTURE_GUID, interface_idx, &interface_data); interface_idx++)
    {
        DWORD required_size = 0;

        SetupDiGetDeviceInterfaceDetailW(device_info, &interface_data, NULL, 0, &required_size, NULL);

        if(required_size == 0)
        {
            continue;
        }

        PSP_DEVICE_INTERFACE_DETAIL_DATA_W detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W)new unsigned char[required_size];
        detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W);

        if(SetupDiGetDeviceInterfaceDetailW(device_info, &interface_data, detail_data, required_size, NULL, NULL))
        {
            std::string path         = StringUtils::wchar_to_string(detail_data->DevicePath);
            std::string path_lower   = path;

            for(unsigned int char_idx = 0; char_idx < path_lower.size(); char_idx++)
            {
                path_lower[char_idx] = (char)tolower((unsigned char)path_lower[char_idx]);
            }

            if(path_lower.find(AVERMEDIA_GC570D_HARDWARE_ID) != std::string::npos)
            {
                HANDLE handle = CreateFileW(detail_data->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

                if(handle == INVALID_HANDLE_VALUE)
                {
                    LOG_WARNING("[%s] Failed to open %s: %lu", AVERMEDIA_GC570D_LOG_NAME, path.c_str(), GetLastError());
                }
                else
                {
                    device_handle   = handle;
                    device_path     = path;

                    if(PropertySupported(AVERMEDIA_LED_PROPERTY_PATTERN) && PropertySupported(AVERMEDIA_LED_PROPERTY_COLOR))
                    {
                        LOG_INFO("[%s] Device successfully detected at %s", AVERMEDIA_GC570D_LOG_NAME, path.c_str());

                        delete[] (unsigned char*)detail_data;
                        SetupDiDestroyDeviceInfoList(device_info);
                        return(true);
                    }

                    LOG_WARNING("[%s] Filter %s does not expose the lighting property set", AVERMEDIA_GC570D_LOG_NAME, path.c_str());

                    CloseHandle(device_handle);
                    device_handle = INVALID_HANDLE_VALUE;
                    device_path.clear();
                }
            }
        }

        delete[] (unsigned char*)detail_data;
    }

    SetupDiDestroyDeviceInfoList(device_info);

    LOG_WARNING("[%s] No matching device found", AVERMEDIA_GC570D_LOG_NAME);

    return(false);
}

std::string AVerMediaGC570DController::GetDeviceName()
{
    return("Live Gamer DUO");
}

std::string AVerMediaGC570DController::GetDeviceLocation()
{
    return("PCI: " + device_path);
}

bool AVerMediaGC570DController::PropertySupported(unsigned int property_id)
{
    AVerMediaKSProperty property;

    property.Set    = AVERMEDIA_LED_PROPERTY_SET;
    property.Id     = property_id;
    property.Flags  = KSPROPERTY_TYPE_BASICSUPPORT;

    unsigned int    support     = 0;
    DWORD           returned    = 0;

    if(!DeviceIoControl(device_handle, IOCTL_KS_PROPERTY, &property, sizeof(property), &support, sizeof(support), &returned, NULL))
    {
        return(false);
    }

    return((support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != 0);
}

bool AVerMediaGC570DController::SetProperty(unsigned int property_id, const unsigned char* payload)
{
    unsigned char   request[AVERMEDIA_KS_BUFFER_SIZE];
    unsigned char   data[AVERMEDIA_KS_BUFFER_SIZE];

    AVerMediaKSProperty property;

    property.Set    = AVERMEDIA_LED_PROPERTY_SET;
    property.Id     = property_id;
    property.Flags  = KSPROPERTY_TYPE_SET;

    memset(request, 0x00, sizeof(request));
    memset(data,    0x00, sizeof(data));
    memcpy(request, &property, AVERMEDIA_KS_HEADER_SIZE);
    memcpy(&request[AVERMEDIA_KS_HEADER_SIZE], payload, AVERMEDIA_KS_PAYLOAD_SIZE);
    memcpy(&data[AVERMEDIA_KS_HEADER_SIZE],    payload, AVERMEDIA_KS_PAYLOAD_SIZE);

    DWORD returned = 0;

    return(DeviceIoControl(device_handle, IOCTL_KS_PROPERTY, request, sizeof(request), data, sizeof(data), &returned, NULL) != FALSE);
}

bool AVerMediaGC570DController::GetProperty(unsigned int property_id, unsigned char* payload)
{
    unsigned char   request[AVERMEDIA_KS_BUFFER_SIZE];
    unsigned char   data[AVERMEDIA_KS_BUFFER_SIZE];

    AVerMediaKSProperty property;

    property.Set    = AVERMEDIA_LED_PROPERTY_SET;
    property.Id     = property_id;
    property.Flags  = KSPROPERTY_TYPE_GET;

    memset(request, 0x00, sizeof(request));
    memset(data,    0x00, sizeof(data));
    memcpy(request, &property, AVERMEDIA_KS_HEADER_SIZE);

    DWORD returned = 0;

    if(!DeviceIoControl(device_handle, IOCTL_KS_PROPERTY, request, sizeof(request), data, sizeof(data), &returned, NULL))
    {
        return(false);
    }

    memcpy(payload, &data[AVERMEDIA_KS_HEADER_SIZE], AVERMEDIA_KS_PAYLOAD_SIZE);

    return(true);
}

void AVerMediaGC570DController::SetColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char speed)
{
    unsigned char payload[AVERMEDIA_KS_PAYLOAD_SIZE];

    memset(payload, 0x00, sizeof(payload));

    payload[0] = red;
    payload[1] = green;
    payload[2] = blue;
    payload[3] = speed;

    SetProperty(AVERMEDIA_LED_PROPERTY_COLOR, payload);
}

void AVerMediaGC570DController::SetMode(unsigned char pattern)
{
    unsigned char payload[AVERMEDIA_KS_PAYLOAD_SIZE];

    memset(payload, 0x00, sizeof(payload));

    payload[0] = pattern;

    SetProperty(AVERMEDIA_LED_PROPERTY_PATTERN, payload);
}

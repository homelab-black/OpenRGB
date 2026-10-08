/*---------------------------------------------------------*\
| RGBController_Proxy.h                                     |
|                                                           |
|   Base class for wrapper (proxy) controllers              |
|   Every call is passed to the managed controller          |
|                                                           |
|   Dmitry Kychanov (k1-801)                    07 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_Proxy.h"

RGBController_Proxy::RGBController_Proxy(device_type placeholder_type, const std::string& placeholder_name, RGBController* target)
{
    target_controller = target;
    name              = placeholder_name;
    type              = placeholder_type;

    if(name.empty())
    {
        name = "Placeholder ";
        name.append(DeviceTypeToString(type));
    }

    /*-----------------------------------------------------*\
    | Proxy controller doesn't actually use its background  |
    | thread, so shut it down immediately to save resources |
    | If managed controller needs it's own thread, it will  |
    | create one of it's own and use that.                  |
    \*-----------------------------------------------------*/
    Shutdown();
    AccessMutex.unlock();
}

RGBController_Proxy::~RGBController_Proxy()
{
    if(target_controller)
    {
        delete target_controller;
    }
    target_controller = nullptr;
}

/*-----------------------------------------------------*\
| Controller Information                                |
\*-----------------------------------------------------*/
std::string RGBController_Proxy::GetName()
{
    if(target_controller)
    {
        return target_controller->GetName();
    }
    return name; // Return placeholder name from constructor
}

std::string RGBController_Proxy::GetVendor()
{
    if(target_controller)
    {
        return target_controller->GetVendor();
    }
    return vendor; // Empty by default
}

std::string RGBController_Proxy::GetDescription()
{
    if(target_controller)
    {
        return target_controller->GetDescription();
    }
    return description; // Empty by default
}

std::string RGBController_Proxy::GetVersion()
{
    if(target_controller)
    {
        return target_controller->GetVersion();
    }
    return version; // Empty by default
}

std::string RGBController_Proxy::GetSerial()
{
    if(target_controller)
    {
        return target_controller->GetSerial();
    }
    return serial; // Empty by default
}

std::string RGBController_Proxy::GetLocation()
{
    if(target_controller)
    {
        return target_controller->GetLocation();
    }
    return location; // Might actually be filled with HID path
}

std::string RGBController_Proxy::GetDisplayName()
{
    if(target_controller)
    {
        return target_controller->GetDisplayName();
    }
    std::string result;
    AccessMutex.lock();
    result = display_name.empty() ? name : display_name;
    AccessMutex.unlock();
    return result;
}

device_type RGBController_Proxy::GetDeviceType()
{
    if(target_controller)
    {
        return target_controller->GetDeviceType();
    }
    return type; // Placeholder value from constructor
}

controller_flags RGBController_Proxy::GetFlags()
{
    if(target_controller)
    {
        return target_controller->GetFlags();
    }
    return flags; // Default value
}

/*-----------------------------------------------------*\
| Zone Functions                                        |
\*-----------------------------------------------------*/
zone RGBController_Proxy::GetZone(unsigned int zone_idx)
{
    if(target_controller)
    {
        return target_controller->GetZone(zone_idx);
    }
    return zone(); // Should not be reached bc zone count is 0
}

int RGBController_Proxy::GetZoneActiveMode(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneActiveMode(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

RGBColor RGBController_Proxy::GetZoneColor(unsigned int zone, unsigned int color_index)
{
    if(target_controller)
    {
        return target_controller->GetZoneActiveMode(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

RGBColor* RGBController_Proxy::GetZoneColorsPointer(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneColorsPointer(zone);
    }
    return nullptr; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneCount()
{
    if(target_controller)
    {
        return target_controller->GetZoneCount();
    }
    return 0; // Should not be reached bc zone count is 0
}

std::string RGBController_Proxy::GetZoneDisplayName(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneDisplayName(zone);
    }
    return std::string(); // Should not be reached bc zone count is 0
}

zone_flags RGBController_Proxy::GetZoneFlags(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneFlags(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneLEDsCount(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneLEDsCount(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneLEDsMax(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneLEDsMax(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneLEDsMin(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneLEDsMin(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

matrix_map_type RGBController_Proxy::GetZoneMatrixMap(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneMatrixMap(zone);
    }
    return matrix_map_type(); // Should not be reached bc zone count is 0
}

const unsigned int* RGBController_Proxy::GetZoneMatrixMapData(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneMatrixMapData(zone);
    }
    return nullptr; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneMatrixMapHeight(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneMatrixMapHeight(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneMatrixMapWidth(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneMatrixMapWidth(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeCount(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeCount(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeBrightness(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeBrightness(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeBrightnessMax(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeBrightnessMax(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeBrightnessMin(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeBrightnessMin(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

RGBColor RGBController_Proxy::GetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeColor(zone, mode, color_index);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeColorMode(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeColorMode(zone, mode);
    }
    return MODE_COLORS_PER_LED; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeColorsCount(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeColorsCount(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeColorsMax(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeColorsMax(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeColorsMin(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeColorsMin(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeDirection(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeDirection(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeFlags(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeFlags(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

std::string RGBController_Proxy::GetZoneModeName(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeName(zone, mode);
    }
    return std::string(); // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeSpeed(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeSpeed(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeSpeedMax(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeSpeedMax(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneModeSpeedMin(unsigned int zone, unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetZoneModeSpeedMin(zone, mode);
    }
    return 0; // Should not be reached bc zone count is 0
}

std::string RGBController_Proxy::GetZoneName(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneName(zone);
    }
    return std::string(); // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentCount(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentCount(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

segment_flags RGBController_Proxy::GetZoneSegmentFlags(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentFlags(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentLEDsCount(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentLEDsCount(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

matrix_map_type RGBController_Proxy::GetZoneSegmentMatrixMap(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentMatrixMap(zone, segment);
    }
    return matrix_map_type(); // Should not be reached bc zone count is 0
}

const unsigned int * RGBController_Proxy::GetZoneSegmentMatrixMapData(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentMatrixMapData(zone, segment);
    }
    return nullptr; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentMatrixMapHeight(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentMatrixMapHeight(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentMatrixMapWidth(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentMatrixMapWidth(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

std::string RGBController_Proxy::GetZoneSegmentName(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentName(zone, segment);
    }
    return std::string(); // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentStartIndex(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentStartIndex(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneSegmentType(unsigned int zone, unsigned int segment)
{
    if(target_controller)
    {
        return target_controller->GetZoneSegmentType(zone, segment);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetZoneStartIndex(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneStartIndex(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

zone_type RGBController_Proxy::GetZoneType(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetZoneType(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

unsigned int RGBController_Proxy::GetLEDsInZone(unsigned int zone)
{
    if(target_controller)
    {
        return target_controller->GetLEDsInZone(zone);
    }
    return 0; // Should not be reached bc zone count is 0
}

void RGBController_Proxy::SetZoneActiveMode(unsigned int zone, int mode)
{
    if(target_controller)
    {
        target_controller->SetZoneActiveMode(zone, mode);
    }
}

void RGBController_Proxy::SetZoneColor(unsigned int zone, unsigned int color_index, RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetZoneColor(zone, color_index, color);
    }
}

void RGBController_Proxy::SetZoneModeBrightness(unsigned int zone, unsigned int mode, unsigned int brightness)
{
    if(target_controller)
    {
        target_controller->SetZoneModeBrightness(zone, mode, brightness);
    }
}

void RGBController_Proxy::SetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index, RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetZoneModeColor(zone, mode, color_index, color);
    }
}

void RGBController_Proxy::SetZoneModeColorMode(unsigned int zone, unsigned int mode, unsigned int color_mode)
{
    if(target_controller)
    {
        target_controller->SetZoneModeColorMode(zone, mode, color_mode);
    }
}

void RGBController_Proxy::SetZoneModeColorsCount(unsigned int zone, unsigned int mode, unsigned int count)
{
    if(target_controller)
    {
        target_controller->SetZoneModeColorsCount(zone, mode, count);
    }
}

void RGBController_Proxy::SetZoneModeDirection(unsigned int zone, unsigned int mode, unsigned int direction)
{
    if(target_controller)
    {
        target_controller->SetZoneModeDirection(zone, mode, direction);
    }
}

void RGBController_Proxy::SetZoneModeSpeed(unsigned int zone, unsigned int mode, unsigned int speed)
{
    if(target_controller)
    {
        target_controller->SetZoneModeSpeed(zone, mode, speed);
    }
}

bool RGBController_Proxy::SupportsPerZoneModes()
{
    if(target_controller)
    {
        return target_controller->SupportsPerZoneModes();
    }
    return false;
}

/*-----------------------------------------------------*\
| Mode Functions                                        |
\*-----------------------------------------------------*/
unsigned int RGBController_Proxy::GetModeCount()
{
    if(target_controller)
    {
        return target_controller->GetModeCount();
    }
    return 0;
}

unsigned int RGBController_Proxy::GetModeBrightness(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeBrightness(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeBrightnessMax(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeBrightnessMax(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeBrightnessMin(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeBrightnessMin(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

RGBColor RGBController_Proxy::GetModeColor(unsigned int mode, unsigned int color_index)
{
    if(target_controller)
    {
        return target_controller->GetModeColor(mode, color_index);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeColorMode(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeColorMode(mode);
    }
    return MODE_COLORS_PER_LED; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeColorsCount(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeColorsCount(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeColorsMax(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeColorsMax(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeColorsMin(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeColorsMin(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeDirection(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeDirection(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeFlags(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeFlags(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

std::string RGBController_Proxy::GetModeName(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeName(mode);
    }
    return std::string(); // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeSpeed(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeSpeed(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeSpeedMax(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeSpeedMax(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}

unsigned int RGBController_Proxy::GetModeSpeedMin(unsigned int mode)
{
    if(target_controller)
    {
        return target_controller->GetModeSpeedMin(mode);
    }
    return 0; // Should not be reached bc mode count is 0
}


void RGBController_Proxy::SetModeBrightness(unsigned int mode, unsigned int brightness)
{
    if(target_controller)
    {
        target_controller->SetModeBrightness(mode, brightness);
    }
}

void RGBController_Proxy::SetModeColor(unsigned int mode, unsigned int color_index, RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetModeColor(mode, color_index, color);
    }
}

void RGBController_Proxy::SetModeColorMode(unsigned int mode, unsigned int color_mode)
{
    if(target_controller)
    {
        target_controller->SetModeColorMode(mode, color_mode);
    }
}

void RGBController_Proxy::SetModeColorsCount(unsigned int mode, unsigned int count)
{
    if(target_controller)
    {
        target_controller->SetModeColorsCount(mode, count);
    }
}

void RGBController_Proxy::SetModeDirection(unsigned int mode, unsigned int direction)
{
    if(target_controller)
    {
        return target_controller->SetModeDirection(mode, direction);
    }
}

void RGBController_Proxy::SetModeSpeed(unsigned int mode, unsigned int speed)
{
    if(target_controller)
    {
        target_controller->SetModeSpeed(mode, speed);
    }
}


int RGBController_Proxy::GetActiveMode()
{
    if(target_controller)
    {
        return target_controller->GetActiveMode();
    }
    return 0;
}

void RGBController_Proxy::SetActiveMode(int mode)
{
    if(target_controller)
    {
        target_controller->SetActiveMode(mode);
    }
}

void RGBController_Proxy::SetCustomMode()
{
    if(target_controller)
    {
        target_controller->SetCustomMode();
    }
}


/*-----------------------------------------------------*\
| LED Functions                                         |
\*-----------------------------------------------------*/
unsigned int RGBController_Proxy::GetLEDCount()
{
    if(target_controller)
    {
        return target_controller->GetLEDCount();
    }
    return 0;
}

std::string RGBController_Proxy::GetLEDName(unsigned int led)
{
    if(target_controller)
    {
        return target_controller->GetLEDName(led);
    }
    return std::string(); // Should not be reached bc LED count is 0
}

std::string RGBController_Proxy::GetLEDDisplayName(unsigned int led)
{
    if(target_controller)
    {
        return target_controller->GetLEDDisplayName(led);
    }
    return std::string(); // Should not be reached bc LED count is 0
}

/*-----------------------------------------------------*\
| Color Functions                                       |
\*-----------------------------------------------------*/
RGBColor RGBController_Proxy::GetColor(unsigned int led)
{
    if(target_controller)
    {
        return target_controller->GetColor(led);
    }
    return 0; // Should not be reached bc LED count is 0
}

RGBColor* RGBController_Proxy::GetColorsPointer()
{
    if(target_controller)
    {
        return target_controller->GetColorsPointer();
    }
    return nullptr;
}

void RGBController_Proxy::SetColor(unsigned int led, RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetColor(led, color);
    }
}

void RGBController_Proxy::SetAllColors(RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetAllColors(color);
    }
}

void RGBController_Proxy::SetAllZoneColors(int zone, RGBColor color)
{
    if(target_controller)
    {
        target_controller->SetAllZoneColors(zone, color);
    }
}

/*-----------------------------------------------------*\
| Device-Specific Configuration Functions               |
\*-----------------------------------------------------*/
nlohmann::json RGBController_Proxy::GetDeviceSpecificConfigurationSchema()
{
    if(target_controller)
    {
        return target_controller->GetDeviceSpecificConfigurationSchema();
    }
    return nlohmann::json();
}

nlohmann::json RGBController_Proxy::GetDeviceSpecificConfiguration()
{
    if(target_controller)
    {
        return target_controller->GetDeviceSpecificConfiguration();
    }
    return nlohmann::json();
}

void RGBController_Proxy::SetDeviceSpecificConfiguration(nlohmann::json configuration_json)
{
    if(target_controller)
    {
        target_controller->SetDeviceSpecificConfiguration(configuration_json);
    }
}

nlohmann::json RGBController_Proxy::GetDeviceSpecificZoneConfigurationSchema(int zone)
{
    if(target_controller)
    {
        return target_controller->GetDeviceSpecificZoneConfigurationSchema(zone);
    }
    return nlohmann::json(); // Should not be reached bc zone count is 0
}

nlohmann::json RGBController_Proxy::GetDeviceSpecificZoneConfiguration(int zone)
{
    if(target_controller)
    {
        return target_controller->GetDeviceSpecificZoneConfiguration(zone);
    }
    return nlohmann::json(); // Should not be reached bc zone count is 0
}

void RGBController_Proxy::SetDeviceSpecificZoneConfiguration(int zone, nlohmann::json configuration_json)
{
    if(target_controller)
    {
        target_controller->SetDeviceSpecificZoneConfiguration(zone, configuration_json);
    }
}

/*-----------------------------------------------------*\
| Update Callback Functions                             |
| IMPORTANT: the callbacks will NOT be called for       |
| the proxy object itself! Instead, they are kept and   |
| copied into the managed controller when it is set     |
\*-----------------------------------------------------*/
void RGBController_Proxy::RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg)
{
    RGBController::RegisterUpdateCallback(new_callback, new_callback_arg);
    if(target_controller)
    {
        target_controller->RegisterUpdateCallback(new_callback, new_callback_arg);
    }
}

void RGBController_Proxy::UnregisterUpdateCallback(void * callback_arg)
{
    RGBController::UnregisterUpdateCallback(callback_arg);
    if(target_controller)
    {
        target_controller->UnregisterUpdateCallback(callback_arg);
    }
}

void RGBController_Proxy::ClearCallbacks()
{
    RGBController::ClearCallbacks();
    if(target_controller)
    {
        target_controller->ClearCallbacks();
    }
}

void RGBController_Proxy::SignalUpdate(unsigned int update_reason)
{
    if(target_controller)
    {
        target_controller->SignalUpdate(update_reason);
    }
}

/*-----------------------------------------------------*\
| Device Update Functions                               |
\*-----------------------------------------------------*/
void RGBController_Proxy::UpdateLEDs()
{
    if(target_controller)
    {
        target_controller->UpdateLEDs();
    }
}

void RGBController_Proxy::UpdateZoneLEDs(int zone)
{
    if(target_controller)
    {
        target_controller->UpdateZoneLEDs(zone);
    }
}

void RGBController_Proxy::UpdateSingleLED(int led)
{
    if(target_controller)
    {
        target_controller->UpdateSingleLED(led);
    }
}

void RGBController_Proxy::UpdateMode()
{
    if(target_controller)
    {
        target_controller->UpdateMode();
    }
}

void RGBController_Proxy::UpdateZoneMode(int zone)
{
    if(target_controller)
    {
        target_controller->UpdateZoneMode(zone);
    }
}

void RGBController_Proxy::SaveMode()
{
    if(target_controller)
    {
        target_controller->SaveMode();
    }
}

void RGBController_Proxy::ClearSegments(int zone)
{
    if(target_controller)
    {
        target_controller->ClearSegments(zone);
    }
}

void RGBController_Proxy::AddSegment(int zone, segment new_segment)
{
    if(target_controller)
    {
        target_controller->AddSegment(zone, new_segment);
    }
}

void RGBController_Proxy::ConfigureZone(int zone_idx, zone new_zone)
{
    if(target_controller)
    {
        target_controller->ConfigureZone(zone_idx, new_zone);
    }
}

void RGBController_Proxy::ResizeZone(int zone, int new_size)
{
    if(target_controller)
    {
        target_controller->ResizeZone(zone, new_size);
    }
}

void RGBController_Proxy::ConfigureDevice(controller_flags new_flags, std::string new_name)
{
    if(target_controller)
    {
        target_controller->ConfigureDevice(new_flags, new_name);
    }
}

/*-----------------------------------------------------*\
| Functions to be implemented in device implementation  |
| These are usually called by RGBController's defined   |
| methods that are all redefined in Proxy               |
| These methods should not receive any calls            |
| TODO: REMOVE when migrating to an interface           |
\*-----------------------------------------------------*/
void RGBController_Proxy::DeviceConfigureDevice()
{
}

void RGBController_Proxy::DeviceConfigureZone(int /*zone_idx*/)
{
}

void RGBController_Proxy::DeviceUpdateLEDs()
{
}

void RGBController_Proxy::DeviceUpdateZoneLEDs(int /*zone*/)
{
}

void RGBController_Proxy::DeviceUpdateSingleLED(int /*led*/)
{
}

void RGBController_Proxy::DeviceUpdateMode()
{
}

void RGBController_Proxy::DeviceUpdateZoneMode(int /*zone*/)
{
}

void RGBController_Proxy::DeviceSaveMode()
{
}

void RGBController_Proxy::DeviceUpdateDeviceSpecificConfiguration()
{
}

void RGBController_Proxy::DeviceUpdateDeviceSpecificZoneConfiguration(int /*zone*/)
{
}

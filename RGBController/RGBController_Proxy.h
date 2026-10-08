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

#pragma once

#include "RGBController.h"

class RGBController_Proxy : public RGBController // TODO: replace with RGBControllerInterface when RM allows to register interface instead of actual controller
{
public:
    RGBController_Proxy(device_type placeholder_type, const std::string& placeholder_name = "", RGBController* target = nullptr);
    ~RGBController_Proxy();

    /*-----------------------------------------------------*\
    | Controller Information                                |
    \*-----------------------------------------------------*/
    std::string             GetName();
    std::string             GetVendor();
    std::string             GetDescription();
    std::string             GetVersion();
    std::string             GetSerial();
    std::string             GetLocation();
    std::string             GetDisplayName();

    device_type             GetDeviceType();
    controller_flags        GetFlags();

    /*-----------------------------------------------------*\
    | Note: Hidden Flag Functions are not passed down       |
    \*-----------------------------------------------------*/

    /*-----------------------------------------------------*\
    | Zone Functions                                        |
    \*-----------------------------------------------------*/
    zone                    GetZone(unsigned int zone_idx);
    int                     GetZoneActiveMode(unsigned int zone);
    RGBColor                GetZoneColor(unsigned int zone, unsigned int color_index);
    RGBColor*               GetZoneColorsPointer(unsigned int zone);
    unsigned int            GetZoneCount();
    std::string             GetZoneDisplayName(unsigned int zone);
    zone_flags              GetZoneFlags(unsigned int zone);
    unsigned int            GetZoneLEDsCount(unsigned int zone);
    unsigned int            GetZoneLEDsMax(unsigned int zone);
    unsigned int            GetZoneLEDsMin(unsigned int zone);
    matrix_map_type         GetZoneMatrixMap(unsigned int zone);
    const unsigned int*     GetZoneMatrixMapData(unsigned int zone);
    unsigned int            GetZoneMatrixMapHeight(unsigned int zone);
    unsigned int            GetZoneMatrixMapWidth(unsigned int zone);
    unsigned int            GetZoneModeCount(unsigned int zone);
    unsigned int            GetZoneModeBrightness(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeBrightnessMax(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeBrightnessMin(unsigned int zone, unsigned int mode);
    RGBColor                GetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index);
    unsigned int            GetZoneModeColorMode(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeColorsCount(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeColorsMax(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeColorsMin(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeDirection(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeFlags(unsigned int zone, unsigned int mode);
    std::string             GetZoneModeName(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeSpeed(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeSpeedMax(unsigned int zone, unsigned int mode);
    unsigned int            GetZoneModeSpeedMin(unsigned int zone, unsigned int mode);
    std::string             GetZoneName(unsigned int zone);
    unsigned int            GetZoneSegmentCount(unsigned int zone);
    segment_flags           GetZoneSegmentFlags(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneSegmentLEDsCount(unsigned int zone, unsigned int segment);
    matrix_map_type         GetZoneSegmentMatrixMap(unsigned int zone, unsigned int segment);
    const unsigned int *    GetZoneSegmentMatrixMapData(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneSegmentMatrixMapHeight(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneSegmentMatrixMapWidth(unsigned int zone, unsigned int segment);
    std::string             GetZoneSegmentName(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneSegmentStartIndex(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneSegmentType(unsigned int zone, unsigned int segment);
    unsigned int            GetZoneStartIndex(unsigned int zone);
    zone_type               GetZoneType(unsigned int zone);

    unsigned int            GetLEDsInZone(unsigned int zone);

    void                    SetZoneActiveMode(unsigned int zone, int mode);
    void                    SetZoneColor(unsigned int zone, unsigned int color_index, RGBColor color);
    void                    SetZoneModeBrightness(unsigned int zone, unsigned int mode, unsigned int brightness);
    void                    SetZoneModeColor(unsigned int zone, unsigned int mode, unsigned int color_index, RGBColor color);
    void                    SetZoneModeColorMode(unsigned int zone, unsigned int mode, unsigned int color_mode);
    void                    SetZoneModeColorsCount(unsigned int zone, unsigned int mode, unsigned int count);
    void                    SetZoneModeDirection(unsigned int zone, unsigned int mode, unsigned int direction);
    void                    SetZoneModeSpeed(unsigned int zone, unsigned int mode, unsigned int speed);

    bool                    SupportsPerZoneModes();

    /*-----------------------------------------------------*\
    | Mode Functions                                        |
    \*-----------------------------------------------------*/
    unsigned int            GetModeCount();
    unsigned int            GetModeBrightness(unsigned int mode);
    unsigned int            GetModeBrightnessMax(unsigned int mode);
    unsigned int            GetModeBrightnessMin(unsigned int mode);
    RGBColor                GetModeColor(unsigned int mode, unsigned int color_index);
    unsigned int            GetModeColorMode(unsigned int mode);
    unsigned int            GetModeColorsCount(unsigned int mode);
    unsigned int            GetModeColorsMax(unsigned int mode);
    unsigned int            GetModeColorsMin(unsigned int mode);
    unsigned int            GetModeDirection(unsigned int mode);
    unsigned int            GetModeFlags(unsigned int mode);
    std::string             GetModeName(unsigned int mode);
    unsigned int            GetModeSpeed(unsigned int mode);
    unsigned int            GetModeSpeedMax(unsigned int mode);
    unsigned int            GetModeSpeedMin(unsigned int mode);

    void                    SetModeBrightness(unsigned int mode, unsigned int brightness);
    void                    SetModeColor(unsigned int mode, unsigned int color_index, RGBColor color);
    void                    SetModeColorMode(unsigned int mode, unsigned int color_mode);
    void                    SetModeColorsCount(unsigned int mode, unsigned int count);
    void                    SetModeDirection(unsigned int mode, unsigned int direction);
    void                    SetModeSpeed(unsigned int mode, unsigned int speed);

    int                     GetActiveMode();
    void                    SetActiveMode(int mode);
    void                    SetCustomMode();

    /*-----------------------------------------------------*\
    | LED Functions                                         |
    \*-----------------------------------------------------*/
    unsigned int            GetLEDCount();
    std::string             GetLEDName(unsigned int led);
    std::string             GetLEDDisplayName(unsigned int led);

    /*-----------------------------------------------------*\
    | Color Functions                                       |
    \*-----------------------------------------------------*/
    RGBColor                GetColor(unsigned int led);
    RGBColor*               GetColorsPointer();
    void                    SetColor(unsigned int led, RGBColor color);
    void                    SetAllColors(RGBColor color);
    void                    SetAllZoneColors(int zone, RGBColor color);

    /*-----------------------------------------------------*\
    | Device-Specific Configuration Functions               |
    \*-----------------------------------------------------*/
    nlohmann::json          GetDeviceSpecificConfigurationSchema();
    nlohmann::json          GetDeviceSpecificConfiguration();
    void                    SetDeviceSpecificConfiguration(nlohmann::json configuration_json);

    nlohmann::json          GetDeviceSpecificZoneConfigurationSchema(int zone);
    nlohmann::json          GetDeviceSpecificZoneConfiguration(int zone);
    void                    SetDeviceSpecificZoneConfiguration(int zone, nlohmann::json configuration_json);

    /*-----------------------------------------------------*\
    | Update Callback Functions                             |
    \*-----------------------------------------------------*/
    void                    RegisterUpdateCallback(RGBControllerCallback new_callback, void * new_callback_arg);
    void                    UnregisterUpdateCallback(void * callback_arg);
    void                    ClearCallbacks();
    void                    SignalUpdate(unsigned int update_reason);

    /*-----------------------------------------------------*\
    | Device Update Functions                               |
    \*-----------------------------------------------------*/
    void                    UpdateLEDs();
    void                    UpdateZoneLEDs(int zone);
    void                    UpdateSingleLED(int led);

    void                    UpdateMode();
    void                    UpdateZoneMode(int zone);
    void                    SaveMode();

    void                    DeviceCallThreadFunction();

    void                    ClearSegments(int zone);
    void                    AddSegment(int zone, segment new_segment);

    void                    ConfigureZone(int zone_idx, zone new_zone);
    void                    ResizeZone(int zone, int new_size);

    void                    ConfigureDevice(controller_flags new_flags, std::string new_name);

    /*-----------------------------------------------------*\
    | Functions to be implemented in device implementation  |
    | Remove when migrating to an interface                 |
    \*-----------------------------------------------------*/
    void            DeviceConfigureDevice();
    void            DeviceConfigureZone(int zone_idx);

    void            DeviceUpdateLEDs();
    void            DeviceUpdateZoneLEDs(int zone);
    void            DeviceUpdateSingleLED(int led);

    void            DeviceUpdateMode();
    void            DeviceUpdateZoneMode(int zone);
    void            DeviceSaveMode();

    void            DeviceUpdateDeviceSpecificConfiguration();
    void            DeviceUpdateDeviceSpecificZoneConfiguration(int zone);

protected:
    RGBController*         target_controller;
};

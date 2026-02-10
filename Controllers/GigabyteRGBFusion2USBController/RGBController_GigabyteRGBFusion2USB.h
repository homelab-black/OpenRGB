/*---------------------------------------------------------*\
| RGBController_GigabyteRGBFusion2USB.h                     |
|                                                           |
|   RGBController for Gigabyte Aorus RGB Fusion 2 USB       |
|   motherboard                                             |
|                                                           |
|   jackun                                      08 Jan 2020 |
|   megadjc                                     31 Jul 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <map>
#include "RGBController.h"
#include "GigabyteFusion2USB_Devices.h"
#include "GigabyteRGBFusion2USBController.h"
#include "SettingsManager.h"

#define RGBFUSION2_DIGITAL_LEDS_MIN         0
#define RGBFUSION2_DIGITAL_LEDS_MAX         1024
#define RGBFUSION2_BRIGHTNESS_MIN           0
#define RGBFUSION2_BRIGHTNESS_MAX           255
#define RGBFUSION2_SPEED_MIN                9
#define RGBFUSION2_SPEED_MID                4
#define RGBFUSION2_SPEED_MAX                0

template<typename K, typename V>
static std::map<V, K> reverse_map(const std::map<K, V>& map)
{
    std::map<V, K> reversed_map;

    for(const std::pair<K, V> entry : map)
    {
        reversed_map[entry.second] = entry.first;
    }

    return reversed_map;
}

struct CustomLayout
{
    std::string name;
    std::string header;
    uint16_t leds_min;
    uint16_t leds_max;
};

using custom_layout_map = std::unordered_map<std::string, std::vector<CustomLayout>>;

struct CustomLayoutSettings
{
    bool enabled;
    custom_layout_map layouts;
};

void to_json(json& j, const CustomLayoutSettings& settings);
void from_json(const json& j, CustomLayoutSettings& settings);

struct CalibrationData
{
    std::string hdr_d_led1;
    std::string hdr_d_led2;
    std::string hdr_d_led3;
    std::string hdr_d_led4;
    std::string mainboard;
    std::string spare0;
    std::string spare1;
    std::string spare2;
    std::string spare3;
};

struct CalibrationSettings
{
    bool enabled;
    CalibrationData data;
};

void to_json(json& j, const CalibrationSettings& settings);
void from_json(const json& j, CalibrationSettings& settings);

struct RGBFusion2USBSettings
{
    CustomLayoutSettings custom_layout;
    CalibrationSettings calibration_settings;
};

void to_json(json& j, const RGBFusion2USBSettings& settings);
void from_json(const json& j, RGBFusion2USBSettings& settings);


class RGBController_RGBFusion2USB: public RGBController
{
public:
    RGBController_RGBFusion2USB(RGBFusion2USBController* controller_ptr, std::string _detector_name);
    ~RGBController_RGBFusion2USB();

    void                        SetupZones();

    void                        DeviceResizeZone(int zone, int new_size);

    void                        DeviceUpdateLEDs();
    void                        DeviceUpdateZoneLEDs(int zone);
    void                        DeviceUpdateSingleLED(int led);

    void                        DeviceUpdateMode();

private:
    std::string                 detector_name;

    RGBFusion2USBController*    controller;
    int                         device_num;
    RGBColor                    null_color      = 0;
    /*---------------------------------------------------------*\
    | The intial value of device_index should point to the      |
    |   layout for the generic_device                           |
    \*---------------------------------------------------------*/
    uint32_t                    device_index    = 0;

    void                        Init_Controller();
    int                         GetLED_Zone(int led_idx);

    CalibrationData             WriteCalJsonFrom(
                                    const EncodedCalibration& src);
    void                        FillMissingWith(
                                    CalibrationData& dst,
                                    const EncodedCalibration& fb);
    custom_layout_map           BuildCustomLayoutJson(
                                    const gb_fusion2_device* layout,
                                    const RvrseLedHeaders& reverseLookup);
    void                        LoadCustomLayoutFromJson(
                                    const CustomLayoutSettings& json_custom,
                                    const FwdLedHeaders& forwardLookup,
                                    gb_fusion2_device* layout);
};

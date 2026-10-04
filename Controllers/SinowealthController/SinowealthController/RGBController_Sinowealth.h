/*---------------------------------------------------------*\
| RGBController_Sinowealth.h                                |
|                                                           |
|   RGBController for Sinowealth mice, including Glorious   |
|                                                           |
|   Niels Westphal (crashniels)                 20 May 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SinowealthController.h"

#include <string>

class RGBController_Sinowealth : public RGBController
{
public:
    RGBController_Sinowealth(SinowealthController* controller_ptr, std::function<void()> shutdown_callback);
    ~RGBController_Sinowealth();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        DeviceUpdateDeviceSpecificConfiguration();
    void        DeviceUpdateDeviceSpecificZoneConfiguration(int zone_idx);

    nlohmann::json GetDeviceSpecificConfigurationSchema();
    nlohmann::json GetDeviceSpecificConfiguration();

private:
    SinowealthController*      controller;
    std::function<void()>      shutdown_callback;

    unsigned char              sensor_id;
    std::string                sensor_model;
    int                        last_mode;
    std::vector<unsigned char> dpi;
    unsigned char              dpi_mask;
    unsigned int               active_dpi_preset;

    void                       setupLeds();
    void                       initDeviceSpecificConfiguration();
    void                       applyDeviceSpecificConfiguration(const nlohmann::json &);

    void                       setColorOrder(const std::string& order);
    std::string                getColorOrder() const;
    uint32_t                   reorderColor(RGBColor in);
    RGBColor                   reorderColorReverse(uint32_t in);
    uint32_t                   color_order;

    void                       loadDeviceConfig();
};

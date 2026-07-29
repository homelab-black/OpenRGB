/*---------------------------------------------------------*\
| MSIMysticLight761Controller.h                             |
|                                                           |
|   Driver for MSI Mystic Light 761-byte motherboard        |
|                                                           |
|   Direct mode functionality has been implemented based on |
|   the SignalRGB project                                   |
|   (https://signalrgb.com/)                                |
|                                                           |
|   rom4ster                                    11 Jun 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstring>
#include <hidapi.h>
#include "MSIMysticLightCommon.h"
#include "RGBController.h"

inline constexpr const char * BOARD_UNSUPPORTED_ERROR =  "No Config Found For Board";

// Some boards (B860 so far) don't understand the per-LED report (0x51), only this
// 290-byte report (0x50, 18x16-byte zone blocks). One solid color per zone, no per-LED
// addressing. param14 also picks a couple of onboard effects but isn't a clean speed/
// bitmask - see #5600.
struct StaticZoneBlock
{
    unsigned char effect        = 0x01;                                 // 0x01 = onboard effect, 0x02 = direct color
    Color         color         { 0, 0, 0 };
    unsigned char reserved1[5]  = { 0x00, 0xFF, 0x00, 0x00, 0x00 };      // unknown, constant
    unsigned char reserved2[4]  = { 0xFF, 0xFF, 0xFF, 0xFF };            // unknown, constant
    unsigned char effect_param  = 0x03;                                 // onboard effect id, 0x00 for direct
    unsigned char param14       = 0x14;                                 // cycle/static/off toggle, see above
    unsigned char param15       = 0x1E;                                 // constant
};

struct FeaturePacket_StaticZone_761
{
    unsigned char    report_id  = 0x50;
    StaticZoneBlock  blocks[18];
    unsigned char    trailing   = 0x00;
};

class MSIMysticLight761Controller
{
public:
    MSIMysticLight761Controller
                    (
                    hid_device*     handle,
                    const char*     path,
                    std::string     dev_name
                    );

    ~MSIMysticLight761Controller();

    void        SetMode
                    (
                    MSI_ZONE        zone,
                    MSI_MODE        mode,
                    MSI_SPEED       speed,
                    MSI_BRIGHTNESS  brightness,
                    bool            rainbow_color
                    );

    MSI_MODE    GetMode();

    void        GetMode
                    (
                    MSI_ZONE        zone,
                    MSI_MODE        &mode,
                    MSI_SPEED       &speed,
                    MSI_BRIGHTNESS  &brightness,
                    bool            &rainbow_color,
                    unsigned int    &color
                    );

    void        SetZoneColor
                    (
                    MSI_ZONE        zone,
                    unsigned char   red1,
                    unsigned char   grn1,
                    unsigned char   blu1,
                    unsigned char   red2,
                    unsigned char   grn2,
                    unsigned char   blu2
                    );

    void        SetLedColor
                    (
                    MSI_ZONE        zone,
                    std::size_t     index,
                    unsigned char   red,
                    unsigned char   grn,
                    unsigned char   blu
                    );

    void        SetCycleCount
                    (
                    MSI_ZONE        zone,
                    unsigned char   cycle_num
                    );

    bool        Update
                    (
                    bool save
                    );

    std::string     GetDeviceName();
    std::string     GetDeviceLocation();
    std::string     GetFWVersion();
    std::string     GetSerial();

    void        SetDirectMode
                    (
                    bool mode
                    );

    bool        IsDirectModeActive();

    size_t      GetMaxDirectLeds
                    (
                    MSI_ZONE zone
                    );

    const std::vector<MSI_ZONE>*
    GetSupportedZones()
    {
        return supported_zones;
    }

    enum DIRECT_MODE
    {
        DIRECT_MODE_DISABLED,
        DIRECT_MODE_PER_LED,
        DIRECT_MODE_ZONE_BASED
    };

    DIRECT_MODE     GetSupportedDirectMode()
    {
        return static_zone_protocol ? DIRECT_MODE_ZONE_BASED : DIRECT_MODE_PER_LED;
    }

    struct ZoneConfig
    {
        MSI_ZONE msi_zone;
        ZoneData * zone_data;
    };

private:
    std::string                                             name;
    std::vector<MSIMysticLight761Controller::ZoneConfig>    zone_configs;
    const std::vector<MSI_ZONE>*                            supported_zones;
    hid_device*                                             dev;
    std::string                                             location;
    std::string                                             version_aprom;
    std::string                                             version_ldrom;
    FeaturePacket_761*                                       data;
    bool                                                     static_zone_protocol = false;
    FeaturePacket_StaticZone_761                             static_data;

    bool            ReadSettings();
    bool            ReadFwVersion();
    void            ReadName();
    int             GetStaticZoneIndex(MSI_ZONE zone);

    ZoneData* GetZoneData
            (
            FeaturePacket_761&  data_packet,
            MSI_ZONE            zone
            );
    RainbowZoneData* GetRainbowZoneData(MSI_ZONE zone);
    Color* GetPerLedZoneData
            (
            MSI_ZONE zone
            );
    void            SelectPerLedProtocol();
};

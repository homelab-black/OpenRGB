/*---------------------------------------------------------*\
| MSIMotherboard761Controller.h                             |
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
#include "MSIMotherboardCommon.h"
#include "RGBController.h"

inline constexpr const char * BOARD_UNSUPPORTED_ERROR =  "No Config Found For Board";

class MSIMotherboard761Controller
{
public:
    MSIMotherboard761Controller
                    (
                    hid_device*     handle,
                    const char*     path,
                    std::string     dev_name
                    );

    ~MSIMotherboard761Controller();

    void        SetMode
                    (
                    MSI_ZONE        zone,
                    int             mode,
                    MSI_SPEED       speed,
                    MSI_BRIGHTNESS  brightness,
                    bool            rainbow_color,
                    bool            direction_left
                    );

    void        SetZonePalette
                    (
                    MSI_ZONE        zone,
                    const Color     palette[],
                    unsigned int    num_colors
                    );

    int         GetMode();

    void        GetMode
                    (
                    MSI_ZONE        zone,
                    int             &mode,
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

    void        SetZoneLEDCount
                    (
                    MSI_ZONE        zone,
                    unsigned char   led_num
                    );

    void        SendConfiguration();

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
        return DIRECT_MODE_PER_LED;
    }

    /*-----------------------------------------------------*\
    | True when this board accepts effect presets in the    |
    | zone rows of the 0x50 report (see MSI_50_EFFECT)      |
    \*-----------------------------------------------------*/
    bool            SupportsHardwareModes()
    {
        return hw_modes;
    }

    /*-----------------------------------------------------*\
    | LED count MSI Center configures these headers with    |
    | (effects only reach a row with a non-zero count)      |
    \*-----------------------------------------------------*/
    unsigned char   GetDefaultLedNum()
    {
        return (unsigned char)default_lednum;
    }

    /*-----------------------------------------------------*\
    | LED count the controller itself reported for a zone   |
    | row when the device was detected, falling back to the |
    | board default.  Used as the initial zone size so the  |
    | cycle length the board is already using is preserved. |
    \*-----------------------------------------------------*/
    unsigned char   GetZoneLedNum
                        (
                        MSI_ZONE zone
                        );

    /*-----------------------------------------------------*\
    | Effect state of one row of the 0x50 report.  Each     |
    | zone's effect is written into that zone's own row     |
    | (verified against MSI Center's traffic on MS-7E47),   |
    | so effects can differ per header.                     |
    \*-----------------------------------------------------*/
    struct Effect50
    {
        int             mode            = MSI_MODE_DIRECT_DUMMY;
        unsigned char   effect          = MSI_50_EFFECT_PER_LED;
        unsigned char   sub             = MSI_50_SUB_PLAIN;
        MSI_SPEED       speed           = MSI_SPEED_MEDIUM;
        MSI_BRIGHTNESS  brightness      = MSI_BRIGHTNESS_LEVEL_100;
        bool            rainbow_color   = false;
        bool            use_palette     = false;
        bool            direction_left  = false;
        Color           palette[MSI_50_NUM_PALETTE_COLORS] =
                        { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
        bool            valid           = false;
    };

    struct ZoneConfig
    {
        MSI_ZONE msi_zone;
        ZoneData * zone_data;
        Effect50 effect;
        unsigned char adopted_lednum = 0;   // LED count the controller reported for this row
    };

private:
    std::string                                             name;
    std::vector<MSIMotherboard761Controller::ZoneConfig>    zone_configs;
    const std::vector<MSI_ZONE>*                            supported_zones;
    hid_device*                                             dev;
    std::string                                             location;
    std::string                                             version_aprom;
    std::string                                             version_ldrom;
    FeaturePacket_761* data;
    unsigned char*                                          setup_data;
    bool                                                    setup_dirty;
    int                                                     numof_pipe1_leds;
    bool                                                    hw_modes;
    bool                                                    direct_mode_active;
    int                                                     default_lednum;

    void            WriteEffectRows();
    bool            ReadEffectRows
                        (
                        unsigned char* arr
                        );
    bool            UpdateEffects
                        (
                        bool save
                        );
    ZoneConfig*     FindZoneConfig
                        (
                        MSI_ZONE zone
                        );
    static unsigned char BrightnessTo50
                        (
                        MSI_BRIGHTNESS brightness
                        );

    bool            ReadSettings();
    bool            ReadFwVersion();
    void            ReadName();

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

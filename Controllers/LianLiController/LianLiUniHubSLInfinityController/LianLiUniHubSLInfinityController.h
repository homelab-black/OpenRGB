/*---------------------------------------------------------*\
| LianLiUniHubSLInfinityController.h                        |
|                                                           |
|   Driver for Lian Li SL Infinity Uni Hub                  |
|                                                           |
|   Pau Verdeguer                               13 May 2026 |
|   Simon McKenna                               21 Oct 2023 |
|   Will Kennedy                                17 Jan 2023 |
|   Oliver P                                    26 Apr 2022 |
|   Credit to Luca Lovisa for original work.                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------*\
| The hub has 4 ports with up to 4 fans each. Each port has |
| two channels, the inner ring (8 LEDs per fan) on channel  |
| port * 2 and the outer ring (12 LEDs per fan) on channel  |
| port * 2 + 1.                                             |
\*---------------------------------------------------------*/
enum
{
    UNIHUB_SLINF_GROUP_COUNT                = 0x04,
    UNIHUB_SLINF_CHANNEL_COUNT              = 0x08,
    UNIHUB_SLINF_MAX_FAN_COUNT              = 0x04,
    UNIHUB_SLINF_INNER_LEDS_PER_FAN         = 0x08,
    UNIHUB_SLINF_OUTER_LEDS_PER_FAN         = 0x0C,
    UNIHUB_SLINF_MAX_CHAN_LED_COUNT         = UNIHUB_SLINF_OUTER_LEDS_PER_FAN * UNIHUB_SLINF_MAX_FAN_COUNT,
    UNIHUB_SLINF_EFFECT_COLOR_SLOTS         = 0x10,
};

/*---------------------------------------------------------*\
| Effect IDs, taken from L-Connect 3. SIM effects run on    |
| both rings of a port at once, MERGED effects run across   |
| all ports.                                                |
\*---------------------------------------------------------*/
enum
{
    UNIHUB_SLINF_LED_MODE_STATIC_COLOR             = 0x01,
    UNIHUB_SLINF_LED_MODE_BREATHING                = 0x02,
    UNIHUB_SLINF_LED_MODE_RAINBOW_MORPH            = 0x04,
    UNIHUB_SLINF_LED_MODE_RAINBOW                  = 0x05,
    UNIHUB_SLINF_LED_MODE_BREATHING_RAINBOW        = 0x06,
    UNIHUB_SLINF_LED_MODE_METEOR_RAINBOW           = 0x08,
    UNIHUB_SLINF_LED_MODE_COLOR_CYCLE              = 0x18,
    UNIHUB_SLINF_LED_MODE_METEOR                   = 0x19,
    UNIHUB_SLINF_LED_MODE_RUNWAY                   = 0x1A,
    UNIHUB_SLINF_LED_MODE_MOP_UP                   = 0x1B,
    UNIHUB_SLINF_LED_MODE_TAICHI                   = 0x1C,
    UNIHUB_SLINF_LED_MODE_DOUBLE_METEOR            = 0x1D,
    UNIHUB_SLINF_LED_MODE_METEOR_CONTEST           = 0x1E,
    UNIHUB_SLINF_LED_MODE_METEOR_MIX               = 0x1F,
    UNIHUB_SLINF_LED_MODE_RETURN_ARC               = 0x20,
    UNIHUB_SLINF_LED_MODE_DOUBLE_ARC               = 0x21,
    UNIHUB_SLINF_LED_MODE_DOOR                     = 0x22,
    UNIHUB_SLINF_LED_MODE_DISCO                    = 0x23,
    UNIHUB_SLINF_LED_MODE_HEART_BEAT               = 0x24,
    UNIHUB_SLINF_LED_MODE_LOTTERY                  = 0x26,
    UNIHUB_SLINF_LED_MODE_COLORFUL_METEOR          = 0x27,
    UNIHUB_SLINF_LED_MODE_WARNING                  = 0x29,
    UNIHUB_SLINF_LED_MODE_VOICE                    = 0x2A,
    UNIHUB_SLINF_LED_MODE_MIXING                   = 0x2B,
    UNIHUB_SLINF_LED_MODE_STACK                    = 0x2C,
    UNIHUB_SLINF_LED_MODE_TIDE                     = 0x2D,
    UNIHUB_SLINF_LED_MODE_SCAN                     = 0x2E,
    UNIHUB_SLINF_LED_MODE_REFLECT                  = 0x30,
    UNIHUB_SLINF_LED_MODE_RAINBOW_SIM              = 0x32,
    UNIHUB_SLINF_LED_MODE_WARNING_SIM              = 0x36,
    UNIHUB_SLINF_LED_MODE_VOICE_SIM                = 0x37,
    UNIHUB_SLINF_LED_MODE_MIXING_SIM               = 0x38,
    UNIHUB_SLINF_LED_MODE_STACK_SIM                = 0x39,
    UNIHUB_SLINF_LED_MODE_TIDE_SIM                 = 0x3A,
    UNIHUB_SLINF_LED_MODE_SCAN_SIM                 = 0x3B,
    UNIHUB_SLINF_LED_MODE_DOOR_SIM                 = 0x3C,
    UNIHUB_SLINF_LED_MODE_RAINBOW_MORPH_SIM        = 0x3F,
    UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_SIM     = 0x41,
    UNIHUB_SLINF_LED_MODE_HEART_BEAT_SIM           = 0x42,
    UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_SIM    = 0x43,
    UNIHUB_SLINF_LED_MODE_MOP_UP_SIM               = 0x44,
    UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY        = 0x45,
    UNIHUB_SLINF_LED_MODE_RUNWAY_MERGED            = 0x46,
    UNIHUB_SLINF_LED_MODE_MOP_UP_MERGED            = 0x47,
    UNIHUB_SLINF_LED_MODE_MIXING_MERGED            = 0x48,
    UNIHUB_SLINF_LED_MODE_STACK_MERGED             = 0x49,
    UNIHUB_SLINF_LED_MODE_TIDE_MERGED              = 0x4A,
    UNIHUB_SLINF_LED_MODE_SCAN_MERGED              = 0x4B,
    UNIHUB_SLINF_LED_MODE_DOOR_MERGED              = 0x4C,
    UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_MERGED = 0x4D,
    UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_MERGED  = 0x4E,
};

enum
{
    UNIHUB_SLINF_LED_SPEED_000              = 0x02,
    UNIHUB_SLINF_LED_SPEED_025              = 0x01,
    UNIHUB_SLINF_LED_SPEED_050              = 0x00,
    UNIHUB_SLINF_LED_SPEED_075              = 0xFF,
    UNIHUB_SLINF_LED_SPEED_100              = 0xFE,
};

enum
{
    UNIHUB_SLINF_LED_DIRECTION_RIGHT        = 0x00,
    UNIHUB_SLINF_LED_DIRECTION_LEFT         = 0x01,
};

/*---------------------------------------------------------*\
| L-Connect also defines 0x04 as the lowest brightness, but |
| it never sends it. Its 0% position is sent as 0x08 (off). |
\*---------------------------------------------------------*/
enum
{
    UNIHUB_SLINF_LED_BRIGHTNESS_000         = 0x08,
    UNIHUB_SLINF_LED_BRIGHTNESS_025         = 0x03,
    UNIHUB_SLINF_LED_BRIGHTNESS_050         = 0x02,
    UNIHUB_SLINF_LED_BRIGHTNESS_075         = 0x01,
    UNIHUB_SLINF_LED_BRIGHTNESS_100         = 0x00,
};

/*---------------------------------------------------------*\
| Colors are sent as a 352 byte output report and           |
| everything else as a 6 byte feature report, both with     |
| report ID 0xE0. The hub has no OUT endpoint, so every     |
| report is a control transfer that only returns once the   |
| hub has handled it.                                       |
\*---------------------------------------------------------*/
enum
{
    UNIHUB_SLINF_REPORT_ID                  = 0xE0,
    UNIHUB_SLINF_OUTPUT_REPORT_SIZE         = 352 + 1,
    UNIHUB_SLINF_FEATURE_REPORT_SIZE        = 6 + 1,
    UNIHUB_SLINF_INPUT_REPORT_SIZE          = 64 + 1,
};

enum
{
    UNIHUB_SLINF_CMD_EFFECT                 = 0x10,
    UNIHUB_SLINF_CMD_COLOR                  = 0x30,
    UNIHUB_SLINF_CMD_QUERY                  = 0x50,
    UNIHUB_SLINF_CMD_SET_FRAME              = 0x60,
};

/*---------------------------------------------------------*\
| Sub-commands, sent on the channel 0 effect address.       |
\*---------------------------------------------------------*/
enum
{
    UNIHUB_SLINF_SUBCMD_FAN_QUANTITY        = 0x60,
    UNIHUB_SLINF_SUBCMD_MB_ARGB_SYNC        = 0x61,
    UNIHUB_SLINF_SUBCMD_MERGE_ORDER         = 0x63,
};

class LianLiUniHubSLInfinityController
{
public:
    LianLiUniHubSLInfinityController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~LianLiUniHubSLInfinityController();

    std::string         GetDeviceLocation();
    std::string         GetFirmwareVersionString();
    std::string         GetName();
    std::string         GetSerialString();

    void                SetFanQuantity
                                (
                                unsigned char       group,
                                unsigned char       quantity
                                );

    /*-----------------------------------------------------*\
    | The SetChannel functions return false if the channel  |
    | already shows this state, in which case nothing is    |
    | sent.                                                 |
    \*-----------------------------------------------------*/
    bool                SetChannelLEDs
                                (
                                unsigned char       channel,
                                RGBColor*           colors,
                                unsigned int        num_colors
                                );

    bool                SetChannelBreathing
                                (
                                unsigned char       channel,
                                RGBColor*           colors,
                                unsigned int        num_colors,
                                const mode&         active_mode
                                );

    bool                SetChannelMode
                                (
                                unsigned char       channel,
                                const mode&         active_mode
                                );

    bool                SetChannelModeSimultaneous
                                (
                                unsigned char       inner_channel,
                                const mode&         active_mode
                                );

    void                SetChannelOff(unsigned char channel);

    void                SetMergedMode(const mode& active_mode);

    /*-----------------------------------------------------*\
    | Every channel has to be sent again after this.        |
    \*-----------------------------------------------------*/
    void                LeaveMergedMode();

    void                SendGlobalTrigger();

    void                MarkAllChannelsStale();
    bool                IsMerged();

private:
    void                SendChannel
                                (
                                unsigned char       channel,
                                unsigned int        num_leds,
                                unsigned char*      led_data,
                                unsigned char       effect,
                                unsigned char       speed,
                                unsigned int        direction,
                                unsigned int        brightness
                                );

    void                SendFeatureReport(unsigned char* data);

    bool                IsCached
                                (
                                unsigned char       channel,
                                unsigned int        num_leds,
                                unsigned char*      led_data,
                                unsigned char       effect,
                                unsigned char       speed,
                                unsigned int        direction,
                                unsigned int        brightness
                                );

    bool                StageZone
                                (
                                unsigned char       channel,
                                unsigned int        num_leds,
                                unsigned char*      led_data,
                                unsigned char       effect,
                                unsigned char       speed,
                                unsigned int        direction,
                                unsigned int        brightness
                                );

    unsigned int        BuildPerLEDData
                                (
                                unsigned char*      led_data,
                                RGBColor*           colors,
                                unsigned int        num_colors
                                );

    hid_device*         dev;
    std::string         location;
    std::string         name;

    /*-----------------------------------------------------*\
    | Last state sent to each channel, used to skip         |
    | channels that already show it. zone_effect_cache      |
    | holds effect, speed, direction and brightness.        |
    \*-----------------------------------------------------*/
    unsigned char       zone_color_cache [UNIHUB_SLINF_CHANNEL_COUNT][UNIHUB_SLINF_MAX_CHAN_LED_COUNT * 3];
    unsigned int        zone_color_count [UNIHUB_SLINF_CHANNEL_COUNT];
    unsigned char       zone_effect_cache[UNIHUB_SLINF_CHANNEL_COUNT][4];

    /*-----------------------------------------------------*\
    | Set when a channel may not show its cached state, for |
    | example an outer ring following a Simultaneous        |
    | effect, a channel taken over by a Merged effect or a  |
    | failed transfer.                                      |
    \*-----------------------------------------------------*/
    bool                zone_stale       [UNIHUB_SLINF_CHANNEL_COUNT];

    std::chrono::steady_clock::time_point last_effect_time[UNIHUB_SLINF_CHANNEL_COUNT];
    bool                last_effect_static[UNIHUB_SLINF_CHANNEL_COUNT];
    std::chrono::steady_clock::time_point last_report_time;
    std::chrono::steady_clock::time_point merge_time;

    bool                is_merged;
};

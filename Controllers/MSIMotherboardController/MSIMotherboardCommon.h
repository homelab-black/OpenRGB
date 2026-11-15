/*---------------------------------------------------------*\
| MSIMotherboardCommon.h                                    |
|                                                           |
|   Common definitions for MSI Mystic Light motherboards    |
|                                                           |
|   Adam Honse                                  06 Mar 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>

enum MSI_ZONE
{
    MSI_ZONE_NONE                                   = 0,
    MSI_ZONE_J_RGB_1                                = 1,
    MSI_ZONE_J_RGB_2                                = 2,
    MSI_ZONE_J_PIPE_1                               = 3,
    MSI_ZONE_J_PIPE_2                               = 4,
    MSI_ZONE_J_PIPE_3                               = 5,
    MSI_ZONE_J_PIPE_4                               = 6,
    MSI_ZONE_J_PIPE_5                               = 7,
    MSI_ZONE_J_RAINBOW_1                            = 8,
    MSI_ZONE_J_RAINBOW_2                            = 9,
    MSI_ZONE_J_RAINBOW_3                            = 10,
    MSI_ZONE_J_CORSAIR                              = 11,
    MSI_ZONE_J_CORSAIR_OUTERLL120                   = 12,
    MSI_ZONE_ON_BOARD_LED_0                         = 13,
    MSI_ZONE_ON_BOARD_LED_1                         = 14,
    MSI_ZONE_ON_BOARD_LED_2                         = 15,
    MSI_ZONE_ON_BOARD_LED_3                         = 16,
    MSI_ZONE_ON_BOARD_LED_4                         = 17,
    MSI_ZONE_ON_BOARD_LED_5                         = 18,
    MSI_ZONE_ON_BOARD_LED_6                         = 19,
    MSI_ZONE_ON_BOARD_LED_7                         = 20,
    MSI_ZONE_ON_BOARD_LED_8                         = 21,
    MSI_ZONE_ON_BOARD_LED_9                         = 22,
    MSI_ZONE_ON_BOARD_LED_10                        = 23,
    MSI_ZONE_JAF                                    = 24,
    MSI_ZONE_JARGB_1                                = 25,
    MSI_ZONE_JARGB_2                                = 26,
    MSI_ZONE_JARGB_3                                = 27,
};

enum MSI_MODE
{
    MSI_MODE_DISABLE                                = 0,
    MSI_MODE_STATIC                                 = 1,
    MSI_MODE_BREATHING                              = 2,
    MSI_MODE_FLASHING                               = 3,
    MSI_MODE_DOUBLE_FLASHING                        = 4,
    MSI_MODE_LIGHTNING                              = 5,
    MSI_MODE_MSI_MARQUEE                            = 6,
    MSI_MODE_METEOR                                 = 7,
    MSI_MODE_WATER_DROP                             = 8,
    MSI_MODE_MSI_RAINBOW                            = 9,
    MSI_MODE_POP                                    = 10,
    MSI_MODE_RAP                                    = 11,
    MSI_MODE_JAZZ                                   = 12,
    MSI_MODE_PLAY                                   = 13,
    MSI_MODE_MOVIE                                  = 14,
    MSI_MODE_COLOR_RING                             = 15,
    MSI_MODE_PLANETARY                              = 16,
    MSI_MODE_DOUBLE_METEOR                          = 17,
    MSI_MODE_ENERGY                                 = 18,
    MSI_MODE_BLINK                                  = 19,
    MSI_MODE_CLOCK                                  = 20,
    MSI_MODE_COLOR_PULSE                            = 21,
    MSI_MODE_COLOR_SHIFT                            = 22,
    MSI_MODE_COLOR_WAVE                             = 23,
    MSI_MODE_MARQUEE                                = 24,
    MSI_MODE_RAINBOW                                = 25,
    MSI_MODE_RAINBOW_WAVE                           = 26,
    MSI_MODE_VISOR                                  = 27,
    MSI_MODE_JRAINBOW                               = 28,
    MSI_MODE_RAINBOW_FLASHING                       = 29,
    MSI_MODE_RAINBOW_DOUBLE_FLASHING                = 30,
    MSI_MODE_RANDOM                                 = 31,
    MSI_MODE_FAN_CONTROL                            = 32,
    MSI_MODE_DISABLE_2                              = 33,
    MSI_MODE_COLOR_RING_FLASHING                    = 34,
    MSI_MODE_COLOR_RING_DOUBLE_FLASHING             = 35,
    MSI_MODE_STACK                                  = 36,
    MSI_MODE_CORSAIR_QUE                            = 37,
    MSI_MODE_FIRE                                   = 38,
    MSI_MODE_LAVA                                   = 39,
    MSI_MODE_DIRECT_DUMMY                           = 100
};

enum MSI_SPEED
{
    MSI_SPEED_LOW                                   = 0,
    MSI_SPEED_MEDIUM                                = 1,
    MSI_SPEED_HIGH                                  = 2,
};

enum MSI_FAN_TYPE
{
    MSI_FAN_TYPE_SP                                 = 0,
    MSI_FAN_TYPE_HD                                 = 1,
    MSI_FAN_TYPE_LL                                 = 2,
};

enum MSI_BRIGHTNESS
{
    MSI_BRIGHTNESS_OFF                              = 0,
    MSI_BRIGHTNESS_LEVEL_10                         = 1,
    MSI_BRIGHTNESS_LEVEL_20                         = 2,
    MSI_BRIGHTNESS_LEVEL_30                         = 3,
    MSI_BRIGHTNESS_LEVEL_40                         = 4,
    MSI_BRIGHTNESS_LEVEL_50                         = 5,
    MSI_BRIGHTNESS_LEVEL_60                         = 6,
    MSI_BRIGHTNESS_LEVEL_70                         = 7,
    MSI_BRIGHTNESS_LEVEL_80                         = 8,
    MSI_BRIGHTNESS_LEVEL_90                         = 9,
    MSI_BRIGHTNESS_LEVEL_100                        = 10,
};

#define NUMOF_PER_LED_MODE_LEDS                     240
#define NUM_LEDS_761                                720

/*---------------------------------------------------------*\
| Definitions for the 290-byte 0x50 report used by the      |
| newer 0db0:0076 Mystic Light controllers (the boards      |
| handled by the 761-byte driver).  The report is a table   |
| of 16-byte rows: one row per zone plus a trailing command |
| row.  Row fields are addressed as (16 * row) + column;    |
| column 0 is unused except in row 0 where it carries the   |
| report ID.                                                |
|                                                           |
| The parameter byte layout was first published by the      |
| standalone script at                                      |
| https://github.com/Picachuchu69/msi-mystic-light-b860-x870-linux |
| (MIT), reverse engineered on an older B860 board.         |
| Everything below was re-derived from captures taken on an |
| MSI X870 GAMING PLUS WIFI (MS-7E47): on this generation   |
| an effect belongs to the zone's own row, where that script|
| drives it from the trailing row instead.                  |
|                                                           |
| On MS-7E47 the ONBOARD_* rows (11..16) stay zeroed in     |
| every MSI Center frame and are never written, which       |
| matches this PCB having no built-in decorative LED zones: |
| everything visible hangs off the JARGB_V2 header (row 2). |
\*---------------------------------------------------------*/
#define MSI_50_ROW_COL_EFFECT                       1   // effect ID, see MSI_50_EFFECT
#define MSI_50_ROW_COL_COLOR1                       2   // up to 4 RGB colors: columns 2 .. 13
#define MSI_50_ROW_COL_SUB                          14  // see MSI_50_SUB_*
#define MSI_50_ROW_COL_PARAM                        15  // see MSI_50_PARAM_*
#define MSI_50_ROW_COL_LEDNUM                       16  // LED count of the row's zone
#define MSI_50_IDX_APPLY                            289 // 01 = commit to controller flash, 00 = volatile only.
                                                    // A committed frame is what the board shows
                                                    // after power is lost: MSI Center's own commit
                                                    // was replayed at POST by an MS-7E47 following a
                                                    // cold power cycle (PSU off over a minute), with
                                                    // no software running.  A warm reboot does not
                                                    // test this: 5V standby keeps the RAM frame.

/*---------------------------------------------------------*\
| The effect of a zone is written into that zone's own row, |
| verified by capturing MSI Center's traffic on an MSI      |
| X870 GAMING PLUS WIFI (MS-7E47): applying an effect to    |
| JARGB_V2_3 only rewrites row 2 and leaves the other rows  |
| at their defaults.  Row 17 (the last row) is a trailing   |
| row that MSI Center keeps at effect 0x01 / param 0x14;     |
| the standalone reverse engineering of the older B860      |
| boards drove effects from that row instead.                |
\*---------------------------------------------------------*/
#define MSI_50_ROW_COMMAND                          17  // trailing row (see note above)

/*---------------------------------------------------------*\
| Effect IDs of the 0x50 report.  These are NOT the same IDs|
| as the 112/162/185-byte protocols' MSI_MODE values, so    |
| the driver translates by name and never by number:        |
| breathing is 0x04 here but 2 there, steady/static 0x02    |
| here but 1 there, lightning 0x06 here but 5 there.        |
|                                                           |
| Every ID below was identified by capturing MSI Center on  |
| an MS-7E47 while applying its effects one by one, in a    |
| logged order, and replaying the frames back:              |
|   0x00 Off      0x01 Wave     0x02 Steady   0x03 Flame    |
|   0x04 Breath   0x05 Color ring  0x06 Lightning           |
|   0x07 Recreation - not offered: it produced no light from |
|      MSI Center's own captured payload nor from any of the  |
|      variants tried here (sub 0x00/0x01/0x03, palette or    |
|      generated colors, cycle 30 and 60), while a            |
|      0x04 Breath control on the same row was visible.       |
|      Why it is inert is not established; MSI Center presents|
|      it next to its software features (its Ambient Link    |
|      discovery found no devices on the test system), but   |
|      the only measured facts are the ones above.           |
|   0x08 Meteor                                             |
| MSI Center's "Default" is 0x01 with the palette cleared,  |
| i.e. the same ID as Wave.  "CPU temperature" is not an    |
| ID at all: MSI Center writes 0x02 (Steady) with a color   |
| derived from the CPU temperature in software.             |
\*---------------------------------------------------------*/
enum MSI_50_EFFECT
{
    MSI_50_EFFECT_DISABLE                           = 0x00,
    MSI_50_EFFECT_COLOR_WAVE                        = 0x01, // MSI Center "Wave"/"Default": sub 0x03, direction applies
    MSI_50_EFFECT_STATIC                            = 0x02, // MSI Center "Steady"
    MSI_50_EFFECT_FIRE                              = 0x03, // MSI Center "Flame": 4 palette colors, sub 0x01
    MSI_50_EFFECT_BREATHING                         = 0x04, // MSI Center "Breath": palette color, sub 0x00
    MSI_50_EFFECT_COLOR_RING                        = 0x05, // MSI Center "Color ring": sub 0x03, direction applies; generated unless the palette bit is set
    MSI_50_EFFECT_LIGHTNING                         = 0x06, // MSI Center "Lightning": palette colors, sub 0x00
    MSI_50_EFFECT_RECREATION                        = 0x07, // MSI Center "Recreation": inert on MS-7E47, see note above
    MSI_50_EFFECT_METEOR                            = 0x08, // MSI Center "Meteor": sub 0x03, palette with a 4-color preset
    MSI_50_EFFECT_PER_LED                           = 0x09, // colors taken from the per-LED packets
};

/*---------------------------------------------------------*\
| Row sub-parameter (column 14).  MSI Center writes 0x00    |
| with the single-colour effects, 0x03 with the generated-  |
| color effects and 0x01 with the 4-colour preset.          |
\*---------------------------------------------------------*/
#define MSI_50_SUB_PLAIN                            0x00
#define MSI_50_SUB_FOUR_COLOR                       0x01
#define MSI_50_SUB_GENERATED                        0x03

/*---------------------------------------------------------*\
| Row parameter byte (column 15), bit by bit                |
\*---------------------------------------------------------*/
#define MSI_50_PARAM_SPEED_SHIFT                    0
#define MSI_50_PARAM_SPEED_MASK                     (0x03 << MSI_50_PARAM_SPEED_SHIFT)
#define MSI_50_PARAM_BRIGHTNESS_SHIFT               2
#define MSI_50_PARAM_BRIGHTNESS_MASK                (0x07 << MSI_50_PARAM_BRIGHTNESS_SHIFT)
#define MSI_50_PARAM_USE_PALETTE                    (1 << 5) // 1 = palette colors, 0 = generated rainbow
#define MSI_50_PARAM_DIRECTION_LEFT                 (1 << 6) // effects with motion only
#define MSI_50_PARAM_ANIMATED                       (1 << 7) // standing patterns (B860 command row only)

/*---------------------------------------------------------*\
| MSI Center never sets bit 7 in a zone row: with bit 7 set |
| the zone row is ignored (measured on MS-7E47).  Bit 7 is  |
| only used by the command row on the older B860 boards.    |
|                                                           |
| A row with bit 7 set is also what the controller presents |
| after power has been lost: on an MS-7E47 that had just    |
| lit a saved profile at POST, the row read back from 0x50  |
| was effect 0x00 / param 0x94, not the profile it was      |
| displaying.  So the readable frame tracks the last        |
| settings write, not what the LEDs are doing - which is    |
| consistent with MSI Center never reading 0x50 back (neither|
| capture contains a GET of it).                            |
\*---------------------------------------------------------*/
#define MSI_50_PARAM_ZONE_MASK                      0x7F

/*---------------------------------------------------------*\
| Brightness in this report is 1..5 (5 = maximum) while     |
| OpenRGB's MSI_BRIGHTNESS runs from 0..10                  |
\*---------------------------------------------------------*/
#define MSI_50_BRIGHTNESS_OFF                       0
#define MSI_50_BRIGHTNESS_MIN                       1
#define MSI_50_BRIGHTNESS_MAX                       5

#define MSI_50_NUM_PALETTE_COLORS                   4

/*---------------------------------------------------------*\
| Flag bitfield bit definitions                             |
| Note that the sync flags split across the two fields      |
\*---------------------------------------------------------*/
#define MSI_SPEED_BRIGHTNESS_FLAG_SPEED_SHIFT       0
#define MSI_SPEED_BRIGHTNESS_FLAG_SPEED_MASK        (0x03 << MSI_SPEED_BRIGHTNESS_FLAG_SPEED_SHIFT)
#define MSI_SPEED_BRIGHTNESS_FLAG_BRIGHTNESS_SHIFT  2
#define MSI_SPEED_BRIGHTNESS_FLAG_BRIGHTNESS_MASK   (0x1F << MSI_SPEED_BRIGHTNESS_FLAG_BRIGHTNESS_SHIFT)
#define MSI_SPEED_BRIGHTNESS_FLAG_SYNC_SETTING_JRGB (1 << 7)
#define MSI_COLOR_FLAG_SYNC_SETTING_ONBOARD         (1 << 0)
#define MSI_COLOR_FLAG_SYNC_SETTING_JRAINBOW1       (1 << 1)
#define MSI_COLOR_FLAG_SYNC_SETTING_JRAINBOW2       (1 << 2)
#define MSI_COLOR_FLAG_SYNC_SETTING_JCORSAIR        (1 << 3)
#define MSI_COLOR_FLAG_SYNC_SETTING_JPIPE1          (1 << 4)
#define MSI_COLOR_FLAG_SYNC_SETTING_JPIPE2          (1 << 5)
#define MSI_COLOR_FLAG_USE_CUSTOM_COLOR             (1 << 7)

struct Color
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
};

struct CorsairZoneData
{
    unsigned char effect                            = MSI_MODE_STATIC;
    Color         color                             { 0, 0, 0 };
    unsigned char fan_flags                         = 40;
    unsigned char corsair_quantity                  = 0;
    unsigned char padding[4]                        = { 0, 0, 0, 0 };
    unsigned char is_individual                     = 0;
};

struct ZoneData
{
    unsigned char effect                            = MSI_MODE_STATIC;
    Color         color                             { 0, 0, 0 };
    unsigned char speedAndBrightnessFlags           = 0;
    Color         color2                            { 0, 0, 0 };
    unsigned char colorFlags                        = 0;
    unsigned char padding                           = 0;
};

struct RainbowZoneData : ZoneData
{
    unsigned char cycle_or_led_num                  = 100;
};

struct FeaturePacket_112
{
    const unsigned char report_id                   = 0x52; // Report ID
    ZoneData            j_rgb_1;                            // 1
    ZoneData            j_rainbow_1;                        // 11
    ZoneData            j_corsair_1;                        // 21
    ZoneData            j_corsair_outerll120;               // 31
    ZoneData            on_board_led;                       // 41
    ZoneData            on_board_led_1;                     // 51
    ZoneData            on_board_led_2;                     // 61
    ZoneData            on_board_led_3;                     // 71
    ZoneData            on_board_led_4;                     // 81
    ZoneData            on_board_led_5;                     // 91
    ZoneData            on_board_led_6;                     // 101
    unsigned char       save_data                   = 0;    // 111
};

struct FeaturePacket_162
{
    const unsigned char report_id                   = 0x52; // Report ID
    ZoneData            j_rgb_1;                            // 1
    ZoneData            j_rainbow_1;                        // 11
    ZoneData            j_corsair_1;                        // 21
    ZoneData            j_corsair_outerll120;               // 31
    ZoneData            on_board_led;                       // 41
    ZoneData            on_board_led_1;                     // 51
    ZoneData            on_board_led_2;                     // 61
    ZoneData            on_board_led_3;                     // 71
    ZoneData            on_board_led_4;                     // 81
    ZoneData            on_board_led_5;                     // 91
    ZoneData            on_board_led_6;                     // 101
    ZoneData            on_board_led_7;                     // 111
    ZoneData            on_board_led_8;                     // 121
    ZoneData            on_board_led_9;                     // 131
    ZoneData            on_board_led_10;                    // 141
    ZoneData            j_rgb_2;                            // 151
    unsigned char       save_data                   = 0;    // 161
};

struct FeaturePacket_185
{
    const unsigned char report_id                   = 0x52; // Report ID
    ZoneData            j_rgb_1;                            // 1
    ZoneData            j_pipe_1;                           // 11
    ZoneData            j_pipe_2;                           // 21
    RainbowZoneData     j_rainbow_1;                        // 31
    RainbowZoneData     j_rainbow_2;                        // 42
    CorsairZoneData     j_corsair;                          // 53
    ZoneData            j_corsair_outerll120;               // 64
    ZoneData            on_board_led;                       // 74
    ZoneData            on_board_led_1;                     // 84
    ZoneData            on_board_led_2;                     // 94
    ZoneData            on_board_led_3;                     // 104
    ZoneData            on_board_led_4;                     // 114
    ZoneData            on_board_led_5;                     // 124
    ZoneData            on_board_led_6;                     // 134
    ZoneData            on_board_led_7;                     // 144
    ZoneData            on_board_led_8;                     // 154
    ZoneData            on_board_led_9;                     // 164
    ZoneData            j_rgb_2;                            // 174
    unsigned char       save_data                   = 0;    // 184
};

struct FeaturePacket_PerLED_185
{
    unsigned char report_id                         = 0x53; // Report ID
    unsigned char hdr0                              = 0x25; // header byte 0
    unsigned char hdr1                              = 0x06; // header byte 1
    unsigned char hdr2                              = 0x00; // header byte 2
    unsigned char hdr3                              = 0x00; // header byte 3
    Color         leds[NUMOF_PER_LED_MODE_LEDS];
};

struct FeaturePacket_PerLED_761
{
    unsigned char report_id                         = 0x51; // Report ID
    unsigned char fixed1                            = 0x09; // Always 9?
    unsigned char hdr0;                                     // 0x08 for ez 0x04 for ARGB 0x06 for LED
    unsigned char hdr1;                                     // 0 for LED/EZ and argb num for ARGB?
    unsigned char fixed2                            = 0x00; // IDK what this is
    unsigned char fixed3                            = 0x00; // IDK what this is
    unsigned char hdr2;                                     // Led Count
    unsigned char colors [NUM_LEDS_761];
};

struct FeaturePacket_Zone_761
{
    MSI_ZONE zone;
    FeaturePacket_PerLED_761 packet;
    bool dirty;
};

struct FeaturePacket_761
{
    FeaturePacket_Zone_761 jargb1;
    FeaturePacket_Zone_761 jargb2;
    FeaturePacket_Zone_761 jargb3;
    FeaturePacket_Zone_761 jaf;
    FeaturePacket_Zone_761 jpipe1;
};

#define MSI_USB_PID_COMMON                          0x0076  // Common PID for a certain set of 185-byte boards

#ifndef STEELSERIESAPSDEVICES_H
#define STEELSERIESAPSDEVICES_H

#include "SteelSeriesApSKLCLayouts.h"

/*-----------------------------------------------------*\
| LightBars ALC                                         |
\*-----------------------------------------------------*/
#define STEELSERIES_APS_ALC_1124_PID 0x1124
#define STEELSERIES_APS_ALC_1134_PID 0x1134
#define STEELSERIES_APS_ALC_113C_PID 0x113C
#define STEELSERIES_APS_ALC_113E_PID 0x113E
#define STEELSERIES_APS_ALC_1153_PID 0x1153
#define STEELSERIES_APS_ALC_114B_PID 0x114B
#define STEELSERIES_APS_ALC_114D_PID 0x114D

/*-----------------------------------------------------*\
| ALC different sizes                                   |
\*-----------------------------------------------------*/
const std::map<uint16_t, uint8_t> steelseries_aps_alc_devices =
{
    { STEELSERIES_APS_ALC_1124_PID, 24  },
    { STEELSERIES_APS_ALC_1134_PID, 24  },
    { STEELSERIES_APS_ALC_113C_PID, 35  },
    { STEELSERIES_APS_ALC_113E_PID, 255 }, // untested
    { STEELSERIES_APS_ALC_1153_PID, 55  },
    { STEELSERIES_APS_ALC_114B_PID, 255 },// untested
    { STEELSERIES_APS_ALC_114D_PID, 56 },
};

#define STEELSERIES_APS_ALC_UNKNOWN_SIZE 255

/*-----------------------------------------------------*\
| Keyboards (KLC                                        |
\*-----------------------------------------------------*/
#define STEELSERIES_APS_KLC_1122_PID 0x1122
#define STEELSERIES_APS_KLC_113A_PID 0x113A

// TODO remove
//#define STEELSERIES_APS_KLC_DUMMY_VID 0x046D
//#define STEELSERIES_APS_KLC_DUMMY_PID 0xC084
// -- TODO

typedef std::tuple<keyboard_keymap_overlay_values, KEYBOARD_LAYOUT> steelseries_aps_klc_device;
typedef std::map<uint8_t, steelseries_aps_klc_device> steelseries_aps_klc_device_layouts_map;
typedef std::map<uint16_t, steelseries_aps_klc_device_layouts_map> steelseries_aps_klc_devices_pid_map;
/*-----------------------------------------------------*\
| 1122_PID                                              |
|    AZERTY         GE73 SKU ?      1E 02 00 00         |
|    Turkish        A13VHG-495TR    37 02 00 00         |
|    NL             14VIG-639NL     33 00 00 00         |
|    US ansi        10UH-231US      27 00 00 00         |
|    Norvegian      7RF-216NE       21 00 00 00         |
| 113A_PID                                              |
|    US ansi        13VH-438US      0E 00 00 00         |
\*-----------------------------------------------------*/
// Uncomment to provide a buid that will spawn a LINEAR ZONE, easier to find layouts ids.
//#define STEELESRIES_APS_IGNORE_KLC

const steelseries_aps_klc_devices_pid_map steelseries_aps_klc_devices =
#ifdef STEELESRIES_APS_IGNORE_KLC
{};
#else
{
    {
        STEELSERIES_APS_KLC_1122_PID,
        {
            { 0x1E, {steelseries_aps_klc_1122_default_layout, KEYBOARD_LAYOUT_ANSI_QWERTY}}, // TODO
            { 0x37, {steelseries_aps_klc_1122_default_layout, KEYBOARD_LAYOUT_ANSI_QWERTY}}, // TODO
            { 0x33, {steelseries_aps_klc_1122_default_layout, KEYBOARD_LAYOUT_ANSI_QWERTY}}, // TODO
            { 0x27, {steelseries_aps_klc_1122_default_layout, KEYBOARD_LAYOUT_ANSI_QWERTY}}, // TODO
            { 0x21, {steelseries_aps_klc_1122_0x21_layout,    KEYBOARD_LAYOUT_ANSI_QWERTY}}, // WIP
        }
    },
    {
        STEELSERIES_APS_KLC_113A_PID,
        {
            // empty data, will automatically fallback on a linear zone
        }
    }
// TODO remove
#ifdef STEELSERIES_APS_KLC_DUMMY_PID
    ,{
        STEELSERIES_APS_KLC_DUMMY_PID,
        {
            { 0xFF, {steelseries_aps_klc_1122_0x21_layout, KEYBOARD_LAYOUT_ANSI_QWERTY }}
        }
    }
#endif
// -- TODO
};
#endif



#endif // STEELSERIESAPSDEVICES_H

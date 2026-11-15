/*---------------------------------------------------------*\
| MSIMotherboard761Controller.cpp                           |
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

#include <vector>
#include "LogManager.h"
#include "MSIMotherboard761Controller.h"
#include "StringUtils.h"

#define NUM_CONFS sizeof(board_configs) / sizeof(mystic_light_761_config)
#define COLOR_BLACK {0, 0, 0}
#define IS_JARGB(X) (X == MSI_ZONE_JARGB_1 || X == MSI_ZONE_JARGB_2 || X == MSI_ZONE_JARGB_3)
#define GET_CHAR_PTR_REF(X) (unsigned char *) &(X)
#define ARRAY_ROW(X, Y) get_zone_setup_index(X)*16 + Y
#define ARRAY_ROW_RAW(X, Y) X*16 + Y
#define SETUP_ARRAY_SIZE 290

struct mystic_light_761_config
{
    const std::string *                      name;                  // Name of the board
    int                                      numof_onboard_leds;    // number of onboard leds
    int                                      numof_pipe1_leds;      // number of pipe 1 leds (used in per LED mode only)
    int                                      numof_pipe2_leds;      // number of pipe 2 leds (used in per LED mode only)
    int                                      numof_JRGBs;           // number of supported JRGB headers (used in per LED mode only)
    const std::vector<MSI_ZONE>*             supported_zones;       // pointer to vector of supported zones
    MSIMotherboard761Controller::DIRECT_MODE per_led_mode;          // type of direct mode support
    bool                                     supports_hw_modes = false; // board accepts effect presets in the 0x50 zone rows
    int                                      default_lednum    = 0; // LED count MSI Center uses for the headers
};

/*---------------------------------------------------------*\
| Modes of the 0x50 report's zone rows.  The IDs and the    |
| per-mode sub values below come from capturing MSI        |
| Center's USB traffic on an MSI X870 GAMING PLUS WIFI     |
| (MS-7E47): applying an effect to a header rewrites that  |
| header's row.  MSI Center uses IDs 0x00 .. 0x08; only    |
| Off / Static / Breathing could be identified with        |
| certainty (the standalone reverse engineering of the     |
| older B860 boards numbers wave as 0x01 and rainbow as    |
| 0x05), so the remaining presets are not offered until    |
| their UI names are matched up.                           |
|                                                           |
| NB: these IDs are not the 112/162/185-byte protocol's     |
| MSI_MODE values, hence this explicit translation table.   |
\*---------------------------------------------------------*/
struct mystic_light_50_effect
{
    int             mode;       // OpenRGB-visible mode value for this driver
    unsigned char   effect;     // byte written to MSI_50_ROW_COL_EFFECT
    unsigned char   sub;        // byte written to MSI_50_ROW_COL_SUB
    bool            palette;    // the effect reads the row's palette slots
};

static const mystic_light_50_effect effect_50_table[] =
{
    { MSI_MODE_DISABLE,            MSI_50_EFFECT_DISABLE,        MSI_50_SUB_PLAIN,      false }, // Off      0x00
    { MSI_MODE_STATIC,             MSI_50_EFFECT_STATIC,         MSI_50_SUB_PLAIN,      true  }, // Steady   0x02
    { MSI_MODE_BREATHING,          MSI_50_EFFECT_BREATHING,      MSI_50_SUB_PLAIN,      true  }, // Breath   0x04
    { MSI_MODE_FIRE,               MSI_50_EFFECT_FIRE,           MSI_50_SUB_FOUR_COLOR, true  }, // Flame     0x03
    { MSI_MODE_COLOR_WAVE,         MSI_50_EFFECT_COLOR_WAVE,     MSI_50_SUB_GENERATED,  true  }, // Wave      0x01
    { MSI_MODE_COLOR_RING,         MSI_50_EFFECT_COLOR_RING,     MSI_50_SUB_GENERATED,  true  }, // Color ring 0x05
    { MSI_MODE_LIGHTNING,          MSI_50_EFFECT_LIGHTNING,      MSI_50_SUB_PLAIN,      true  }, // Lightning 0x06
    { MSI_MODE_METEOR,             MSI_50_EFFECT_METEOR,         MSI_50_SUB_GENERATED,  true  }, // Meteor    0x08

    /*-------------------------------------------------*\
    | ID 0x07 (MSI Center "Recreation") is deliberately |
    | absent: it produced no light from MSI Center's    |
    | own captured frame, nor with sub 0x01/0x03, the   |
    | palette or generated colors, or cycle 30/60 -     |
    | while a 0x04 Breath written into the same row was |
    | clearly visible, so the row itself was fine.      |
    \*-------------------------------------------------*/
};

static const mystic_light_50_effect * FindEffect50ById(unsigned char effect_id)
{
    for(std::size_t i = 0; i < sizeof(effect_50_table) / sizeof(effect_50_table[0]); i++)
    {
        if(effect_50_table[i].effect == effect_id)
        {
            return &effect_50_table[i];
        }
    }

    return nullptr;
}

static const mystic_light_50_effect * FindEffect50(int mode)
{
    for(std::size_t i = 0; i < sizeof(effect_50_table) / sizeof(effect_50_table[0]); i++)
    {
        if(effect_50_table[i].mode == mode)
        {
            return &effect_50_table[i];
        }
    }

    return nullptr;
}

static const std::vector<MSI_ZONE> zone_set1 =
{
        MSI_ZONE_JAF,
        MSI_ZONE_JARGB_1,
        MSI_ZONE_JARGB_2,
        MSI_ZONE_JARGB_3,
};

static const std::vector<MSI_ZONE> zone_set2 =
{
    MSI_ZONE_JAF,
    MSI_ZONE_JARGB_1,
    MSI_ZONE_JARGB_2,
    MSI_ZONE_JARGB_3,
    MSI_ZONE_J_PIPE_1,
};

static const std::string board_names[] =
{
    "MSI MAG X870 TOMAHAWK WIFI (MS-7E51)",
    "MSI MAG B850M MORTAR WIFI (MS-7E61)",
    "MSI MPG B850I EDGE TI WIFI (MS-7E79)",
    "MSI X870 GAMING PLUS WIFI (MS-7E47)",
    "MSI B850 GAMING PLUS WIFI6E (MS-7E80)",
    "MSI B850M GAMING PLUS WIFI6E (MS-7E81)",
    "MSI MPG X870E CARBON WIFI (MS-7E49)",
    "MSI Z890 GAMING PLUS WIFI (MS-7E34)",
    "MSI Z890 GAMING PLUS WIFI6E (MS-7E34)",
    "MSI X870E GAMING PLUS WIFI (MS-7E70)",
    "MSI MAG X870E TOMAHAWK WIFI (MS-7E59)",
    "MSI PRO B850-P WIFI (MS-7E56)",
    "MSI B850M GAMING PLUS WIFI (MS-7E66)",
    "MSI PRO X870E-P WIFI (MS-7E70)",
    "MSI MPG X870I EDGE TI EVO WIFI (MS-7E50)",
    "MSI B850 GAMING PLUS WIFI (MS-7E56)",
    "MSI PRO X870-P WIFI (MS-7E47)",
    "MSI MPG X870E EDGE TI WIFI (MS-7E59)",
    "MSI MAG B850 TOMAHAWK MAX WIFI (MS-7E62)",
    "MSI PRO B850M-P WIFI (MS-7E71)",
    "MSI MAG Z890 TOMAHAWK WIFI (MS-7E32)",
    "MSI MPG B850 EDGE TI WIFI (MS-7E62)",
    "MSI PRO B850M-VC WIFI6E (MS-7E71)",
    "MSI MAG B850 TOMAHAWK WIFI (MS-7E53)",
    "MSI MEG Z890 UNIFY-X (MS-7E20)",
    "MSI PRO X870E-S EVO WIFI (MS-7E86)",
    "MSI MAG X870E GAMING MAX WIFI (MS-7E86)",
    "MSI PRO Z890-P WIFI (MS-7E34)",
    "MSI B850 GAMING PLUS WIFI PZ (MS-7E75)",
    "MSI MAG X870E TOMAHAWK MAX WIFI PZ (MS-7E84)",
    "MSI PRO B850M-A WIFI PZ (MS-7E78)",
    "MSI B850MPOWER (MS-7E83)",
    "MSI B840M GAMING WIFI6E (MS-7E76)",
    "MSI PRO B850M-A WIFI (MS-7E66)",
    "MSI PRO B850-S WIFI6E (MS-7E80)",
    "MSI B850M GAMING WIFI (MS-7E82)",
    "MSI PRO B850-S EVO WIFI (MS-7E97)",
    "MSI B850M GAMING WIFI6E (MS-7E87)"
};

static const mystic_light_761_config board_configs[] =
{
    { &(board_names[0]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI X870 TOMAHAWK WIFI
    { &(board_names[1]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI MAG B850M MORTAR WIFI
    { &(board_names[2]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI MPG B850I EDGE TI WIFI
    { &(board_names[3]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED, true, 30 },  // MSI X870 GAMING PLUS WIFI (validated with a MSI Center capture, LED count 30 as MSI Center uses)
    { &(board_names[4]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI B850 GAMING PLUS WIFI6E
    { &(board_names[5]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI B850M GAMING PLUS WIFI6E
    { &(board_names[6]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI MPG X870E CARBON WIFI
    { &(board_names[7]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI Z890 GAMING PLUS WIFI
    { &(board_names[8]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI Z890 GAMING PLUS WIFI 6E
    { &(board_names[9]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI X870E GAMING PLUS WIFI
    { &(board_names[10]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },      // MSI MAG X870E TOMAHAWK WIFI
    { &(board_names[11]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850-P WIFI (MS-7E56)
    { &(board_names[12]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850M GAMING PLUS WIFI
    { &(board_names[13]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO X870E-P WIFI
    { &(board_names[14]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MPG X870I EDGE TI EVO WIFI (MS-7E50)
    { &(board_names[15]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850 GAMING PLUS WIFI (MS-7E56)
    { &(board_names[16]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO X870-P WIFI
    { &(board_names[17]), 0,  10, 0, 1, &zone_set2,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MPG X870E EDGE TI WIFI
    { &(board_names[18]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MAG B850 TOMAHAWK MAX WIFI
    { &(board_names[19]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850M-P WIFI (MS-7E71)
    { &(board_names[20]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MAG Z890 TOMAHAWK WIFI (MS-7E32)
    { &(board_names[21]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MPG B850 EDGE TI WIFI (MS-7E62)
    { &(board_names[22]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850M-VC WIFI6E (MS-7E71)
    { &(board_names[23]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MAG B850 TOMAHAWK WIFI (MS-7E53)
    { &(board_names[24]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MEG Z890 UNIFY-X (MS-7E20)
    { &(board_names[25]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO X870E-S EVO WIFI (MS-7E86)
    { &(board_names[26]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MAG X870E GAMING MAX WIFI (MS-7E86)
    { &(board_names[27]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO Z890-P WIFI (MS-7E34)
    { &(board_names[28]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850 GAMING PLUS WIFI PZ (MS-7E75)
    { &(board_names[29]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI MAG X870E TOMAHAWK MAX WIFI PZ (MS-7E84)
    { &(board_names[30]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850M-A WIFI PZ (MS-7E78)
    { &(board_names[31]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850MPOWER (MS-7E83)
    { &(board_names[32]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B840M GAMING WIFI6E (MS-7E76)
    { &(board_names[33]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850M-A WIFI (MS-7E66)
    { &(board_names[34]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850-S WIFI6E (MS-7E80)
    { &(board_names[35]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850M GAMING WIFI (MS-7E82)
    { &(board_names[36]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI PRO B850-S EVO WIFI (MS-7E97)
    { &(board_names[37]), 0,  0,  0, 1, &zone_set1,  MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED },     // MSI B850M GAMING WIFI6E (MS-7E87)
};

enum MSI_ZONE setup_map [] =
    {
        MSI_ZONE_JARGB_1,
        MSI_ZONE_JARGB_2,
        MSI_ZONE_JARGB_3,
        MSI_ZONE_JAF,
        MSI_ZONE_J_PIPE_1,
        MSI_ZONE_J_PIPE_2,
        MSI_ZONE_J_PIPE_3,
        MSI_ZONE_J_PIPE_4,
        MSI_ZONE_J_PIPE_5,
        MSI_ZONE_J_RGB_1,
        MSI_ZONE_J_RGB_2,
        MSI_ZONE_ON_BOARD_LED_0,
        MSI_ZONE_ON_BOARD_LED_1,
        MSI_ZONE_ON_BOARD_LED_2,
        MSI_ZONE_ON_BOARD_LED_3,
        MSI_ZONE_ON_BOARD_LED_4,
        MSI_ZONE_ON_BOARD_LED_5,

};

int get_zone_setup_index(MSI_ZONE index)
{
    int size = sizeof(setup_map) / sizeof(setup_map[0]);
    for(int i = 0; i < size; i++)
    {
        if(setup_map[i] == index)
        {
            return i;
        }
    }
    return -1;
}

// Copying from signal plugin
// DO NOT MODIFY THIS ARRAY DIRECTLY, MAKE COPY
unsigned char initial_setup_array[] =
{
    0x50,
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x15, 0x78, //JARGB 1
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x15, 0x78, //JARGB 2
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x15, 0x78, //JARGB 3
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x15, 0x78, //JAF //Fans go here with the weird connector
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x95, 0x1E, //JPIPE1
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //JPIPE2 //95 for active zones 94 for inactive
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x95, 0x1E, //JPIPE3
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //JPIPE4
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //JPIPE5
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x95, 0x1E, //JRGB1
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //JRGB2
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard1
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard2
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard3
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard4
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard5
    0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94, 0x1E, //Onboard6
    0x09, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x95, 0x1E, //Select all?
    0x00
};

unsigned char * initializer_array()
{
    unsigned char * arr = (unsigned char *) malloc(SETUP_ARRAY_SIZE);
    for(int i = 0; i < SETUP_ARRAY_SIZE; i++)
    {
        arr[i] = initial_setup_array[i];
    }
    return arr;
}

void init_packet(FeaturePacket_Zone_761 * packet)
{
    packet->dirty         = false;
    packet->packet.fixed1 = 0x09;
    packet->packet.fixed2 = 0x00;
    packet->packet.fixed3 = 0x00;
    packet->packet.hdr2   = 240;

    for(int i = 0; i < NUM_LEDS_761; i++)
    {
        packet->packet.colors[i] = 0x0;
    }

}

MSIMotherboard761Controller::MSIMotherboard761Controller(hid_device* handle, const char* path, std::string dev_name)
{
    dev                                             = handle;
    location                                        = path;
    name                                            = dev_name;

    const mystic_light_761_config * board_config    = nullptr;
    for(std::size_t i = 0; i < NUM_CONFS; i++)
    {
        if(*(board_configs[i].name) == name)
        {
            board_config = &board_configs[i];
            break;
        }
    }

    if(board_config != nullptr)
    {
        supported_zones     = (std::vector<MSI_ZONE>*) board_config->supported_zones;
        numof_pipe1_leds    = board_config->numof_pipe1_leds;
        hw_modes            = board_config->supports_hw_modes;
        default_lednum      = board_config->default_lednum;
        direct_mode_active  = true;
        unsigned int max    = 0;

        for(std::size_t i = 0; i < board_config->supported_zones[0].size(); i++)
        {
            unsigned int curr_val = (unsigned int) (board_config->supported_zones[0][i]);

            if(curr_val > max)
            {
                max = curr_val;
            }
        }

        /*-------------------------------------------------*\
        | Need to send configuration to board               |
        \*-------------------------------------------------*/
        unsigned char * conf_arr = initializer_array();

        /*-------------------------------------------------*\
        | First set everything off                          |
        \*-------------------------------------------------*/
        for(int i = 4; i < 17; i++)
        {
            conf_arr[ARRAY_ROW_RAW(i, 1)]  = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 2)]  = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 6)]  = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 10)] = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 11)] = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 12)] = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 13)] = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 14)] = 0x00;
            conf_arr[ARRAY_ROW_RAW(i, 15)] = 0x00;
        }

        for(std::size_t i = 0; i < supported_zones->size(); i++)
        {
            MSI_ZONE supp_zone = (*supported_zones)[i];
            ZoneConfig conf;
            conf.msi_zone = supp_zone;
            /*---------------------------------------------*\
            | Turn on relevant zones (0-3 are always        |
            | active)                                       |
            \*---------------------------------------------*/
            if(get_zone_setup_index(supp_zone) > 3)
            {
                conf_arr[ARRAY_ROW(supp_zone,1 )] = 0x09;
                conf_arr[ARRAY_ROW(supp_zone,2 )] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,6 )] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,10)] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,11)] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,12)] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,13)] = 0xFF;
                conf_arr[ARRAY_ROW(supp_zone,14)] = 0x03;
                conf_arr[ARRAY_ROW(supp_zone,15)] = 0x95;
            }
            ZoneData * dat = new ZoneData;
            conf.zone_data = dat;
            zone_configs.push_back(conf);
        }

        if(dev)
        {
            ReadName();
            ReadFwVersion();
            ReadSettings();

            /*---------------------------------------------*\
            | On these boards the effect state lives in the |
            | 0x50 report itself, so read it back before    |
            | pushing a configuration: otherwise detecting  |
            | (or resizing a zone) would overwrite whatever |
            | the board is running, and the UI would claim  |
            | Direct while an effect is in fact lit.        |
            \*---------------------------------------------*/
            if(hw_modes)
            {
                ReadEffectRows(conf_arr);
            }

            /*---------------------------------------------*\
            | Push config so setting colors works           |
            \*---------------------------------------------*/
            int res = hid_send_feature_report(dev, conf_arr, SETUP_ARRAY_SIZE);
            LOG_INFO("Sending configuration resulted in %i\n", res);

            data                        = new FeaturePacket_761;

            data->jaf.zone              = MSI_ZONE_JAF;
            data->jargb1.zone           = MSI_ZONE_JARGB_1;
            data->jargb2.zone           = MSI_ZONE_JARGB_2;
            data->jargb3.zone           = MSI_ZONE_JARGB_3;
            data->jpipe1.zone           = MSI_ZONE_J_PIPE_1;

            data->jaf.packet.hdr0       = 0x08;
            data->jargb1.packet.hdr0    = 0x04;
            data->jargb2.packet.hdr0    = 0x04;
            data->jargb3.packet.hdr0    = 0x04;
            data->jpipe1.packet.hdr0    = 0x06;

            data->jaf.packet.hdr1       = 0x00;
            data->jargb1.packet.hdr1    = 0x00;
            data->jargb2.packet.hdr1    = 0x01;
            data->jargb3.packet.hdr1    = 0x02;
            data->jpipe1.packet.hdr1    = 0x00;

            init_packet(&data->jaf);
            init_packet(&data->jargb1);
            init_packet(&data->jargb2);
            init_packet(&data->jargb3);
            init_packet(&data->jpipe1);
            data->jpipe1.packet.hdr2 = numof_pipe1_leds;
        }

        /*-------------------------------------------------*\
        | Sent again when a zone is resized                 |
        \*-------------------------------------------------*/
        setup_data  = conf_arr;
        setup_dirty = false;
    }
    else
    {
        throw std::runtime_error(BOARD_UNSUPPORTED_ERROR);
    }

}

MSIMotherboard761Controller::~MSIMotherboard761Controller()
{
    hid_close(dev);

    if(data)
    {
        delete data;
        data = nullptr;
    }

    if(setup_data)
    {
        free(setup_data);
        setup_data = nullptr;
    }

    for(ZoneConfig& zone : zone_configs)
    {
        if(zone.zone_data)
        {
            delete zone.zone_data;
            zone.zone_data = nullptr;
        }
    }
    zone_configs.clear();
}

void MSIMotherboard761Controller::SetMode
    (
    MSI_ZONE        zone,
    int             mode,
    MSI_SPEED       speed,
    MSI_BRIGHTNESS  brightness,
    bool            rainbow_color,
    bool            direction_left
    )
{
    if(!hw_modes)
    {
        return; // only direct mode is known to work on this board
    }

    /*-----------------------------------------------------*\
    | Look up the ID this protocol uses for the requested   |
    | mode; modes without a captured ID are not offered     |
    \*-----------------------------------------------------*/
    const mystic_light_50_effect* effect = FindEffect50(mode);

    if(effect == nullptr)
    {
        LOG_WARNING("[%s]: no 0x50 effect ID known for mode %d, mode not applied", name.c_str(), mode);
        return;
    }

    /*-----------------------------------------------------*\
    | Update the zone's effect state; each zone's effect is |
    | written into its own row of the 0x50 report           |
    \*-----------------------------------------------------*/
    ZoneConfig* conf = FindZoneConfig(zone);

    if(conf == nullptr)
    {
        return;
    }

    conf->effect.mode            = mode;
    conf->effect.effect          = effect->effect;
    conf->effect.sub             = effect->sub;
    conf->effect.speed           = speed;
    conf->effect.brightness      = brightness;
    conf->effect.rainbow_color   = rainbow_color;
    conf->effect.use_palette     = effect->palette && !rainbow_color;
    conf->effect.direction_left  = direction_left;
    conf->effect.valid           = true;

    direct_mode_active           = false;
}

MSIMotherboard761Controller::ZoneConfig* MSIMotherboard761Controller::FindZoneConfig
    (
    MSI_ZONE zone
    )
{
    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        if(zone_configs[i].msi_zone == zone)
        {
            return &zone_configs[i];
        }
    }

    return nullptr;
}

unsigned char MSIMotherboard761Controller::BrightnessTo50
    (
    MSI_BRIGHTNESS brightness
    )
{
    /*-----------------------------------------------------*\
    | This report uses brightness 1..5 (5 = maximum), with  |
    | 0 switching the effect off.  OpenRGB's MSI_BRIGHTNESS |
    | runs 0..10.                                           |
    \*-----------------------------------------------------*/
    if(brightness <= MSI_BRIGHTNESS_OFF)
    {
        return MSI_50_BRIGHTNESS_OFF;
    }

    unsigned char level = (unsigned char)(((unsigned int)brightness + 1) / 2);

    if(level < MSI_50_BRIGHTNESS_MIN)
    {
        level = MSI_50_BRIGHTNESS_MIN;
    }
    if(level > MSI_50_BRIGHTNESS_MAX)
    {
        level = MSI_50_BRIGHTNESS_MAX;
    }

    return level;
}

void MSIMotherboard761Controller::SetZonePalette
    (
    MSI_ZONE        zone,
    const Color     palette[],
    unsigned int    num_colors
    )
{
    if(!hw_modes)
    {
        return;
    }

    ZoneConfig* conf = FindZoneConfig(zone);

    if(conf == nullptr)
    {
        return;
    }

    for(unsigned int i = 0; (i < num_colors) && (i < MSI_50_NUM_PALETTE_COLORS); i++)
    {
        conf->effect.palette[i] = palette[i];
    }
}

bool MSIMotherboard761Controller::ReadEffectRows(unsigned char* arr)
{
    if((dev == nullptr) || (arr == nullptr))
    {
        return false;
    }

    unsigned char buffer[SETUP_ARRAY_SIZE];
    buffer[0] = 0x50;
    memset(&buffer[1], 0x00, sizeof(buffer) - 1);

    if(hid_get_feature_report(dev, buffer, sizeof(buffer)) < (int)sizeof(buffer))
    {
        LOG_INFO("[%s]: could not read the 0x50 report back, using the default configuration", name.c_str());
        return false;
    }

    /*-----------------------------------------------------*\
    | Adopt the rows as the controller holds them, so a     |
    | configuration send cannot clobber a running effect.   |
    | The apply byte is never carried over: a configuration |
    | send must not commit to controller flash.             |
    \*-----------------------------------------------------*/
    memcpy(&arr[1], &buffer[1], SETUP_ARRAY_SIZE - 1);
    arr[MSI_50_IDX_APPLY] = 0x00;

    int adopted = 0;

    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        int idx = get_zone_setup_index(zone_configs[i].msi_zone);

        if(idx < 0)
        {
            continue;
        }

        zone_configs[i].adopted_lednum = arr[ARRAY_ROW_RAW(idx, MSI_50_ROW_COL_LEDNUM)];

        unsigned char sub = arr[ARRAY_ROW_RAW(idx, MSI_50_ROW_COL_SUB)];
        unsigned char par = arr[ARRAY_ROW_RAW(idx, MSI_50_ROW_COL_PARAM)];

        /*-------------------------------------------------*\
        | An unknown id (the per-LED marker among them) is  |
        | not an effect this driver models, and a row whose |
        | parameter byte has bit 7 set is the controller's  |
        | idle row - the state it holds after power has been|
        | lost, and one the controller ignores.  Neither is |
        | adopted; only the LED count is kept from it.      |
        \*-------------------------------------------------*/
        const mystic_light_50_effect* effect = FindEffect50ById(arr[ARRAY_ROW_RAW(idx, MSI_50_ROW_COL_EFFECT)]);

        if((effect == nullptr) || ((par & ~MSI_50_PARAM_ZONE_MASK) != 0))
        {
            continue;
        }

        Effect50& state   = zone_configs[i].effect;

        state.mode          = effect->mode;
        state.effect        = effect->effect;
        state.sub           = sub;
        state.speed         = (MSI_SPEED)(par & MSI_50_PARAM_SPEED_MASK);
        state.brightness    = (MSI_BRIGHTNESS)(2 * ((par & MSI_50_PARAM_BRIGHTNESS_MASK) >> MSI_50_PARAM_BRIGHTNESS_SHIFT));
        state.use_palette   = (par & MSI_50_PARAM_USE_PALETTE) != 0;
        state.rainbow_color = !state.use_palette;
        state.direction_left= (par & MSI_50_PARAM_DIRECTION_LEFT) != 0;
        state.valid         = true;

        for(int c = 0; c < MSI_50_NUM_PALETTE_COLORS; c++)
        {
            int col = MSI_50_ROW_COL_COLOR1 + (c * 3);

            state.palette[c].R = arr[ARRAY_ROW_RAW(idx, col    )];
            state.palette[c].G = arr[ARRAY_ROW_RAW(idx, col + 1)];
            state.palette[c].B = arr[ARRAY_ROW_RAW(idx, col + 2)];
        }

        adopted++;
        direct_mode_active = false;
    }

    LOG_INFO("[%s]: adopted %d effect row(s) from the controller", name.c_str(), adopted);

    return true;
}

unsigned char MSIMotherboard761Controller::GetZoneLedNum
    (
    MSI_ZONE zone
    )
{
    ZoneConfig* conf = FindZoneConfig(zone);

    if((conf != nullptr) && (conf->adopted_lednum > 0))
    {
        return conf->adopted_lednum;
    }

    return (unsigned char)default_lednum;
}

void MSIMotherboard761Controller::WriteEffectRows()
{
    if(setup_data == nullptr)
    {
        return;
    }

    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        const ZoneConfig& conf = zone_configs[i];

        if(!conf.effect.valid)
        {
            continue;
        }

        /*-------------------------------------------------*\
        | Pack the row's parameter byte.  Bit 7 is kept     |
        | clear: MSI Center never sets it in a zone row and |
        | the controller ignores the row when it is.        |
        \*-------------------------------------------------*/
        unsigned char param = 0;

        param  = (unsigned char)(((unsigned int)conf.effect.speed << MSI_50_PARAM_SPEED_SHIFT) & MSI_50_PARAM_SPEED_MASK);
        param |= (unsigned char)((BrightnessTo50(conf.effect.brightness) << MSI_50_PARAM_BRIGHTNESS_SHIFT) & MSI_50_PARAM_BRIGHTNESS_MASK);

        /*-------------------------------------------------*\
        | Random (generated) colors clear the palette bit,  |
        | custom colors use the row's palette slots         |
        \*-------------------------------------------------*/
        if(conf.effect.use_palette)
        {
            param |= MSI_50_PARAM_USE_PALETTE;
        }

        if(conf.effect.direction_left)
        {
            param |= MSI_50_PARAM_DIRECTION_LEFT;
        }

        /*-------------------------------------------------*\
        | Bit 7 is masked off: MSI Center never sets it in  |
        | a zone row and the controller ignores the row    |
        | when it is (measured on MS-7E47).  It is only an  |
        | animate flag in the command row of the older     |
        | B860 boards.                                      |
        \*-------------------------------------------------*/
        param &= MSI_50_PARAM_ZONE_MASK;

        MSI_ZONE zone = conf.msi_zone;

        /*-------------------------------------------------*\
        | An effect only reaches a header whose row has a   |
        | non-zero LED count (measured on MS-7E47), so keep |
        | the row sized the way MSI Center sizes it.        |
        \*-------------------------------------------------*/
        if(setup_data[ARRAY_ROW(zone, MSI_50_ROW_COL_LEDNUM)] == 0)
        {
            setup_data[ARRAY_ROW(zone, MSI_50_ROW_COL_LEDNUM)] = (unsigned char)default_lednum;
        }

        setup_data[ARRAY_ROW(zone, MSI_50_ROW_COL_EFFECT)] = conf.effect.effect;
        setup_data[ARRAY_ROW(zone, MSI_50_ROW_COL_SUB)]    = conf.effect.sub;
        setup_data[ARRAY_ROW(zone, MSI_50_ROW_COL_PARAM)]  = param;

        for(int c = 0; c < MSI_50_NUM_PALETTE_COLORS; c++)
        {
            int col = MSI_50_ROW_COL_COLOR1 + (c * 3);

            setup_data[ARRAY_ROW(zone, col    )] = conf.effect.palette[c].R;
            setup_data[ARRAY_ROW(zone, col + 1)] = conf.effect.palette[c].G;
            setup_data[ARRAY_ROW(zone, col + 2)] = conf.effect.palette[c].B;
        }
    }
}

bool MSIMotherboard761Controller::UpdateEffects
    (
    bool save
    )
{
    bool any_effect = false;

    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        any_effect = any_effect || zone_configs[i].effect.valid;
    }

    if((dev == nullptr) || (setup_data == nullptr) || !any_effect)
    {
        return false;
    }

    WriteEffectRows();

    /*-----------------------------------------------------*\
    | Byte 289 commits the frame to the controller's      |
    | flash, 0x00 only changes the frame held in RAM.  A  |
    | committed frame is what the board shows after power |
    | is lost, measured by replaying MSI Center's own     |
    | commit and cold-cycling the machine.  The byte is   |
    | set only on an explicit save, to spare the flash.   |
    |                                                     |
    | Note that a warm reboot is not a test of this: 5V   |
    | standby power keeps the RAM frame alive, so the same|
    | lights come back either way.                        |
    \*-----------------------------------------------------*/
    setup_data[MSI_50_IDX_APPLY] = save ? 0x01 : 0x00;

    if(hid_send_feature_report(dev, setup_data, SETUP_ARRAY_SIZE) < 0)
    {
        LOG_ERROR("[%s]: failed to send effect settings", name.c_str());
        return false;
    }

    setup_dirty = false;

    /*-------------------------------------------------*\
    | Drop the flash commit again so it cannot be       |
    | replayed by a later configuration send.           |
    \*-------------------------------------------------*/
    setup_data[MSI_50_IDX_APPLY] = 0x00;

    return true;
}

std::string MSIMotherboard761Controller::GetDeviceName()
{
    return name;
}

std::string MSIMotherboard761Controller::GetFWVersion()
{
    return std::string("AP/LD ").append(version_aprom).append(" / ").append(version_ldrom);
}

std::string MSIMotherboard761Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string MSIMotherboard761Controller::GetSerial()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

bool MSIMotherboard761Controller::ReadSettings()
{
    /*-----------------------------------------------------*\
    | Read packet from hardware, return true if successful  |
    \*-----------------------------------------------------*/
    unsigned char buffer [500];
    buffer[0] = 0x51;
    return (hid_get_feature_report(dev, buffer, 500)) > 0 ;
}

bool MSIMotherboard761Controller::Update
    (
    bool save
    )
{
    /*-----------------------------------------------------*\
    | Hardware modes are driven by the 0x50 report's command|
    | row; the per-LED packets must not be sent in that     |
    | case or they overwrite the effect.                    |
    \*-----------------------------------------------------*/
    if(hw_modes && !direct_mode_active)
    {
        return UpdateEffects(save);
    }

    bool flag = true;
    FeaturePacket_Zone_761* packets[] = { &data->jaf, &data->jargb1, &data->jargb2, &data->jargb3, &data->jpipe1 };

    SendConfiguration();

    /*-----------------------------------------------------*\
    | Only send changed zones on boards with JPIPE1 so     |
    | updating onboard LEDs does not clear the headers.    |
    \*-----------------------------------------------------*/
    for(unsigned int i = 0; i < sizeof(packets) / sizeof(packets[0]); i++)
    {
        FeaturePacket_Zone_761* packet = packets[i];

        if((packet->zone == MSI_ZONE_J_PIPE_1) && (numof_pipe1_leds == 0))
        {
            continue;
        }

        if((numof_pipe1_leds > 0) && !packet->dirty)
        {
            continue;
        }

        if(hid_send_feature_report(dev, GET_CHAR_PTR_REF(packet->packet), sizeof(FeaturePacket_PerLED_761)) < 0)
        {
            flag = false;
        }
        else
        {
            packet->dirty = false;
        }
    }

    return flag;
}

void MSIMotherboard761Controller::SetZoneColor
    (
    MSI_ZONE        zone,
    unsigned char   red1,
    unsigned char   grn1,
    unsigned char   blu1,
    unsigned char   red2,
    unsigned char   grn2,
    unsigned char   blu2
    )
{
    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        if(zone_configs[i].msi_zone == zone)
        {
            zone_configs[i].zone_data->color.R = red1;
            zone_configs[i].zone_data->color.G = grn1;
            zone_configs[i].zone_data->color.B = blu1;
            zone_configs[i].zone_data->color2.R = red2;
            zone_configs[i].zone_data->color2.G = grn2;
            zone_configs[i].zone_data->color2.B = blu2;
        }
    }

    if(hw_modes)
    {
        /*-------------------------------------------------*\
        | Palette slot 1 is what the single-colour effects  |
        | use; MSI Center also fills slots 3 and 4 with the |
        | default colors, but keeping the two given colors  |
        | in the first slots is what drives the effect.     |
        \*-------------------------------------------------*/
        ZoneConfig* conf = FindZoneConfig(zone);

        if(conf != nullptr)
        {
            Color color1 { red1, grn1, blu1 };
            Color color2 { red2, grn2, blu2 };

            conf->effect.palette[0] = color1;
            conf->effect.palette[1] = color2;
            conf->effect.palette[2] = color1;
            conf->effect.palette[3] = color2;
        }
    }
}

void set_data_color(FeaturePacket_Zone_761 * packet, std::size_t index, unsigned char color_val)
{
    if(packet == nullptr)
    {
        return;
    }
    packet->packet.colors[index] = color_val;
}

void MSIMotherboard761Controller::SetLedColor
    (
    MSI_ZONE        zone,
    std::size_t     index,
    unsigned char   red,
    unsigned char   grn,
    unsigned char   blu
    )
{
    FeaturePacket_Zone_761 * ptr = nullptr;
    switch(zone)
    {
    case MSI_ZONE_JAF:
        ptr = &data->jaf;
        break;
    case MSI_ZONE_JARGB_1:
        ptr = &data->jargb1;
        break;
    case MSI_ZONE_JARGB_2:
        ptr = &data->jargb2;
        break;
    case MSI_ZONE_JARGB_3:
        ptr = &data->jargb3;
        break;
    case MSI_ZONE_J_PIPE_1:
        ptr = &data->jpipe1;
        break;
    default:
        break;
    }

    std::size_t candidate_index = (index * 3);

    if((ptr != nullptr) && (index < GetMaxDirectLeds(zone)))
    {
        set_data_color(ptr, candidate_index,     red);
        set_data_color(ptr, candidate_index + 1, grn);
        set_data_color(ptr, candidate_index + 2, blu);
        ptr->dirty = true;
    }
}

ZoneData *MSIMotherboard761Controller::GetZoneData
    (
    FeaturePacket_761&  /*data_packet*/,
    MSI_ZONE            zone
    )
{
    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        if(zone_configs[i].msi_zone == zone)
        {
            return zone_configs[i].zone_data;
        }
    }

    return nullptr;
}

Color *MSIMotherboard761Controller::GetPerLedZoneData
    (
    MSI_ZONE  zone
    )
{
    return &(GetZoneData(*data, zone)->color);
}

RainbowZoneData *MSIMotherboard761Controller::GetRainbowZoneData
    (
    MSI_ZONE /*zone*/
    )
{
    return nullptr;
}

bool MSIMotherboard761Controller::ReadFwVersion()
{
    return true;
}
void MSIMotherboard761Controller::ReadName()
{
    wchar_t tname[256];

    /*-----------------------------------------------------*\
    | Get the manufacturer string from HID                  |
    \*-----------------------------------------------------*/
    hid_get_manufacturer_string(dev, tname, 256);

    /*-----------------------------------------------------*\
    | Convert to std::string                                |
    \*-----------------------------------------------------*/
    name = StringUtils::wstring_to_string(tname);

    /*-----------------------------------------------------*\
    | Get the product string from HID                       |
    \*-----------------------------------------------------*/
    hid_get_product_string(dev, tname, 256);

    /*-----------------------------------------------------*\
    | Append the product string to the manufacturer string  |
    \*-----------------------------------------------------*/
    name.append(" ").append(StringUtils::wstring_to_string(tname));
}

int MSIMotherboard761Controller::GetMode()
{
    if(direct_mode_active)
    {
        return MSI_MODE_DIRECT_DUMMY;
    }

    /*-----------------------------------------------------*\
    | Report the first zone that has an effect set          |
    \*-----------------------------------------------------*/
    for(std::size_t i = 0; i < zone_configs.size(); i++)
    {
        if(zone_configs[i].effect.valid)
        {
            return zone_configs[i].effect.mode;
        }
    }

    return MSI_MODE_DIRECT_DUMMY;
}

void MSIMotherboard761Controller::GetMode
    (
    MSI_ZONE            zone,
    int &               mode,
    MSI_SPEED &         speed,
    MSI_BRIGHTNESS &    brightness,
    bool &              rainbow_color,
    unsigned int &      color
    )
{
    /*-----------------------------------------------------*\
    | On boards with hardware presets the per-zone effect   |
    | state is the authoritative source                     |
    \*-----------------------------------------------------*/
    ZoneConfig* conf = FindZoneConfig(zone);

    if((conf != nullptr) && conf->effect.valid)
    {
        mode          = conf->effect.mode;
        speed         = conf->effect.speed;
        brightness    = conf->effect.brightness;
        rainbow_color = conf->effect.rainbow_color;
        color         = ToRGBColor(conf->effect.palette[0].R, conf->effect.palette[0].G, conf->effect.palette[0].B);
        return;
    }

    /*-----------------------------------------------------*\
    | Get data for given zone                               |
    \*-----------------------------------------------------*/
    ZoneData *zone_data = GetZoneData(*data, zone);

    /*-----------------------------------------------------*\
    | Return if zone is invalid                             |
    \*-----------------------------------------------------*/
    if(!zone_data)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Update pointers with data                             |
    \*-----------------------------------------------------*/
    // Actual support of non direct modes needs to be further investigated
    mode            = (MSI_MODE)zone_data->effect;
    speed           = (MSI_SPEED)(zone_data->speedAndBrightnessFlags & 0x03);
    brightness      = (MSI_BRIGHTNESS)((zone_data->speedAndBrightnessFlags >> 2) & 0x1F);
    rainbow_color   = (zone_data->colorFlags & 0x80) == 0 ? true : false;
    color           = ToRGBColor(zone_data->color.R, zone_data->color.G, zone_data->color.B);
}

void MSIMotherboard761Controller::SetZoneLEDCount
    (
    MSI_ZONE        zone,
    unsigned char   led_num
    )
{
    FeaturePacket_Zone_761* packet = nullptr;

    if(!dev)
    {
        return;
    }

    switch(zone)
    {
        case MSI_ZONE_JAF:
            packet = &data->jaf;
            break;
        case MSI_ZONE_JARGB_1:
            packet = &data->jargb1;
            break;
        case MSI_ZONE_JARGB_2:
            packet = &data->jargb2;
            break;
        case MSI_ZONE_JARGB_3:
            packet = &data->jargb3;
            break;
        default:
            return;
    }

    /*-----------------------------------------------------*\
    | LED count is the last byte of the zone's config row   |
    \*-----------------------------------------------------*/
    if(setup_data[ARRAY_ROW(zone, 16)] != led_num)
    {
        setup_data[ARRAY_ROW(zone, 16)] = led_num;
        setup_dirty                     = true;
    }

    packet->packet.hdr2 = led_num;
}

void MSIMotherboard761Controller::SendConfiguration()
{
    if(!dev || !setup_dirty)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Stays dirty on failure so the next update tries again |
    \*-----------------------------------------------------*/
    /*-----------------------------------------------------*\
    | This path only carries zone configuration, so make    |
    | sure a flash commit left over from an earlier save    |
    | cannot be replayed here.                              |
    \*-----------------------------------------------------*/
    setup_data[MSI_50_IDX_APPLY] = 0x00;

    if(hid_send_feature_report(dev, setup_data, SETUP_ARRAY_SIZE) < 0)
    {
        LOG_ERROR("[%s]: failed to send configuration", name.c_str());
        return;
    }

    setup_dirty = false;
}

void MSIMotherboard761Controller::SetDirectMode
    (
    bool mode
    )
{
    direct_mode_active = mode;

    /*-----------------------------------------------------*\
    | Leaving a hardware mode hands the row's colors back   |
    | to the per-LED packets, so clear the zone rows' effect|
    | IDs again (this is what the setup array leaves them at|
    | when the controller is created).                      |
    \*-----------------------------------------------------*/
    if(mode && (setup_data != nullptr))
    {
        for(std::size_t i = 0; i < zone_configs.size(); i++)
        {
            if(!zone_configs[i].effect.valid)
            {
                continue;
            }

            int idx = get_zone_setup_index(zone_configs[i].msi_zone);

            if(idx >= 0)
            {
                setup_data[ARRAY_ROW_RAW(idx, MSI_50_ROW_COL_EFFECT)] = MSI_50_EFFECT_PER_LED;
            }

            zone_configs[i].effect.valid = false;
            setup_dirty                  = true;
        }
    }

    SelectPerLedProtocol();
}

bool MSIMotherboard761Controller::IsDirectModeActive()
{
    return direct_mode_active;
}

size_t MSIMotherboard761Controller::GetMaxDirectLeds
    (
    MSI_ZONE zone
    )
{
    switch(zone)
    {
        case MSI_ZONE_JAF:
        case MSI_ZONE_JARGB_1:
        case MSI_ZONE_JARGB_2:
        case MSI_ZONE_JARGB_3:
            return 240;
            break;
        case MSI_ZONE_J_PIPE_1:
            return numof_pipe1_leds;
            break;
        default:
            return 1;
    }
}

void MSIMotherboard761Controller::SelectPerLedProtocol()
{
    return;
}

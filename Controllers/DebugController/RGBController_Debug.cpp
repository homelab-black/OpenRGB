/*---------------------------------------------------------*\
| RGBController_Debug.cpp                                   |
|                                                           |
|   Debug RGBController that can mimic various devices for  |
|   development and test purposes                           |
|                                                           |
|   Adam Honse (CalcProgrammer1)                25 Feb 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cstring>
#include "KeyboardLayoutManager.h"
#include "RGBController_Debug.h"

/**------------------------------------------------------------------*\
    @name Debug
    @category Unknown
    @type I2C
    @save :x:
    @direct :x:
    @effects :x:
    @detectors DetectDebugControllers
    @comment
\*-------------------------------------------------------------------*/

//0xFFFFFFFF indicates an unused entry in matrix
#define NA  0xFFFFFFFF

#define NUM_LAYOUTS 6

static const std::string layout_names[] =
{
    "Default",
    "ANSI QWERTY",
    "ISO QWERTY",
    "ISO QWERTZ",
    "ISO AZERTY",
    "JIS"
};

static unsigned int debug_keyboard_underglow_map[3][10] =
    { {    0,   1,   2,   3,   4,   5,   6,   7,   8,   9 },
      {   10,  11,  12,  13,  14,  15,  16,  17,  18,  19 },
      {   20,  21,  22,  23,  24,  25,  26,  27,  28,  29 } };

RGBController_Debug::RGBController_Debug(bool custom_controller, json debug_settings)
{
    bool        zone_single     = true;
    bool        zone_linear     = true;
    bool        zone_resizable  = false;
    bool        zone_keyboard   = false;
    bool        zone_underglow  = false;
    std::string name_setting    = "";
    std::string type_setting    = "keyboard";

    if(debug_settings.contains("name"))
    {
        name_setting = debug_settings["name"];
    }

    if(debug_settings.contains("type"))
    {
        type_setting = debug_settings["type"];
    }

    if(debug_settings.contains("single"))
    {
        zone_single = debug_settings["single"];
    }

    if(debug_settings.contains("linear"))
    {
        zone_linear = debug_settings["linear"];
    }

    if(debug_settings.contains("resizable"))
    {
        zone_resizable = debug_settings["resizable"];
    }

    if(debug_settings.contains("keyboard"))
    {
        zone_keyboard = debug_settings["keyboard"];
    }

    if(debug_settings.contains("underglow"))
    {
        zone_underglow = debug_settings["underglow"];
    }

    //if(json_kbd.contains("layout"))
    //{
    //    layout = json_kbd["layout"];
    //}
    //if(json_kbd.contains("size"))
    //{
    //    size = json_kbd["size"];
    //}

    if(custom_controller)
    {

    }
    else
    {
        if(type_setting == "motherboard")
        {
            name                = "Debug Motherboard";
            type                = DEVICE_TYPE_MOTHERBOARD;
        }
        else if(type_setting == "dram")
        {
            name                = "Debug DRAM";
            type                = DEVICE_TYPE_DRAM;
        }
        else if(type_setting == "gpu")
        {
            name                = "Debug GPU";
            type                = DEVICE_TYPE_GPU;
        }
        else if(type_setting == "keyboard")
        {
            name                = "Debug Keyboard";
            type                = DEVICE_TYPE_KEYBOARD;
        }
//            /*---------------------------------------------------------*\
//            | Create a debug Keyboard                                   |
//            \*---------------------------------------------------------*/
//            name                                        = new_kb.GetName();
//            type                                        = DEVICE_TYPE_KEYBOARD;
//            json json_kbd           = debug_settings["devices"][device_idx];
//            KEYBOARD_LAYOUT layout  = KEYBOARD_LAYOUT::KEYBOARD_LAYOUT_ANSI_QWERTY;
//            KEYBOARD_SIZE   size    = KEYBOARD_SIZE::KEYBOARD_SIZE_FULL;
//
//
//
//
//
//
//            KeyboardLayoutManager new_kb(layout, size);
//
//            /*---------------------------------------------------------*\
//            | Check for custom key inserts and swaps                    |
//            \*---------------------------------------------------------*/
//            std::vector<keyboard_led> change;
//            const char* change_keys = "change_keys";
//
//            const char* ins_row     = "ins_row";
//            const char* rmv_key     = "rmv_key";
//            const char* rmv_row     = "rmv_row";
//            const char* swp_key     = "swp_key";
//
//            const char* dbg_zone    = "Zone";
//            const char* dbg_row     = "Row";
//            const char* dbg_col     = "Col";
//            const char* dbg_val     = "Val";
//            const char* dbg_name    = "Name";
//            const char* dbg_opcode  = "Opcode";
//
//            if(json_kbd.contains(change_keys))
//            {
//                for(size_t i = 0; i < json_kbd[change_keys].size(); i++)
//                {
//                    keyboard_led* key = new keyboard_led;
//
//                    key->zone    = json_kbd[change_keys][i][dbg_zone];
//                    key->row     = json_kbd[change_keys][i][dbg_row];
//                    key->col     = json_kbd[change_keys][i][dbg_col];
//                    key->value   = json_kbd[change_keys][i][dbg_val];
//                    key->name    = json_kbd[change_keys][i][dbg_name].get_ref<const std::string&>().c_str();
//
//                    if(json_kbd[change_keys][i][dbg_opcode] == ins_row)
//                    {
//                        key->opcode  = KEYBOARD_OPCODE_INSERT_ROW;
//                    }
//                    else if(json_kbd[change_keys][i][dbg_opcode] == rmv_key)
//                    {
//                        key->opcode  = KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT;
//                    }
//                    else if(json_kbd[change_keys][i][dbg_opcode] == rmv_row)
//                    {
//                        key->opcode  = KEYBOARD_OPCODE_REMOVE_ROW;
//                    }
//                    else if(json_kbd[change_keys][i][dbg_opcode] == swp_key)
//                    {
//                        key->opcode  = KEYBOARD_OPCODE_SWAP_ONLY;
//                    }
//                    else
//                    {
//                        key->opcode  = KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT;
//                    }
//
//                    change.push_back(*key);
//                }
//            }
//
//            new_kb.ChangeKeys(change);
//
//
//            debug_keyboard->description                      = debug_keyboard->name;
//            debug_keyboard->description.append(" Debug Keyboard");
//            debug_keyboard->location                         = "Debug Keyboard Location";
//            debug_keyboard->version                          = "Debug Keyboard Version";
//            debug_keyboard->serial                           = "Debug Keyboard Serial";
//
//            /*---------------------------------------------------------*\
//            | Create a direct mode for the debug keyboard               |
//            \*---------------------------------------------------------*/
//            mode debug_keyboard_direct_mode;
//
//            debug_keyboard_direct_mode.name                  = "Direct";
//            debug_keyboard_direct_mode.value                 = 0;
//            debug_keyboard_direct_mode.flags                 = MODE_FLAG_HAS_PER_LED_COLOR;
//            debug_keyboard_direct_mode.color_mode            = MODE_COLORS_PER_LED;
//
//            debug_keyboard->modes.push_back(debug_keyboard_direct_mode);
//
//            /*---------------------------------------------------------*\
//            | Create a matrix zone for the debug Keyboard               |
//            \*---------------------------------------------------------*/
//            zone debug_keyboard_matrix_zone;
//
//            debug_keyboard_matrix_zone.name                  = "Keyboard Matrix Zone";
//            debug_keyboard_matrix_zone.type                  = ZONE_TYPE_MATRIX;
//            debug_keyboard_matrix_zone.leds_min              = new_kb.GetKeyCount();
//            debug_keyboard_matrix_zone.leds_max              = new_kb.GetKeyCount();
//            debug_keyboard_matrix_zone.leds_count            = new_kb.GetKeyCount();
//            debug_keyboard_matrix_zone.matrix_map            = new matrix_map_type;
//            debug_keyboard_matrix_zone.matrix_map->height    = new_kb.GetRowCount();
//            debug_keyboard_matrix_zone.matrix_map->width     = new_kb.GetColumnCount();
//            debug_keyboard_matrix_zone.matrix_map->map       = new unsigned int[debug_keyboard_matrix_zone.matrix_map->height
//                                                                                * debug_keyboard_matrix_zone.matrix_map->width];
//            new_kb.GetKeyMap(debug_keyboard_matrix_zone.matrix_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT);
//
//            debug_keyboard->zones.push_back(debug_keyboard_matrix_zone);
//
//            for(unsigned int led_idx = 0; led_idx < debug_keyboard_matrix_zone.leds_count; led_idx++)
//            {
//                led debug_keyboard_led;
//                debug_keyboard_led.name = new_kb.GetKeyNameAt(led_idx);
//                debug_keyboard->leds.push_back(debug_keyboard_led);
//                debug_keyboard->led_alt_names.push_back(new_kb.GetKeyAltNameAt(led_idx));
//            }
      else if(type_setting == "mouse")
        {
            name                = "Debug Mouse";
            type                = DEVICE_TYPE_MOUSE;
        }
        else if(type_setting == "argb")
        {
            name                = "Debug ARGB Controller";
            type                = DEVICE_TYPE_LEDSTRIP;
        }

        /*---------------------------------------------------------*\
        | Fill in debug controller information                      |
        \*---------------------------------------------------------*/
        description                     = name + " Device";
        vendor                          = name + " Vendor String";
        location                        = name + " Location String";
        version                         = name + " Version String";
        serial                          = name + " Serial String";

        if(name_setting != "")
        {
            name                        = name_setting;
        }

        /*---------------------------------------------------------*\
        | Create a direct mode                                      |
        \*---------------------------------------------------------*/
        mode Direct;

        Direct.name                     = "Direct";
        Direct.value                    = 0;
        Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR;
        Direct.color_mode               = MODE_COLORS_PER_LED;

        modes.push_back(Direct);

        /*---------------------------------------------------------*\
        | Create a single zone/LED                                  |
        \*---------------------------------------------------------*/
        if(zone_single)
        {
            zone single_zone;

            single_zone.name            = "Single Zone";
            single_zone.type            = ZONE_TYPE_SINGLE;
            single_zone.leds_min        = 1;
            single_zone.leds_max        = 1;
            single_zone.leds_count      = 1;
            single_zone.matrix_map      = NULL;

            zones.push_back(single_zone);

            led single_led;

            single_led.name             = "Single LED";

            leds.push_back(single_led);

            led_alt_names.push_back("");
        }

        /*---------------------------------------------------------*\
        | Create a linear zone                                      |
        \*---------------------------------------------------------*/
        if(zone_linear)
        {
            zone linear_zone;

            linear_zone.name            = "Linear Zone";
            linear_zone.type            = ZONE_TYPE_LINEAR;
            linear_zone.leds_min        = 10;
            linear_zone.leds_max        = 10;
            linear_zone.leds_count      = 10;
            linear_zone.matrix_map      = NULL;

            zones.push_back(linear_zone);

            for(std::size_t led_idx = 0; led_idx < 10; led_idx++)
            {
                led linear_led;

                linear_led.name         = "Linear LED " + std::to_string(led_idx);

                leds.push_back(linear_led);

                led_alt_names.push_back("");
            }
        }
        /*---------------------------------------------------------*\
        | Create a keyboard matrix zone                             |
        \*---------------------------------------------------------*/
        if(zone_keyboard)
        {
            KEYBOARD_LAYOUT layout              = KEYBOARD_LAYOUT::KEYBOARD_LAYOUT_ANSI_QWERTY;
            KEYBOARD_SIZE   size                = KEYBOARD_SIZE::KEYBOARD_SIZE_FULL;

            if(debug_settings.contains("layout"))
            {
                KEYBOARD_LAYOUT temp_layout = debug_settings["layout"];

                if(temp_layout < NUM_LAYOUTS)
                {
                    layout = temp_layout;
                }
            }

            KeyboardLayoutManager new_kb(layout, size);

            description                        += ", Layout: " + layout_names[layout];

            /*-----------------------------------------------------*\
            | Check for custom key inserts and swaps                |
            \*-----------------------------------------------------*/
            const char* change_keys             = "change_keys";

            if(debug_settings.contains(change_keys))
            {
                std::vector<keyboard_led> change;

                const char* ins_row             = "ins_row";
                const char* rmv_key             = "rmv_key";
                const char* rmv_row             = "rmv_row";
                const char* swp_key             = "swp_key";

                const char* dbg_zone            = "Zone";
                const char* dbg_row             = "Row";
                const char* dbg_col             = "Col";
                const char* dbg_val             = "Val";
                const char* dbg_name            = "Name";
                const char* dbg_opcode          = "Opcode";

                for(size_t i = 0; i < debug_settings[change_keys].size(); i++)
                {
                    keyboard_led* key           = new keyboard_led;

                    key->zone                   = debug_settings[change_keys][i][dbg_zone];
                    key->row                    = debug_settings[change_keys][i][dbg_row];
                    key->col                    = debug_settings[change_keys][i][dbg_col];
                    key->value                  = debug_settings[change_keys][i][dbg_val];
                    key->name                   = debug_settings[change_keys][i][dbg_name].get_ref<const std::string&>().c_str();

                    if(debug_settings[change_keys][i][dbg_opcode] == ins_row)
                    {
                        key->opcode             = KEYBOARD_OPCODE_INSERT_ROW;
                    }
                    else if(debug_settings[change_keys][i][dbg_opcode] == rmv_key)
                    {
                        key->opcode             = KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT;
                    }
                    else if(debug_settings[change_keys][i][dbg_opcode] == rmv_row)
                    {
                        key->opcode             = KEYBOARD_OPCODE_REMOVE_ROW;
                    }
                    else if(debug_settings[change_keys][i][dbg_opcode] == swp_key)
                    {
                        key->opcode             = KEYBOARD_OPCODE_SWAP_ONLY;
                    }
                    else
                    {
                        key->opcode             = KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT;
                    }

                    change.push_back(*key);
                }

                new_kb.ChangeKeys(change);
            }

            zone keyboard_zone;

            keyboard_zone.name                  = "Keyboard Zone";
            keyboard_zone.type                  = ZONE_TYPE_MATRIX;
            keyboard_zone.leds_min              = new_kb.GetKeyCount();
            keyboard_zone.leds_max              = new_kb.GetKeyCount();
            keyboard_zone.leds_count            = new_kb.GetKeyCount();
            keyboard_zone.matrix_map            = new matrix_map_type;
            keyboard_zone.matrix_map->height    = new_kb.GetRowCount();
            keyboard_zone.matrix_map->width     = new_kb.GetColumnCount();
            keyboard_zone.matrix_map->map       = new unsigned int[keyboard_zone.matrix_map->height * keyboard_zone.matrix_map->width];

            new_kb.GetKeyMap(keyboard_zone.matrix_map->map, KEYBOARD_MAP_FILL_TYPE_COUNT);

            zones.push_back(keyboard_zone);

            for(unsigned int led_idx = 0; led_idx < keyboard_zone.leds_count; led_idx++)
            {
                led keyboard_led;

                keyboard_led.name = new_kb.GetKeyNameAt(led_idx);

                leds.push_back(keyboard_led);

                led_alt_names.push_back(new_kb.GetKeyAltNameAt(led_idx));
            }
        }
        /*---------------------------------------------------------*\
        | Create an underglow matrix zone                           |
        \*---------------------------------------------------------*/
        if(zone_underglow)
        {
            zone underglow_zone;

            underglow_zone.name                 = "Underglow Zone";
            underglow_zone.type                 = ZONE_TYPE_MATRIX;
            underglow_zone.leds_min             = 30;
            underglow_zone.leds_max             = 30;
            underglow_zone.leds_count           = 30;
            underglow_zone.matrix_map           = new matrix_map_type;
            underglow_zone.matrix_map->height   = 3;
            underglow_zone.matrix_map->width    = 10;
            underglow_zone.matrix_map->map      = (unsigned int*)&debug_keyboard_underglow_map;

            zones.push_back(underglow_zone);

            for(std::size_t led_idx = 0; led_idx < underglow_zone.leds_count; led_idx++)
            {
                led underglow_led;

                underglow_led.name              = "Underglow LED "  + std::to_string(led_idx);;

                leds.push_back(underglow_led);

                led_alt_names.push_back("");
            }
        }
        /*---------------------------------------------------------*\
        | Create a resizable linear zone                            |
        \*---------------------------------------------------------*/
        if(zone_resizable)
        {
            zone resizable_zone;

            resizable_zone.name         = "Resizable Zone";
            resizable_zone.type         = ZONE_TYPE_LINEAR;
            resizable_zone.leds_min     = 0;
            resizable_zone.leds_max     = 100;
            resizable_zone.leds_count   = 0;
            resizable_zone.matrix_map   = NULL;

            zones.push_back(resizable_zone);
        }
    }

    SetupColors();
}

RGBController_Debug::~RGBController_Debug()
{

}

void RGBController_Debug::SetupZones()
{

}

void RGBController_Debug::ResizeZone(int index, int new_size)
{
    //Make sure that it isn't out of bounds (negative numbers)
    if(new_size < int(zones[index].leds_min))
    {
         new_size = zones[index].leds_min;
    }

    // Same thing as the above line except for over 100
    if(new_size > int(zones[index].leds_max))
    {
        new_size = zones[index].leds_max;
    }

    // Store the previous amount of LEDs
    int old_size = zones[index].leds_count;

    // Set the LED count in the zone to the new ammount
    zones[index].leds_count = new_size;

    // Set the new ammount of LEDs for to the new size
    size_t old_leds_size = leds.size();

    // Add the new ammount of LEDs to the old ammount
    size_t new_leds_size = leds.size() - old_size + new_size;

    leds.resize(std::max(old_leds_size, new_leds_size));

    memmove((void *)(&leds[zones[index].start_idx] + old_leds_size), (const void *)(&leds[zones[index].start_idx] + new_leds_size), (old_leds_size - zones[index].start_idx - old_size) * sizeof(led));

    leds.resize(new_leds_size);

    for(int i = 0; i < new_size; ++i)
    {
        leds[zones[index].start_idx + i].name = "Linear LED " + std::to_string(i);
    }

    SetupColors();
}

void RGBController_Debug::DeviceUpdateLEDs()
{

}

void RGBController_Debug::UpdateZoneLEDs(int /*zone*/)
{

}

void RGBController_Debug::UpdateSingleLED(int /*led*/)
{

}

void RGBController_Debug::SetCustomMode()
{

}

void RGBController_Debug::DeviceUpdateMode()
{

}

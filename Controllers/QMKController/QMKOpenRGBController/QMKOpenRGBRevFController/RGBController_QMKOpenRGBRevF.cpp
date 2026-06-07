/*---------------------------------------------------------*\
| RGBController_QMKOpenRGBRevF.cpp                          |
|                                                           |
|   Driver for OpenRGB QMK Keyboard Protocol Revision F     |
|                                                           |
|   reklrekl                                    06 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "hsv.h"
#include "LogManager.h"
#include "RGBController_QMKOpenRGBRevF.h"

RGBController_QMKOpenRGBRevF::RGBController_QMKOpenRGBRevF(QMKOpenRGBRevFController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = controller->GetDeviceVendor();
    description = "QMK OpenRGB Device (Protocol Version " + std::to_string(controller->GetProtocolVersion()) + ")";
    type        = DEVICE_TYPE_KEYBOARD;
    location    = controller->GetLocation();
    version     = controller->GetQMKVersion();

    /*-----------------------------------------------------*\
    | Fetch all enabled modes                                 |
    \*-----------------------------------------------------*/
    std::vector<uint8_t> all_mode_ids = controller->GetEnabledModes();

    /*-----------------------------------------------------*\
    | Initialize modes from firmware capabilities            |
    \*-----------------------------------------------------*/
    const bool save = (controller->GetFeatures() & QMK_OPENRGB_FEATURE_SAVE);
    LOG_INFO("[%s] Keyboard feature flags: %u", name.c_str(), controller->GetFeatures());
    unsigned int custom_no = 1;
    for (unsigned int i = 0; i < all_mode_ids.size(); i++)
    {
        const uint8_t mode_id = all_mode_ids[i];
        unsigned int qmk_id = i + 1;

        QMKOpenRGBModeLookup lookup = QMKOpenRGBLookupMode(mode_id);
        if (mode_id == QMK_OPENRGB_MODE_CUSTOM) {
            if (controller->GetFeatures() & QMK_OPENRGB_FEATURE_EFFECT_NAMES){
                std::string effect_name = controller->GetEffectName(qmk_id);
                if (!effect_name.empty()) {
                    InitializeMode(effect_name, qmk_id, lookup.flags, save);
                    continue;
                }
            }
            InitializeMode(lookup.name + std::to_string(custom_no), qmk_id, lookup.flags, save);
            custom_no++;
            continue;
        }

        InitializeMode(lookup.name, qmk_id, lookup.flags, save);
    }

    /*-----------------------------------------------------*\
    | Determine active mode from firmware state              |
    \*-----------------------------------------------------*/
    unsigned int firmware_mode = controller->GetMode();
    active_mode = 0;

    for (unsigned int i = 0; i < modes.size(); i++)
    {
        if (modes[i].value == firmware_mode)
        {
            active_mode = i;
            break;
        }
    }

    SetupZones();
}

RGBController_QMKOpenRGBRevF::~RGBController_QMKOpenRGBRevF()
{
    for(unsigned int zone_index = 0; zone_index < zones.size(); zone_index++)
    {
        if(zones[zone_index].matrix_map != NULL)
        {
            delete zones[zone_index].matrix_map;
        }
    }
}

void RGBController_QMKOpenRGBRevF::SetupZones()
{
    /*---------------------------------------------------------*\
    | Get the number of LEDs from the device                    |
    \*---------------------------------------------------------*/
    const unsigned int total_number_of_leds                     = controller->GetTotalNumberOfLEDs();

    LOG_INFO("[%s] Keyboard has %u LEDs total", name.c_str(), total_number_of_leds);

    /*---------------------------------------------------------*\
    | Get information for each LED                              |
    \*---------------------------------------------------------*/
    controller->GetLEDInfo(total_number_of_leds);

    /*---------------------------------------------------------*\
    | Get LED vectors from controller                           |
    \*---------------------------------------------------------*/
    std::vector<point_t>        led_points                      = controller->GetLEDPoints();
    std::vector<unsigned int>   led_flags                       = controller->GetLEDFlags();
    std::vector<std::string>    led_names                       = controller->GetLEDNames();
    std::vector<unsigned int>   led_values                      = controller->GetLEDValues();

    if(led_points.empty())
    {
        LOG_WARNING("[%s] Keyboard has 0 LEDs, controller will be non-functional", name.c_str());
        return;
    }

    /*---------------------------------------------------------*\
    | Count key LEDs and underglow LEDs                         |
    \*---------------------------------------------------------*/
    unsigned int    key_led_count                          = controller->GetKeyLEDCount();
    unsigned int    underglow_led_count                    = controller->GetUnderglowLEDCount();

    /*---------------------------------------------------------*\
    | Count total LEDs and check if underglow exists            |
    \*---------------------------------------------------------*/
    unsigned int    number_of_leds                              = key_led_count + underglow_led_count;
    bool            has_underglow                               = underglow_led_count > 0;
    LOG_INFO("[%s] Keyboard has %u underglow LEDs", name.c_str(), underglow_led_count);

    /*---------------------------------------------------------*\
    | Create sets for row and column position values            |
    \*---------------------------------------------------------*/
    std::set<int> rows, columns;
    for (unsigned int i = 0; i < number_of_leds; i++)
    {
        rows.insert(led_points[i].y);
        columns.insert(led_points[i].x);
    }

    /*---------------------------------------------------------*\
    | Calculate matrix map from QMK positions                   |
    \*---------------------------------------------------------*/
    unsigned int divisor = CalculateDivisor(led_points, rows, columns);
    LOG_DEBUG("[%s] Distance between standard keys calculated to be %u", name.c_str(), divisor);

    VectorMatrix matrix_map;
    VectorMatrix underglow_map;

    PlaceLEDsInMaps(rows, columns, divisor, led_points, led_flags, matrix_map, underglow_map);
    CleanMatrixMaps(matrix_map, underglow_map, (unsigned int)rows.size(), has_underglow);

    /*---------------------------------------------------------*\
    | These vectors are class members because if they go out of |
    | scope, the underlying array (used by each zones'          |
    | matrix_map) is unallocated.                               |
    \*---------------------------------------------------------*/
    flat_matrix_map     = FlattenMatrixMap(matrix_map);
    flat_underglow_map  = FlattenMatrixMap(underglow_map);

    /*---------------------------------------------------------*\
    | Create Keyboard zone                                      |
    \*---------------------------------------------------------*/
    if(!matrix_map.empty())
    {
        zone keys_zone;
        keys_zone.name                          = "Keyboard";
        keys_zone.type                          = ZONE_TYPE_MATRIX;
        keys_zone.leds_min                      = key_led_count;
        keys_zone.leds_max                      = keys_zone.leds_min;
        keys_zone.leds_count                    = keys_zone.leds_min;
        keys_zone.matrix_map                    = new matrix_map_type;
        keys_zone.matrix_map->width             = (unsigned int)matrix_map[0].size();
        keys_zone.matrix_map->height            = (unsigned int)matrix_map.size();
        keys_zone.matrix_map->map               = flat_matrix_map.data();
        zones.push_back(keys_zone);
    }

    /*---------------------------------------------------------*\
    | Create Underglow zone if it exists                        |
    \*---------------------------------------------------------*/
    if(has_underglow && !underglow_map.empty())
    {
        zone underglow_zone;
        underglow_zone.name                 = "Underglow";
        underglow_zone.type                 = ZONE_TYPE_MATRIX;
        underglow_zone.leds_min             = underglow_led_count;
        underglow_zone.leds_max             = underglow_zone.leds_min;
        underglow_zone.leds_count           = underglow_zone.leds_min;
        underglow_zone.matrix_map           = new matrix_map_type;
        underglow_zone.matrix_map->width    = (unsigned int)underglow_map[0].size();
        underglow_zone.matrix_map->height   = (unsigned int)underglow_map.size();
        underglow_zone.matrix_map->map      = flat_underglow_map.data();
        zones.push_back(underglow_zone);
    }

    /*---------------------------------------------------------*\
    | Create LEDs                                               |
    \*---------------------------------------------------------*/
    for(unsigned int led_idx = 0; led_idx < number_of_leds; led_idx++)
    {
        led keyboard_led;

        if(led_idx < led_names.size())
        {
            keyboard_led.name               = led_names[led_idx];
        }

        keyboard_led.value                  = led_values[led_idx];

        leds.push_back(keyboard_led);
    }

    /*---------------------------------------------------------*\
    | Setup Colors                                              |
    \*---------------------------------------------------------*/
    SetupColors();

    if(controller->GetLEDColors().size() < leds.size())
    {
        LOG_WARNING("[%s] LED color count (%u) < LED count (%u), some LEDs may not initialize correctly",
                    name.c_str(), (unsigned int)controller->GetLEDColors().size(), (unsigned int)leds.size());
    }

    for(unsigned int i = 0; i < leds.size(); i++)
    {
        colors[i] = controller->GetLEDColors()[i];
    }
}

void RGBController_QMKOpenRGBRevF::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_QMKOpenRGBRevF::DeviceUpdateLEDs()
{
    controller->DirectModeSetLEDs(colors, controller->GetTotalNumberOfLEDs());
}

void RGBController_QMKOpenRGBRevF::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_QMKOpenRGBRevF::UpdateSingleLED(int led)
{
    RGBColor      color = colors[led];
    unsigned char red   = RGBGetRValue(color);
    unsigned char grn   = RGBGetGValue(color);
    unsigned char blu   = RGBGetBValue(color);

    controller->DirectModeSetSingleLED(led, red, grn, blu);
}

void RGBController_QMKOpenRGBRevF::DeviceUpdateMode()
{
    if(modes.empty())
    {
        return;
    }

    if(modes[active_mode].color_mode == MODE_COLORS_PER_LED)
    {
        controller->SetMode({ 0, 255, 255 }, modes[active_mode].value, 127, false);
    }
    else if(modes[active_mode].color_mode == MODE_COLORS_NONE)
    {
        controller->SetMode({ 0, 255, 255 }, modes[active_mode].value, modes[active_mode].speed, false);
    }
    else if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        RGBColor rgb_color = modes[active_mode].colors[0];
        hsv_t hsv_color;
        rgb2hsv(rgb_color, &hsv_color);

        if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
        {
            controller->SetMode(hsv_color, modes[active_mode].value, modes[active_mode].speed, false);
        }
        else
        {
            controller->SetMode(hsv_color, modes[active_mode].value, 127, false);
        }
    }
}

void RGBController_QMKOpenRGBRevF::DeviceSaveMode()
{
    if(modes.empty())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Direct mode (MODE_COLORS_PER_LED) is not handled here.    |
    | Direct LED values cannot be saved to the keyboard - only  |
    | the mode itself (e.g. switching to Direct mode) can be.   |
    \*---------------------------------------------------------*/
    if(modes[active_mode].color_mode == MODE_COLORS_NONE)
    {
        controller->SetMode({ 0, 255, 255 }, modes[active_mode].value, modes[active_mode].speed, true);
    }
    else if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        RGBColor rgb_color = modes[active_mode].colors[0];
        hsv_t hsv_color;
        rgb2hsv(rgb_color, &hsv_color);

        if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
        {
            controller->SetMode(hsv_color, modes[active_mode].value, modes[active_mode].speed, true);
        }
        else
        {
            controller->SetMode(hsv_color, modes[active_mode].value, 127, true);
        }
    }
}

void RGBController_QMKOpenRGBRevF::InitializeMode
    (
    std::string name,
    unsigned int qmk_id,
    unsigned int flags,
    bool save
    )
{
    mode qmk_mode;
    qmk_mode.name       = name;
    qmk_mode.value      = qmk_id;
    qmk_mode.flags      = flags;

    if(flags & MODE_FLAG_HAS_PER_LED_COLOR)
    {
        qmk_mode.color_mode = MODE_COLORS_PER_LED;
    }
    else if(flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        qmk_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
    }
    else
    {
        qmk_mode.color_mode = MODE_COLORS_NONE;
    }

    if(flags & MODE_FLAG_HAS_SPEED)
    {
        qmk_mode.speed_min     = QMK_OPENRGB_SPEED_SLOWEST;
        qmk_mode.speed_max     = QMK_OPENRGB_SPEED_FASTEST;
        qmk_mode.speed         = QMK_OPENRGB_SPEED_NORMAL;
    }
    if(flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        qmk_mode.colors_min = 1;
        qmk_mode.colors_max = 1;
        qmk_mode.colors.resize(1);
        qmk_mode.colors[0] = controller->GetModeColor();
    }

    /*-----------------------------------------------------*\
    | Direct mode it the last mode on the QMK firmware      |
    | but we still want it to appear first on the UI        |
    \*-----------------------------------------------------*/
    if(flags & MODE_FLAG_HAS_PER_LED_COLOR)
    {
        modes.insert(modes.begin(), qmk_mode);
    }
    else
    {
        /*-----------------------------------------------------*\
        | Every mode apart from direct is save-able             |
        \*-----------------------------------------------------*/
        if(save)
        {
            qmk_mode.flags = flags | MODE_FLAG_MANUAL_SAVE;
        }
        modes.push_back(qmk_mode);
    }
}

unsigned int RGBController_QMKOpenRGBRevF::CalculateDivisor
    (
    std::vector<point_t> led_points,
    std::set<int> rows,
    std::set<int> /*columns*/
    )
{
    std::vector< std::vector<point_t> > row_points(rows.size());
    for(const point_t &pt : led_points)
    {
        for(const int &i : rows)
        {
            if(pt.y == i)
            {
                row_points[std::distance(rows.begin(), rows.find(i))].push_back(pt);
            }
        }
    }

    int last_pos;
    std::vector<int> distances;
    for(const std::vector<point_t> &row : row_points)
    {
        last_pos = 0;
        std::for_each(row.begin(), row.end(), [&distances, &last_pos](const point_t &pt)
        {
            distances.push_back(std::abs(pt.x - last_pos));
            last_pos = pt.x;
        });
    }
    std::map<int, int> counts;
    for(const int &i : distances)
    {
        counts[i]++;
    }

    /*---------------------------------------------------------*\
    | Guard against empty distances (malformed LED data)        |
    \*---------------------------------------------------------*/
    if(distances.empty())
    {
        LOG_WARNING("[%s] No valid LED distances found, using default divisor of 1", name.c_str());
        return 1;
    }

    unsigned int divisor = distances[0];
    for(const std::pair<const int, int> &i : counts)
    {
        if(counts[divisor] < i.second)
        {
            divisor = i.first;
        }
    }

    if(divisor == 0)
    {
        LOG_WARNING("[%s] Calculated divisor is 0, using default of 1. This may indicate malformed LED position data.", name.c_str());
        return 1;
    }

    return divisor;
}

void RGBController_QMKOpenRGBRevF::PlaceLEDsInMaps
    (
        std::set<int>               unique_rows,
        std::set<int>               /*unique_cols*/,
        unsigned int                divisor,
        std::vector<point_t>        led_points,
        std::vector<unsigned int>   led_flags,
        VectorMatrix&               matrix_map_xl,
        VectorMatrix&               underglow_map_xl
     )
{
    matrix_map_xl                   = MakeEmptyMatrixMap(unique_rows.size(), (std::size_t)(std::round(255 / divisor) + 10));
    underglow_map_xl                = MakeEmptyMatrixMap(unique_rows.size(), (std::size_t)(std::round(255 / divisor) + 10));

    unsigned int x                  = 0;
    unsigned int y                  = 0;
    unsigned int underglow_counter  = 0;

    for(unsigned int i = 0; i < led_points.size(); i++)
    {
        if(led_points[i].x != 255 && led_points[i].y != 255)
        {
            bool underglow          = led_flags[i] & 2;

            x                       = (unsigned int)(std::round(led_points[i].x / divisor));
            y                       = (unsigned int)(std::distance(unique_rows.begin(), unique_rows.find(led_points[i].y)));

            if(!underglow)
            {
                while(matrix_map_xl[y][x] != NO_LED)
                {
                    x++;
                }
                matrix_map_xl[y][x] = i;
                LOG_DEBUG("[%s] Key Matrix LED %u, (%u, %u) being placed into (%u, %u)", name.c_str(), i, led_points[i].x, led_points[i].y, x, y);
            }
            else
            {
                while(underglow_map_xl[y][x] != NO_LED)
                {
                    x++;
                }
                underglow_map_xl[y][x] = underglow_counter;
                underglow_counter++;
                LOG_DEBUG("[%s] Underglow  LED %u, (%u, %u) being placed into (%u, %u)", name.c_str(), i, led_points[i].x, led_points[i].y, x, y);
            }
        }
    }
}

VectorMatrix RGBController_QMKOpenRGBRevF::MakeEmptyMatrixMap
    (
        std::size_t height,
        std::size_t width
    )
{
    std::vector<std::vector<unsigned int> > matrix_map(height);
    for(std::size_t i = 0; i < height; i++)
    {
        for(std::size_t j = 0; j < width; j++)
        {
            matrix_map[i].push_back(NO_LED);
        }
    }
    return matrix_map;
}

void RGBController_QMKOpenRGBRevF::CleanMatrixMaps
    (
    VectorMatrix&   matrix_map,
    VectorMatrix&   underglow_map,
    unsigned int    height,
    bool            has_underglow
    )
{
    bool empty_col              = true;
    bool empty_col_udg          = true;
    bool empty_row              = true;
    int width                   = 0;
    int width_udg               = 0;

    std::vector<int> empty_rows;

    bool can_break;
    bool can_break_udg;

    for(unsigned int i = 0; i < height; i++)
    {
        empty_row               = true;
        can_break               = false;
        can_break_udg           = false;

        for(int j = (int)matrix_map[i].size() - 1; j --> 0; )
        {
            if(matrix_map[i][j] != NO_LED && width < (j + 1) && !can_break)
            {
                width           = (j + 1);
                can_break       = true;
                empty_row       = false;
            }
            else if(matrix_map[i][j] != NO_LED)
            {
                empty_row       = false;
            }
            if(underglow_map[i][j] != NO_LED && width_udg < (j + 1) && !can_break_udg)
            {
                width_udg       = (j + 1);
                can_break_udg   = true;
            }
            if (can_break && can_break_udg) break;
        }

        if(matrix_map[i][0] != NO_LED)
        {
            empty_col           = false;
        }

        if(underglow_map[i][0] != NO_LED)
        {
            empty_col_udg       = false;
        }

        if(empty_row)
        {
            empty_rows.push_back(i);
        }
    }

    unsigned int new_height     = height - (unsigned int)empty_rows.size();
    width                       = empty_col ? width - 1 : width;
    width_udg                   = empty_col_udg && empty_col ? width_udg - 1 : width_udg;
    LOG_DEBUG("[%s] Key LED Matrix: %ux%u", name.c_str(), width, new_height);
    LOG_DEBUG("[%s] Underglow LED Matrix: %ux%u", name.c_str(), width_udg, new_height);

    for(unsigned int i = (unsigned int)empty_rows.size(); i --> 0; )
    {
        matrix_map.erase(matrix_map.begin()+empty_rows[i]);
    }

    for(unsigned int i = 0; i < new_height; i++)
    {
        if(empty_col)
        {
            matrix_map[i].erase(matrix_map[i].begin(), matrix_map[i].begin() + 1);
        }

        if(empty_col_udg && empty_col)
        {
            underglow_map[i].erase(underglow_map[i].begin(), underglow_map[i].begin() + 1);
        }

        matrix_map[i].erase(matrix_map[i].begin()+width, matrix_map[i].end());

        if(has_underglow)
        {
            underglow_map[i].erase(underglow_map[i].begin()+width_udg, underglow_map[i].end());
        }
    }

    if(has_underglow)
    {
        for(unsigned int i = new_height; i < height; i++)
        {
            underglow_map[i].erase(underglow_map[i].begin()+width_udg, underglow_map[i].end());
        }
    }
}

std::vector<unsigned int> RGBController_QMKOpenRGBRevF::FlattenMatrixMap
    (
        VectorMatrix matrix_map
    )
{
    std::vector<unsigned int> flat_map;

    for(const std::vector<unsigned int> &row : matrix_map)
    {
        for(const unsigned int &item : row)
        {
            flat_map.push_back(item);
        }
    }
    return flat_map;
}
